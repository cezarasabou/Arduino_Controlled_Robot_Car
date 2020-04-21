#include <avr/io.h>
#include <avr/interrupt.h>
#define echoPin 7 //echo
#define trigPin 8 // Trigger
#define echoPin1 3 //echo
#define trigPin1 4 // Trigger
#define leds 9 // the lights
#define buzzer 13 //buzzer
#include "timer.h"

#define backward_right 0
#define forward_left 1

int carState;


byte cmd[4];
unsigned char sreg;
long duration, distance;
long duration1, distance1;
boolean tooClose = false;
boolean parkingState = false;


Timer t;
unsigned long counterStart = 0;
unsigned long PARK_TIME = 1500; // 10 sec

void setup() {

  pinMode(11, OUTPUT); // pins for timer 1
  pinMode(12, OUTPUT);

  pinMode(5, OUTPUT); // pins for timer 3
  pinMode(2, OUTPUT);

  pinMode(9, OUTPUT); // pins for the lights
  pinMode(13, OUTPUT); // pin for the buzzer

  //set the pins for proximity sensor
  pinMode(trigPin, OUTPUT); // Trigger PIN
  pinMode(echoPin, INPUT); // Echo PIN

  pinMode(trigPin1, OUTPUT); // Trigger PIN
  pinMode(echoPin1, INPUT); // Echo PIN

//  //setTheCounters();
  t.setInterval(50); 
  t.setCallback(updateTheDistance);
   t.start();
  //t.setInterv(50, updateTheDistance);

  carState = backward_right;
 
  Serial.begin(9600);
  Serial1.begin(9600);
}


void inputProcessing() {
  /**
  Serial.println(cmd[0]);
  Serial.println(cmd[1]);
  Serial.println(cmd[2]);
  Serial.println(cmd[3]);
  Serial.println(distance);
  Serial.println();
  **/
  if (cmd[1] > 0 && cmd[0] == 241 && tooClose == false) { // move forward
    //Serial.println("Moving forward");
    analogWrite(11, 255);
    analogWrite(12, 0);
  }

  if (cmd[1] > 0 && cmd[0] == 242) { // move backward
    //Serial.println("Moving backward");
    analogWrite(11, 0);
    analogWrite(12, 255);
    tooClose = false;
    analogWrite(13, 0);
  }

  if (cmd[1] == 0 && cmd[0] == 243) { // don't move
   //Serial.println("Not moving");
    analogWrite(11, 0);
    analogWrite(12, 0);
    analogWrite(2, 0);
    analogWrite(5, 0);
  }

  if (cmd[2] > 88) { // move wheels right
    //Serial.println("Wheels right");
    analogWrite(2, 255);
    analogWrite(5, 0);
  }

  if (cmd[2] < 88) { // move wheels left
    //Serial.println("Wheels left");
    analogWrite(2, 0);
    analogWrite(5, 255);
  }
}

void inputProcessing_lights() {

//  Serial.println(cmd[0]);
//  Serial.println(cmd[1]);
//  Serial.println(cmd[2]);
//  Serial.println(cmd[3]);
//  Serial.println(distance);
//  Serial.println();
  
  if (cmd[1] > 0 && cmd[0] == 241 ) { // move forward
    //Serial.println("Moving forward");
    analogWrite(11, 255);
    analogWrite(12, 0);
    tooClose = false;
  }

  if (cmd[1] > 0 && cmd[0] == 242&& tooClose == false) { // move backward
    //Serial.println("Moving backward");
    analogWrite(11, 0);
    analogWrite(12, 255);
    
    analogWrite(13, 0);
  }

  if (cmd[1] == 0 && cmd[0] == 243) { // don't move
   //Serial.println("Not moving");
    analogWrite(11, 0);
    analogWrite(12, 0);
    analogWrite(2, 0);
    analogWrite(5, 0);
  }

  if (cmd[2] > 88) { // move wheels right
    //Serial.println("Wheels right");
    analogWrite(2, 150);
    analogWrite(5, 0);
  }

  if (cmd[2] < 88) { // move wheels left
    //Serial.println("Wheels left");
    analogWrite(2, 0);
    analogWrite(5, 150);
  }

  // read lights control
  if(bitRead(cmd[3], 7)) {
    analogWrite(9, 153);
  }
  else {
    analogWrite(9, 0);
  }

  // read buzzer control
  if(bitRead(cmd[3], 5)) {
    analogWrite(13, 153);
  }
  else {
    analogWrite(13, 0);
  }

  // read park control
  if(bitRead(cmd[3], 6)) {
    Serial.print("The side distance: ");
    Serial.println(distance1);
    parkCar();
  }
}

void parkCar(){
  Serial.println(carState);
  if(carState == backward_right){
    //pas 1 : maxim dreapta
    analogWrite(2, 150);
    analogWrite(5, 0);
    //backward
     analogWrite(11, 0);
    analogWrite(12, 255);

    //Serial.println("Backward right");
    
    if(distance < 25 ){
      carState = forward_left;
      counterStart = millis();
    }
  }
    
  if(carState == forward_left){
          //pas 1 : maxim stanga
      analogWrite(2, 0);
      analogWrite(5, 150);
      //forward
      analogWrite(11, 255);
      analogWrite(12, 0);
      //Serial.println("Forward left");
      //Serial.print("Time: ");
      //Serial.println(millis() - counterStart);
      if((millis() - counterStart) >= PARK_TIME){
          analogWrite(11, 0);
          analogWrite(12, 0);
          Serial.println("Changed state to backward right");
          carState = backward_right;
      }
   }
}

long microsecondsToCentimeters (long microseconds) {
 // The speed of sound is 340 m/s or 29 microseconds per centimeter
 // The ping travels forth and back, so, the distance is half the distance traveled
 return microseconds / 29 / 2;
}

void updateTheDistance() {

  digitalWrite (trigPin, LOW);
  //digitalWrite (trigPin1, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  //digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);

  digitalWrite (trigPin, LOW);
  //digitalWrite (trigPin1, LOW);

  //Now, lets read the read the bounced wave
  duration = pulseIn(echoPin, HIGH);
  //duration1 = pulseIn(echoPin1, HIGH);

  //calculate the distance
  distance = microsecondsToCentimeters(duration);
  //distance1 = microsecondsToCentimeters(duration1);

  //Serial.println(distance1);

  if(distance < 10) {
    analogWrite(11, 0);
    analogWrite(12, 0);
    tooClose = true;
    analogWrite(13, 20 );
  }
  else{
    analogWrite(13, 0);
  }
}

void loop() {

  t.update();

  if (Serial1.available() > 4) {
    cmd[0] = Serial1.read();//Direction
    cmd[1] = Serial1.read();//Speed
    cmd[2] = Serial1.read();//Angle
    cmd[3] = Serial1.read();//Lights and buttons states

    inputProcessing_lights();
  }
}
