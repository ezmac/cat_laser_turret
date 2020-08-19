/* Cat laser attak
 */

#include <Servo.h>
#include <Servo.h>
#include <avr/sleep.h>

Servo servoX;
Servo servoY;
const int laser = 11;
int etat = 0;
// Globals for storing position...  probably useless since the servo library does it for me
int posX=165;
int posY=90;
int xRandRangeMax=15;
int xRandRangeMin=-15;

// Don't want these functions in my way, so prototype them and define them after loop.
void xyMove(int currentX, int currentY, int newX, int newY, int steps=10, int msDuration=500);
void handleInput();



// I'm not very concerned with timing being accurate.  I just want to turn off after ROUGHLY N minutes.  Probably 10.
unsigned long int timer=0;

// timer will track the amount of time this has been on by adding.. until I figure out real time.  When it's larger than X, it'll reset and sleep.
unsigned long int catInterestTime=10;// minutes


unsigned long int highTimer=0;
unsigned long int maxHighTime=15000;//ms; can stay above horizon for 15s.  Logic is broken, but generally working.


void setup() {
  // put your setup code here, to run once:

  pinMode (laser, OUTPUT);
  servoX.attach(10);
  servoY.attach(9);

  //digitalWrite (laser, HIGH);
  servoX.write(posX);
  servoY.write(posY);
  Serial.begin(9600);
  etat = 0;

// related to wake on button press.
  digitalWrite (2, HIGH);  // enable pull-up
}


int elapsedMillis(int startMillis, int endMillis){
  return endMillis - startMillis;
}
unsigned long int trackTime(unsigned long int timer, int startMillis, int endMillis){
  unsigned long int elapsed = (unsigned long int)elapsedMillis(startMillis, endMillis);
  return timer + elapsed;
}

bool isXHigh(int x){
  if (x <= 115){
    return true;
  }
  return false;
}

bool isYHigh(int y){
  if (y>=135){
    return true;
  }
  return false;
}

bool isYLow(int y){
  if (y<=45){
    return true;
  }
  return false;
}
bool isHigh(int x, int y){

/* For the full sweep of x,y, if X>110 ish, points to floor for 0<y<180
   if x<110, points straight out or towards ceiling
   */
  if (isXHigh(x)){
    return true;
  }
  if (isYHigh(y) || isYLow(y)){
    return true;
  }
  return false;
}

void loop(){ 
  if (Serial.available() != 0){
    handleInput(); //debug mode
  }

  digitalWrite(laser, 1);
  int startMillis= millis();
  int millisBetweenTicks=random(1, 1000);

  int xDir=random(xRandRangeMin,xRandRangeMax);
  int yDir=random(-15,15);
  posX=servoX.read();
  posY=servoY.read();
  Serial.print("(X, Y): "); Serial.print(posX); Serial.print(", ");Serial.println(posY);

  if (highTimer >= maxHighTime){
    if (isXHigh(posX)){
      xDir=random(0, xRandRangeMax);
    }
    if (isYHigh(posY)){
      yDir=random(-15, -1);
    }
    if (isYLow(posY)){
      yDir=random(1, 15);
    }
  }
  Serial.print("(Xmod, Ymod): "); Serial.print(xDir); Serial.print(", ");Serial.println(yDir);

  int laserIntensity=random(0,1); // right now, on or off.  Maybe later, PWM.
  // where as before, we would move by chunks, now we may not.

  xyMove(posX, posY, (posX + xDir), (posY + yDir));
  delay(millisBetweenTicks);

  posX=servoX.read();
  posY=servoY.read();


  int endMillis= millis();
  timer=trackTime(timer, startMillis, endMillis);
  if (isHigh(posX, posY)){
    Serial.print("High: "); Serial.println(highTimer);
    highTimer=trackTime(highTimer, startMillis, endMillis);
  }
  else {
    if (highTimer >= maxHighTime){
      highTimer=0;
    }
  }
  if (shouldSleep(timer)){
    Serial.println("Sleeping");
    timer=0;
    digitalWrite(laser, 0);
    sleep();
  }
}

bool shouldSleep(unsigned long int timer){
  if (timer >= (catInterestTime * 60 * 1000)){
    return true;
  }
  return false;
}

void sleep(){
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
  sleep_enable();
  // Do not interrupt before we go to sleep, or the
  // ISR will detach interrupts and we won't wake.
  noInterrupts ();
  // will be called when pin D2 goes low  
  attachInterrupt (0, wake, FALLING);
  EIFR = bit (INTF0);  // clear flag for interrupt 0
  // turn off brown-out enable in software
  // BODS must be set to one and BODSE must be set to zero within four clock cycles
  MCUCR = bit (BODS) | bit (BODSE);
  // The BODS bit is automatically cleared after three clock cycles
  MCUCR = bit (BODS); 
  // We are guaranteed that the sleep_cpu call will be done
  // as the processor executes the next instruction after
  // interrupts are turned on.
  interrupts ();  // one cycle
  sleep_cpu ();   // one cycle
}

