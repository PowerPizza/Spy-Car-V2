// --------------- includes -------------
#include <WiFi.h>
#include <analogWrite.h>
#include <ESP32Servo.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>
#include <ESP32Servo.h>

// -------------------- Extra functions classes and variables macros ------------------
char* ssid = "HKUCAK";
char* password = "PROPLAYERS";
#define SCREEN_W 128
#define SCREEN_H 32
#define SCREEN_RESET -1
#define SCREEN_ADDR 0x3C

WiFiServer webServer(80);
Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, SCREEN_RESET);

void showBanner(){
  display.clearDisplay();
  display.setCursor(0, 0);
  display.drawFastHLine(0, 0, SCREEN_W, SSD1306_WHITE);
  display.drawFastHLine(0, SCREEN_H-1, SCREEN_W, SSD1306_WHITE);
  display.drawFastVLine(0, 0, SCREEN_H, SSD1306_WHITE);
  display.drawFastVLine(SCREEN_W-1, 0, SCREEN_H, SSD1306_WHITE);
  display.display();
  
  display.setCursor(3, 3);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  String toPtr = "MADE BY :\nTEAM S.H.";
  for (int i = 0; i < toPtr.length(); i++){
    display.print(String(toPtr.charAt(i)));
    display.display();
    if (toPtr.charAt(i) == '\n'){
      display.setTextSize(2);
      display.setCursor(7, 11);
    }
    delay(200);
  }
  delay(3000);

  display.setCursor(0, 0);
  for (int i = 0; i < SCREEN_H; i++){
    display.drawFastHLine(0, i, SCREEN_W, SSD1306_WHITE);
    display.display();
    delay(50);
  }
  display.setCursor(0, 0);
  for (int i = 0; i < SCREEN_W; i++){
    display.drawFastVLine(i, 0, SCREEN_H, SSD1306_BLACK);
    display.display();
  }
}

String CameraIP;
void cameraSetupStart(){
  Serial2.begin(115200, SERIAL_8N1, 16, 17);
  int time_out = 0;
  
  while (true){
    display.clearDisplay();
    display.setCursor(2, 2);
    display.setTextSize(1);
    display.println("CONNECTING CAMERA");
    display.setTextSize(2);
    
    time_out += 1;
    if (Serial2.available() > 0){
      String data_ = Serial2.readStringUntil('\n');
      Serial.print("Got Data : "); Serial.println(data_);
//      analogWrite(2, 200);
      if (data_.indexOf("<IP>") != -1 && data_.indexOf("</IP>") != -1){
        CameraIP = data_.substring(4).substring(0, data_.indexOf("</IP>")-4);
        break;
      }
      else if (data_.indexOf("Camera init failed") != -1){
        display.clearDisplay();
        display.setCursor(2, 2);
        display.setTextSize(1);
        display.println("Camera Init Failed!");
        display.setTextSize(2);
        display.display();
        delay(3000);
        break;
      }
//      analogWrite(2, 0);
    }
    else {
      display.clearDisplay();
      display.setCursor(2, 2);
      display.setTextSize(1);
      display.println("Waiting for serial\nconnection.");
      display.setTextSize(2);
      display.display();
      delay(600);
      continue;
    }
    
    display.print("."); display.display(); delay(100);
    display.print("."); display.display(); delay(100);
    display.print("."); display.display(); delay(100);
    display.print("."); display.display(); delay(100);
    display.print("."); display.display(); delay(100);

    if (time_out == 40){
      display.clearDisplay();
      display.setCursor(2, 2);
      display.setTextSize(1);
      display.println("Camera not found\nstarting model.");
      display.display();
      analogWrite(2, 0);
      delay(1000);
      break;
    }
  }
}

void connectWiFi(char* ssid, char* password){
  WiFi.begin(ssid, password);
  display.clearDisplay();
  display.setTextSize(2);
  
  while (WiFi.status() != WL_CONNECTED){
    display.setCursor(2, 2);
    display.print("connecting."); display.display();
    delay(200);
    display.print("."); display.display();
    delay(200);
    display.print("."); display.display();
    delay(200);
    display.print("."); display.display();
    delay(200);
    display.clearDisplay();
  }
  display.setCursor(2, 2);
  display.println("Connected");
  display.setTextSize(1);
  display.print("IP : ");
  display.println(WiFi.localIP());
  display.display();
  Serial.println("");
  Serial.println("WiFi connected!");
}

