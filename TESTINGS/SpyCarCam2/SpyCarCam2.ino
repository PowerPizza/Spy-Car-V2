#include "esp_camera.h"
#include <WiFi.h>

const char* ssid = "HKUCAK";
const char* password = "PROPLAYERS";

#define CAMERA_MODEL_AI_THINKER

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

WiFiServer websvr(80);
void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(600);
  }

  Serial.println();
  Serial.print("Connected at : "); Serial.println(WiFi.localIP());
  Serial.println("Config camera start!");

  websvr.begin();
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; // JPEG
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  Serial.println("Step config 1 done. entering second");
  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera failed with error 0x%x\n", err);
    Serial.println("Restart");
  }
  Serial.println("Config done");

//  pinMode(4, OUTPUT);
//  digitalWrite(4, HIGH);
//  delay(2000);
//  digitalWrite(4, LOW);
}

String header = "";
void loop() {
  WiFiClient mycli = websvr.available();
  if (mycli){
    while (mycli.connected()){
      if (mycli.available()){
        char c = mycli.read();
        header += c;

        if (c == '\n'){
          if (header.indexOf("GET /img") >= 0){
            camera_fb_t * fb = NULL;
            fb = esp_camera_fb_get();
            if (fb){
              Serial.println("Capture...");
            }
            else{
              Serial.println("fail");
            }
//            char myd[fb->len+2] = {'\0'};
//            int i = 0;
//            for (; i < 100; i++){
//              myd[i] = (char)fb->buf[i];
//            }
//            myd[i+1] = '\0';
            char a[200];
            for (int i = 0; i < 198; i++){
              a[i] = (char)fb->buf[i];
            }
            Serial.println((char*)fb->buf);
            
//            Serial.print("printed all. ");

            mycli.println("HTTP/1.1 200 OK");
            mycli.println("Content-type:image/jpeg");
            mycli.println("Connection: close");
            mycli.println();
            mycli.println(a);
            mycli.println();

            esp_camera_fb_return(fb);
            Serial.println("done");
          }
          
          mycli.println("HTTP/1.1 200 OK");
          mycli.println("Content-type:text/html");
          mycli.println("Connection: close");
          mycli.println();
          mycli.println("<html lang=\"en\"><head><meta charset=\"UTF-8\"><meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>hi</title></head><body><button onclick=\"getImg()\">Get img</button><script>function getImg(){let xmlsvr = new XMLHttpRequest();xmlsvr.open(\"GET\", \"/img\");xmlsvr.responseType = \"blob\";xmlsvr.onload = ()=>{console.log(xmlsvr.response);}xmlsvr.send();}</script></body></html>");
          mycli.println();
          break;
        }
      }
    }
  }
}
