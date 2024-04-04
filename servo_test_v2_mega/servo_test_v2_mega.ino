
#include <Encoder.h>
//#include "Servo.h"
#include "MultiServo.h"
#include "Steppers.h"
// Change these two numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability

// pindef_____________________________________________________________________

const int z1en0 = 27;
const int z1en1 = 26;
const int z1pwm = 7;

const int z2pwm = 6;
const int z2en0 = 23;
const int z2en1 = 22;

const int d1en0 = 17;
const int d1en1 = 16;
const int d1pwm = 5;

const int d2en0 = 24;
const int d2en1 = 25;
const int d2pwm = 4;

const int s1dir = 28;
const int s1step = 29;
const int s2dir = 30;
const int s2step = 31;
const int voltPin = A15;

// end pindef__________________________________________________________________
long currPosition  = 0;
long currPosition2 = 0;
//Encoder myEnc(2, 8);
Servo s1(z1en0, z1en1, z1pwm, 19, 42);
Servo s2(z2en0, z2en1, z2pwm, 18, 43);
Servo d1(d1en0, d1en1, d1pwm, 3, 40);
Servo d2(d2en0, d2en1, d2pwm, 2, 41);
MultiServo zAxis(s1, s2);
MultiServo driveTrain(d1,d2);
Steppers xAxis(s1dir, s1step, s2dir, s2step);
//   avoid using pins with LEDs attached

void setup() {
  Serial.begin(9600);
  Serial.println("Basic Encoder Test:");

  pinMode(z1en0, OUTPUT);
  pinMode(z1en1, OUTPUT);
  pinMode(z1pwm, OUTPUT);

  pinMode(z2en0, OUTPUT);
  pinMode(z2en1, OUTPUT);
  pinMode(z2pwm, OUTPUT);

  pinMode(d1en0, OUTPUT);
  pinMode(d1en1, OUTPUT);
  pinMode(d1pwm, OUTPUT);

  pinMode(d2en0, OUTPUT);
  pinMode(d2en1, OUTPUT);
  pinMode(d2pwm, OUTPUT);


  pinMode(s1dir, OUTPUT);
  pinMode(s1step, OUTPUT);
  pinMode(s2dir, OUTPUT);
  pinMode(s2step, OUTPUT);

  pinMode(A0, INPUT);


  //s1.set_current_limiting(400, true);
}

double get_voltage(){
  double voltage = (double)analogRead(voltPin) / 1023 * 55.7;
  return voltage;
}