class SH_Motor{
  public:SH_Motor(){
    setMoveSpeed(100);
  }
  public:void moveBackward(){
    stopMove();
    digitalWrite(27, HIGH);
    digitalWrite(23, HIGH);
  }
  public:void moveForward(){
    stopMove();
    digitalWrite(26, HIGH);
    digitalWrite(19, HIGH);
  }
  public:void moveLeft(){
    stopMove();
    digitalWrite(23, HIGH);
    digitalWrite(26, HIGH);
  }
  public:void moveRight(){
    stopMove();
    digitalWrite(19, HIGH);
    digitalWrite(27, HIGH);
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

class Cam_Rotor{
  private:Servo x_axis;
  private:Servo y_axis;
  public:void init(int servo_x, int servo_y){
    x_axis.attach(servo_x);
    y_axis.attach(servo_y);
    x_axis.write(90);
    y_axis.write(100);
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

// >-------------------------- Main CODE -----------------------<
Cam_Rotor rotor;

void setup() {
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDR);

  showBanner();
  delay(1000);
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(2, 2);
  
  pinMode(27, OUTPUT); // IN1
  pinMode(26, OUTPUT); // IN2
  pinMode(19, OUTPUT); // IN3
  pinMode(23, OUTPUT); // IN4 and pin 5 of esp32 reserved for pwn (motor speed)
  pinMode(33, OUTPUT);

  rotor.init(15, 4);
  cameraSetupStart();

  Serial.println("servo done");
  
  connectWiFi(ssid, password);
  Serial.print("Server started at : ");
  Serial.println(WiFi.localIP());
  webServer.begin();
}


String header = "";
SH_Motor motor;

void loop() {
  WiFiClient cli = webServer.available();  // listen clients
  
  if (WiFi.status() != WL_CONNECTED){
    motor.stopMove();
    analogWrite(14, 0);
    cameraSetupStart();
    connectWiFi(ssid, password);
  }
  
  if (cli){
    while (cli.connected()){  // connected means client is serving this page.
      if (cli.available()){  // available means client is at loading screen or sending or recieving any http request of webpage.
        char c = cli.read();
        header += c;

        if (c == '\n'){
          Serial.println(header);
//          if (header.indexOf("GET /hi") >= 0){ // if str not in indexOf() so -1 if in but at 0 so 0 else > 0;
//            Serial.println("client saying hi!");
//            cli.println("Welcome to this page :)");
//            cli.println();
//            header = "";
//            break;
//          }
          if (header.indexOf("GET /symbols_arrow_up") >= 0){
            motor.moveForward();
            Serial.println("Going forword!");
          }
          else if (header.indexOf("GET /symbols_arrow_down") >= 0){
            motor.moveBackward();
            Serial.println("Going backword!");
          }
          else if (header.indexOf("GET /symbols_arrow_left") >= 0){
            motor.moveLeft();
            Serial.println("Going left!");
          }
          else if (header.indexOf("GET /symbols_arrow_right") >= 0){
            motor.moveRight();
            Serial.println("Going right!");
          }
          else if (header.indexOf("GET /symbols_arrow_down") >= 0){
            motor.moveBackward();
            Serial.println("Going backword!");
          }
          else if (header.indexOf("GET /setSpeed,") >= 0){
            motor.setMoveSpeed(header.substring(header.indexOf(",")+1, header.indexOf(",")+4).toInt());
            Serial.println("setting speed!");
          }
          else if (header.indexOf("GET /ledon") >= 0){
            analogWrite(14, 227);
          }
          else if (header.indexOf("GET /ledoff") >= 0){
            analogWrite(14, 0);
          }
          else if (header.indexOf("GET /hornOn") >= 0){
            digitalWrite(33, HIGH);
          }
          else if (header.indexOf("GET /hornOff") >= 0){
            digitalWrite(33, LOW);
          }
          else if (header.indexOf("GET /symbols_circle_target") >= 0){
            motor.stopMove();
          }
          else if (header.indexOf("GET /flashon") >= 0){
            analogWrite(32, 200);
          }
          else if (header.indexOf("GET /flashoff") >= 0){
            analogWrite(32, 0);
          }
          else if (header.indexOf("GET /setXcam_rotor,") >= 0){
            int deg = header.substring(header.indexOf(",")+1, header.indexOf(",")+4).toInt();
            rotor.rotateX(deg);
            Serial.println(deg);
          }
          else if (header.indexOf("GET /setYcam_rotor,") >= 0){
            int deg = header.substring(header.indexOf(",")+1, header.indexOf(",")+4).toInt();
            rotor.rotateY(70+deg);
            Serial.println("setting Ycam_rotor!");
          }
          else if (header.indexOf("POST /giveCamIP") >= 0){
            cli.println("HTTP/1.1 200 OK");
            cli.println("Content-type:text/html");
            cli.println("Connection: close");
            cli.println();
            cli.println(CameraIP);
            cli.println();
            header = "";
            break;
          }
          else if (header.indexOf("GET /none") >= 0){
            cli.println("HTTP/1.1 200 OK");
            cli.println("Content-type:text/html");
            cli.println("Connection: close");
            cli.println();
            cli.println("camera viewport");
            cli.println();
            header = "";
            break;
          }
          header = "";

          // Displaying webpage also say it response to client.
          cli.println("HTTP/1.1 200 OK");
          cli.println("Content-type:text/html");
          cli.println("Connection: close");
          cli.println();
          cli.println("<html lang=\"en\"><head><meta charset=\"UTF-8\"><meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><meta charset=\"UTF-8\"><title>Control Panel</title><!-- Development only --><!-- <link rel=\"stylesheet\" href=\"webPage.css\"> --><!-- For production --><link rel=\"stylesheet\" href=\"https://drive.google.com/uc?id=1Obs79z8ctScwBTC_5wYyVDWYe29qFgVn\"></head><body><div class=\"controls\"><button id=\"symbols_horn\" name=\"extraOption\" onclick=\"onClickHorn()\"></button><button class=\"forward\" id=\"symbols_arrow_up\" onclick=\"onclickArrowBtns('symbols_arrow_up')\"></button><button id=\"symbols_head_light\" name=\"extraOption\" onclick=\"onClickLight()\"></button><button class=\"left\" id=\"symbols_arrow_left\" onclick=\"onclickArrowBtns('symbols_arrow_left')\"></button><button class=\"stop\" id=\"symbols_circle_target\" onclick=\"onclickArrowBtns('symbols_circle_target')\"></button><button class=\"right\" id=\"symbols_arrow_right\" onclick=\"onclickArrowBtns('symbols_arrow_right')\"></button><div name=\"extraOption\" id=\"speed\"><span class=\"lable_speed\">SPEED</span><div class=\"value_bar\"><span class=\"changeable_text\">100</span><span></span><input type=\"range\" min=\"60\" max=\"255\" value=\"100\"><span></span></div></div><button class=\"backward\" id=\"symbols_arrow_down\" onclick=\"onclickArrowBtns('symbols_arrow_down')\"></button><button name=\"extraOption\" id=\"symbols_rotation_arrow\" onclick=\"enableCameraMode()\"></button></div><div class=\"camera_controls\"><div class=\"y_origin_rotation\"></div><button class=\"startLive\" id=\"liveStream\" onclick=\"onCameraLive()\"></button><!-- <button class=\"forward\" id=\"symbols_cam_up\" onclick=\"forCameraControls('symbols_cam_up')\"></button> --><button class=\"captureOneImage\" id=\"captureImage\" onclick=\"onCameraCapture()\"></button><!-- <button class=\"left\" id=\"symbols_cam_left\" onclick=\"forCameraControls('symbols_cam_left')\"></button> --><button class=\"stop\" id=\"symbols_cam_target\" onclick=\"on_flash()\"></button><!-- <button class=\"right\" id=\"symbols_cam_right\" onclick=\"forCameraControls('symbols_cam_right')\"></button> --><button class=\"settings\" id=\"settings_cam\" onclick=\"forCameraSettings()\"></button><!-- <button class=\"backward\" id=\"symbols_cam_down\" onclick=\"forCameraControls('symbols_cam_down')\"></button> --><button class=\"control_changer\" id=\"symbols_cam_close\" onclick=\"enableCameraMode()\"></button><div class=\"x_origin_rotation\"></div></div><div class=\"display\"><div class=\"camera\"><button class=\"fullScreenCam\" onclick=\"onFullScreenCam()\">⛶</button><img src=\"none\" alt=\"View Port\" class=\"viewPhotage\"></div></div><div class=\"cam_settings_window\"><div class=\"main_settings\"><h2>Camera Settings</h2><div class=\"options option-1-resolution\"><span>Resolution</span><select id=\"resolutions\"><option value=\"QVGA\">QVGA (320 x 240)</option><option value=\"CIF\">CIF (352 x 288)</option><option value=\"VGA\" selected>VGA (640 x 480)</option><option value=\"SVGA\">SVGA (800 x 600)</option><option value=\"XGA\">XGA (1024 x 768)</option><option value=\"SXGA\">SXGA (1280 x 1024)</option><option value=\"UXGA\">UXGA (1600 x 1200)</option></select></div><div class=\"options option-2-spacial_effect\"><span>Spacial Effect</span><select id=\"spacial_effect\"><option value=\"0\" selected>No Effect</option><option value=\"1\">Negative</option><option value=\"2\">Grayscale</option><option value=\"3\">Red Tint</option><option value=\"4\">Green Tint</option><option value=\"5\">Blue Tint</option><option value=\"6\">Sepia</option></select></div><div class=\"options option-3-filters\"><span>Filter</span><select id=\"filters\"><option value=\"0\" selected>Auto</option><option value=\"1\">Sunny</option><option value=\"2\">Cloudy</option><option value=\"3\">Office</option><option value=\"4\">Home</option></select></div><div class=\"options option-4-brightness\"><span>Brightness</span><div class=\"value_bar\"><span class=\"changeable_text\">0</span><span>-2</span><input type=\"range\" min=\"-2\" max=\"2\" value=\"0\" id=\"brightness\"><span>2</span></div></div><div class=\"options option-5-contrast\"><span>Contrast</span><div class=\"value_bar\"><span class=\"changeable_text\">0</span><span>-2</span><input type=\"range\" min=\"-2\" max=\"2\" value=\"0\" id=\"contrast\"><span>2</span></div></div><div class=\"options option-6-saturation\"><span>Saturation</span><div class=\"value_bar\"><span class=\"changeable_text\">0</span><span>-2</span><input type=\"range\" min=\"-2\" max=\"2\" value=\"0\" id=\"saturation\"><span>2</span></div></div><div class=\"options option-7-quality\"><span>Quality</span><div class=\"value_bar\"><span class=\"changeable_text\">10</span><span>10</span><input type=\"range\" min=\"10\" max=\"63\" value=\"10\" id=\"quality\"><span>63</span></div></div><button class=\"apply_changes\" onclick=\"onApplySettings()\">Apply Changes ✅</button><button class=\"close\" onclick=\"forCameraSettings()\">Close</button></div></div><!-- javascript work--><!-- Development only --><!-- <script src=\"webPage.js\"></script><script src=\"symbols.js\"></script> --><!-- For production --><script type=\"text/javascript\" src=\"https://drive.google.com/uc?id=1t_MHHDxCGf4LnITZPup1M6A_ChPCzmMb\" for_=\"icons\"></script><script src=\"https://drive.google.com/uc?id=1UvMzk9UAZBdNXkOmuxhQKA4nIAhefLzR\" for_=\"logic code\"></script></body></html>");
          cli.println();
          break;
        }
      }
    }
  }
}