void wake ()
{
  // cancel sleep as a precaution
  sleep_disable();
  // precautionary while we do other stuff
  detachInterrupt (0);
}  // end of wake
/*
void old_loop() {

  if (Serial.available() != 0){
    handleInput();
  }


  // put your main code here, to run repeatedly:

  //xyMove(0,0,100,100,100,5000);
  delay(1000);

  timer+=1000;
  //xyMove(100,100,0,0,100,5000);
  int moveX=0;
  int moveY=0;

  // out move range is 0-180;
  int mode=0;

  // number of ms used in a standard move.
  int default_move_time=500;
  mode = random(0,8);
  int confuzzle = random(0,8);
  if (confuzzle==0 && mode <=3) {
    digitalWrite(laser, 0);
    delay(10);
    Serial.println("Confuzzle mode");
  }
  else {
    digitalWrite(laser, 1);
    delay(10);
    Serial.println("NOT Confuzzle mode");
  }
  Serial.print("Mode: "); Serial.println(mode);

  switch(mode){
    case 0: // dunno but move 10 degrees both directions.
      moveX=random(-10,10);
      moveY=random(-10,10);
      xyMove(posX, posY, (posX + moveX), (posY + moveY));
      timer += default_move_time;
      break;
    case 1:
      moveX=random(-30,30);
      moveY=random(-30,30);
      xyMove(posX, posY, (posX + moveX), (posY + moveY));
      //Normal attraction mode, move at a low pace, smooth.
      timer += default_move_time;
      break;
    case 2: // dunno but move 10 degrees both directions.
      moveX=random(-60,60);
      moveY=random(-60,60);
      xyMove(posX, posY, (posX + moveX), (posY + moveY));
      timer += default_move_time;
      break;
    case 3:
      moveX=random(-90,90);
      moveY=random(-90,90);
      xyMove(posX, posY, (posX + moveX), (posY + moveY));
      //Normal attraction mode, move at a low pace, smooth.
      timer += default_move_time;
      break;
      // no breaks.  Extra delays
    case 4:
      timer += 100;
      delay(100);
    case 5:
      break;
    case 6:
      timer += 300;
      delay(300);
    case 7:
      timer += 400;
      delay(400);
    case 8:
      timer += 500;
      delay(500);

  }
  Serial.print("checking timer"); Serial.println(timer);
  if (timer >= (catInterestTime * 60 * 1000)){
    Serial.println("Sleeping");
    timer=0;
    //disable laser
    digitalWrite(laser, 0);
    sleep();

  }
}
*/





void xyMove(int currentX, int currentY, int newX, int newY, int steps=10, int msDuration=500){

  // digitalWrite (laser, HIGH);
  // disable ADC
  // ADCSRA = 0;  
  int xMin=0;
  int yMin=0;
  int xMax=0;
  int yMax=0;
  int msDelay=msDuration/steps;
  double xChunk=(newX-currentX)/(double)steps;
  double yChunk=(newY-currentY)/(double)steps;

  double xChunkAccumulator=0.0;
  double yChunkAccumulator=0.0;

  Serial.print("xChunk: "); Serial.println(xChunk);
  Serial.print("yChunk: "); Serial.println(yChunk);
  Serial.print("msDelay: "); Serial.println(msDelay);
  // xCA should be whole and abs 
  for (int i=0; i<steps; i++){
    xChunkAccumulator += (xChunk);
    // Serial.print("xCA: "); Serial.println(xChunkAccumulator);

    if (abs(xChunkAccumulator)>=1){
      int ixChunk=(int)xChunkAccumulator;
/*
      Serial.print("xChunk: "); Serial.println(xChunk);
      Serial.print("xWrite: "); Serial.println(ixChunk);
      Serial.print("xTotal: "); Serial.println(ixChunk+currentX);
*/
      xChunkAccumulator-=ixChunk;
      servoX.write(currentX + ixChunk);
      currentX=servoX.read();
    }
    yChunkAccumulator += (yChunk);
    if (abs(yChunkAccumulator)>=1){
      int iyChunk=(int)yChunkAccumulator;
      yChunkAccumulator-=iyChunk;
/*
      Serial.print("yChunk: "); Serial.println(yChunk);
      Serial.print("yWrite: "); Serial.println(iyChunk);
      Serial.print("yTotal: "); Serial.println(iyChunk+currentY);
*/
      servoY.write(currentY + iyChunk);
      currentY=servoY.read();
    }
    delay(msDelay);
  }
  posX=servoX.read();
  posY=servoY.read();
  return;

}




void handleInput(){
  String input;
  int x, y;
  while (input != "x"){
    Serial.print("Waiting: ");

    while(Serial.available() ==0){
    }
    input = Serial.readString();

    Serial.print(input);
    if (input=="g"){

      Serial.print("X:");
      while(Serial.available() ==0){
      }
      x = Serial.parseInt();

      Serial.print("Y:");
      while(Serial.available() ==0){
      }
      y = Serial.parseInt();
      posX=servoX.read();
      posY=servoY.read();

      Serial.print(x);
      Serial.print(",");
      Serial.println(y);

      servoX.write(x);
      servoY.write(y);
    }
    if (input=="G"){

      Serial.print("X:");
      while(Serial.available() ==0){
      }
      x = Serial.parseInt();

      Serial.print("Y:");
      while(Serial.available() ==0){
      }
      y = Serial.parseInt();

      Serial.print("Time:");
      while(Serial.available() ==0){
      }
      int t = Serial.parseInt();
      posX=servoX.read();
      posY=servoY.read();

      Serial.print(x);
      Serial.print(",");
      Serial.println(y);

      int moveX=x;
      int moveY=y;
      xyMove(posX, posY, (posX + moveX), (posY + moveY), 10, t);
    }
    if (input=="t"){
      x=0;
      while (x<180){
        servoX.write(x);
        delay(15);
        x++;
      }
    }
  }
}
