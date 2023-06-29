#include <analogWrite.h>
#include <ESP32Servo.h>

Servo frontMotor;
class SH_Motor{
  public:void moveBackward(){
    stopMove();
    digitalWrite(27, HIGH);
    digitalWrite(23, HIGH);
  }
  public:void moveForward(){
    stopMove();
    frontMotor.write(28);
    digitalWrite(26, HIGH);
    digitalWrite(19, HIGH);
  }
  public:void moveLeft(){
    frontMotor.write(15);
    delay(500);
  }
  public:void moveRight(){
    frontMotor.write(51);
    delay(500);
  }
  public:void stopMove(){
    digitalWrite(27, LOW);
    digitalWrite(23, LOW);
    digitalWrite(26, LOW);
    digitalWrite(19, LOW);
    delay(100);
  }
  
  public:void setMoveSpeed(int Speed){
    analogWrite(5, Speed);
  }
};
void setup() {
  pinMode(27, OUTPUT); // IN1
  pinMode(26, OUTPUT); // IN2
  pinMode(19, OUTPUT); // IN3
  pinMode(23, OUTPUT); // IN4
  frontMotor.attach(18); // Front tyer servo

  SH_Motor motor;
  motor.setMoveSpeed(255);
  motor.moveForward();
  delay(10000);
  motor.stopMove();
}

void loop() {
  // put your main code here, to run repeatedly:
  
}
