#include <Servo.h>

class Cam_Rotor{
  private:Servo x_axis;
  private:Servo y_axis;
  public:void init(int servo_x, int servo_y){
    x_axis.attach(servo_x);
    y_axis.attach(servo_y);
    x_axis.write(0);
    y_axis.write(90);
  }

  public:String rotateX(int deg){
    // 0 to 180
    if (deg < 0 && deg > 180){
      return "OUT_OF_BOX";
    }
    x_axis.write(deg);
  }
  public:String rotateY(int deg){
    // 70 to 180
    if (deg < 70 && deg > 180){
      return "OUT_OF_BOX";
    }
    y_axis.write(deg);
  }
  public:int getY(){
    return y_axis.read();
  }
  public:int getX(){
    return x_axis.read();
  }
};

Cam_Rotor rotor;
void setup() {
  Serial.begin(9600);
  Serial.println("start");
  rotor.init(6, 7);
}

void loop() {
  if (Serial.available()){
    int deg_ = Serial.readStringUntil('\r').toInt();
    if (deg_ > 0){
      Serial.println(rotor.getY());
      rotor.rotateY(deg_);
    }
  }
  delay(600);
}
