#include <ESP32Servo.h>

Servo myservo;
Servo bldc1;
Servo bldc2;

void calibrateESC(Servo motor){
  motor.write(180);
  delay(1000);
  motor.write(0);
  delay(2000);
}

void setup() {
  bldc1.attach(13);
  bldc2.attach(23);
  bldc1.write(180);
  bldc2.write(180);
  delay(1000);
  bldc1.write(0);
  bldc2.write(0);
  delay(2000);

  myservo.attach(21);
  Serial.begin(115200);
  Serial.println("servo");
}

void loop() {
  bldc1.write(0); bldc2.write(0);
  myservo.write(0);
  delay(1000);
  bldc1.write(180); bldc2.write(180);
  delay(10000);

  bldc1.write(0); bldc2.write(0);
  myservo.write(180);
  delay(1000);
  bldc1.write(180); bldc2.write(180);
  delay(10000);

  //Serial.print(bldc1.read());Serial.println(bldc2.read());
}