void drive_till_edge(int ospeed, int max_range){
  int max_speed = 300;
  double correction = 6.0;
  long initPos1 = d1.myEnc.read();
  long initPos2 = d2.myEnc.read();
  long currPos1 = d1.myEnc.read();
  long currPos2 = d2.myEnc.read();
  int thresh = 150;
  // Serial.println(analogRead(A9));
  // Serial.println(abs(currPos - initPos));
  // Serial.println(analogRead(A9) < thresh);
  // Serial.println(abs(currPos - initPos) < max_range);
  long lastPosition1 = d1.myEnc.read();
  long lastPosition2 = d1.myEnc.read();
  long lastT = millis();
  long t = millis();
  double velocity1 = 0;
  double velocity2 = 0;
  int control1 = ospeed;
  int control2 = ospeed;
  while((analogRead(A9) > thresh) && (abs(currPos1 - initPos1) < max_range)){
    control1 = ospeed;
    control2 = ospeed;
    t = millis();
    Serial.println(analogRead(A9));

    currPos1 = d1.myEnc.read();
    velocity1 = double(currPos1 - lastPosition1) / double(t - lastT + 0.1) * 1000;
    currPos2 = d2.myEnc.read();
    velocity2 = double(currPos2 - lastPosition2) / double(t - lastT + 0.1) * 1000;

    

    if(abs(velocity1) > max_speed){
      control1 = driveTrain.limit_speed(abs(velocity1), control1 / max_speed);
    }
    if(abs(velocity2) > max_speed){
      control2 = driveTrain.limit_speed(abs(velocity2), control2 / max_speed);
    }

    double rot = (abs(currPos1 - initPos1) - abs(currPos2 - initPos2)) / 792 /4.6 ;//28/6.1 = 4.6
    control2 *= (1.0 + rot * correction);
  
    Serial.print("velocity:");
    Serial.println(velocity1);
    Serial.print("control");
    Serial.println(control1);
    if(control1 < 0){
      d1.bw(-control1);
    }else{
      d1.fw(control1);
    }
    if(control2 < 0){
      d1.bw(-control2);
    }else{
      d1.fw(control2);
    }
    lastT = t;
    lastPosition1 = currPos1;
    lastPosition2 = currPos2;
    
  }
  d1.mbreak();
  d2.mbreak();
}
void cmd_servo_multi(){
  Serial.println("Enter CMD:");
  while (Serial.available() == 0){
    delay(10);
  }
  String cmd = Serial.readString();
  
  int index = 0;
  for(int i = 0; i < 5 && i < cmd.length(); i++){
    if(isDigit(cmd[i]) or cmd[i] == '-'){
      index = i;
      break;
    }
  }
  if(index == 0){
    index = cmd.length();
  }
  String dir = cmd.substring(0,index);
  double rev =cmd.substring(index).toFloat();
  Serial.print("cmd is:");
  Serial.println(dir);
  Serial.print("rev is:");
  Serial.println(rev);
  
  // if(dir == 'w'){
  //   s1.servo_to(tick1, 160, 0.2, 0.1);
  // }
  if(dir == "w"){
    long currPosition1 = s1.myEnc.read();
    long currPosition2 = s2.myEnc.read();
    long tick1 = -rev * 1836 + currPosition1;
    long tick2 = rev * 1836 + currPosition2;
    zAxis.servo_to(tick1, tick2, 40, 0.3, 0.1);
  }
  if(dir == "t"){
    Serial.println("poi");
    long currPosition1 = d1.myEnc.read();
    long currPosition2 = d2.myEnc.read();
    long tick1 = rev * 792 + currPosition1;
    long tick2 = rev * 792 + currPosition2;
    driveTrain.servo_to(tick1, tick2, 150, 1.4, 0.1, true,500);
  }
  if(dir == "fw"){
    long currPosition1 = d1.myEnc.read();
    long currPosition2 = d2.myEnc.read();
    long tick1 = rev * 792 + currPosition1;
    long tick2 = -rev * 792 + currPosition2;
    driveTrain.servo_to(tick1, tick2, 140, 0.5, 0.2, true, 400);
  }
  if(dir == "l"){
    long currPosition1 = d1.myEnc.read();
    long currPosition2 = d2.myEnc.read();
    long tick1 = 0.4 * 1836 + currPosition1;
    long tick2 = -0.4 * 1836 + currPosition2;
    driveTrain.servo_to_no_correction(tick1, tick2, 180, 0.8, 0.1, false);
    delay(500);
    tick1 = 1.0 * 1836 + currPosition1;
    tick2 = -1.0 * 1836 + currPosition2;
    driveTrain.servo_to_no_correction(tick1, tick2, 120, 0.4, 0.1, true);
  }
  if(dir == "a"){
    xAxis.revStepperSRamp(abs(rev), 1, 7 );
  }
  if(dir == "d"){
    xAxis.revStepperSRamp(abs(rev), -1, 7 );
  }
  if(dir == "z"){
    s1.myEnc.write(0);
    s2.myEnc.write(0);
    driveTrain.clear_odo();
  }
   if(dir == "p"){
    Serial.print("s1: ");
    Serial.println(s1.myEnc.read());
    Serial.print("s1: ");
    Serial.println(s2.myEnc.read());
  }
  if(dir == "v"){
    zAxis.servo_to(4.3 * 1836, -4.3* 1836, 80, 0.4, 0.1);
  }
  if(dir == "b"){
    xAxis.revStepperSRamp(6.4, -1, 6 );
  }
  if(dir == "n"){
    zAxis.servo_to(-0.4 * 1836, 0.4 * 1836, 80, 0.4, 0.1);
  }
  if(dir == "m"){
    xAxis.revStepperSRamp(6.4, 1, 6 );
  }

  if(dir == "i"){
    for(int i = 0; i < 4; i++){
      zAxis.servo_to(4.3 * 1836, -4.3* 1836, 100, 0.4, 0.1);
    delay(200);
    xAxis.revStepperSRamp(6.4, -1, 6 );
    delay(200);

    
    
    zAxis.servo_to(-0.2 * 1836, 0.2 * 1836, 100, 0.4, 0.1);
    delay(200);
    xAxis.revStepperSRamp(6.4, 1, 6 );
    delay(200);

    long currPosition1 = d1.myEnc.read();
    long currPosition2 = d2.myEnc.read();
    long tick1 = 0.68 * 792 + currPosition1;
    long tick2 = -0.68 * 792 + currPosition2;
    driveTrain.servo_to_no_correction(tick1, tick2, 200, 1.9, 0.1, false);
    delay(200);
    tick1 = 2.0 * 792 + currPosition1;
    tick2 = -2.0 * 792 + currPosition2;
    driveTrain.servo_to_no_correction(tick1, tick2, 140, 0.9, 0.1, true);
    delay(200);
    }
    
  }
  if(dir == "di"){
    zAxis.servo_to(-0.2 * 1836, 0.2 * 1836, 100, 0.4, 0.1);
    delay(200);
    for(int i = 0; i < 1; i++){
      drive_till_edge(-90, 2000);
      delay(500);                                                                                                                                                                         
      // long currPosition1 = d1.myEnc.read();
      // long currPosition2 = d2.myEnc.read();
      // long tick1 = -0.18 * 792 + currPosition1;
      // long tick2 = 0.18 * 792 + currPosition2;
      // driveTrain.servo_to_no_correction(tick1, tick2, 200, 4.5, 0.1, false);

      xAxis.revStepperSRamp(6.4, -1, 6 );//frame back
      delay(200);

      zAxis.servo_to(4.3 * 1836, -4.3* 1836, 100, 0.4, 0.1);//frame down
      delay(200);

      xAxis.revStepperSRamp(6.4, 1, 6 );//bot back
      delay(200);

      zAxis.servo_to(-0.2 * 1836, 0.2 * 1836, 100, 0.4, 0.1);//bot down
      delay(200);
      
      tick1 = 2.0 * 792 + currPosition1;
      tick2 = -2.0 * 792 + currPosition2;
      driveTrain.servo_to_no_correction(tick1, tick2, 140, 0.9, 0.1, true);//forward to square
      delay(200);
    }

    
  }
  if(dir == "?"){
    Serial.print("Votage is: ");
    Serial.println(get_voltage());
  }
  if(dir == "dte"){
    Serial.println("poi");
    drive_till_edge(rev, 2000);
  }
  if(dir == "odo"){
    Serial.print("x, y, theta");
    Serial.println(driveTrain.dx);
    Serial.println(driveTrain.dy);
    Serial.println(driveTrain.dtheta);
  }
}

// void cmd_servo(){
//   Serial.println("Enter M1 revolution:");
//   while (Serial.available() == 0){
//     delay(10);
//   }
//   String cmd = Serial.readString();
//   Serial.println(cmd);
//   char dir = cmd[0];
//   double rev =cmd.substring(1).toFloat();
  
//   currPosition = s1.myEnc.read();
//   int tick = rev * 792 + currPosition;
//   if(dir == 'w'){
//     s1.servo_to(tick, 160, 0.2, 0.1);
//   }

//   currPosition2 = s2.myEnc.read();
//   int tick2 = rev * 792 + currPosition2;
//   if(dir == 'w'){
//     s2.servo_to(tick2, 160, 0.2, 0.1);
//   }
// }
void loop() {
  cmd_servo_multi();



}
