#include "esp_http_server.h"
#include <WiFi.h>
#include "esp_camera.h"
#include <analogWrite.h>

char *ssid = "HKUCAK";
char *password = "PROPLAYERS";
int stream_status = 0;

esp_err_t homePage(httpd_req_t *req){
  char resp[] = "Welcome to home";
  httpd_resp_send(req, resp, sizeof(resp));
  return ESP_OK;
}

esp_err_t adminPage(httpd_req_t *req){
  char resp[] = "<h1>You are admin!</h1>";
  httpd_resp_send(req, resp, sizeof(resp));
  return ESP_OK;
}

esp_err_t loginPage(httpd_req_t *req){
  char resp[] = "LOGIN Here! IDK";
  httpd_resp_send(req, resp, sizeof(resp));
  return ESP_OK;
}

esp_err_t image(httpd_req_t *req){
  char * boundry = "\r\n--frame\r\n";
  char * fileType = "Content-Type: image/jpeg\r\n\r\n";
  camera_fb_t *fb = NULL;
  while (true){
    fb = esp_camera_fb_get();
    if (!fb){
      esp_camera_fb_return(fb);
      Serial.println("Unable to capture");
      char resp[] = "failed to capture image";
      httpd_resp_send(req, resp, sizeof(resp));
      return ESP_FAIL;
    }
    
    else{
      httpd_resp_set_type(req, "multipart/x-mixed-replace;boundary=\"--frame\"");
      httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
      httpd_resp_send_chunk(req, boundry, strlen(boundry));
      httpd_resp_send_chunk(req, fileType, strlen(fileType));
      httpd_resp_send_chunk(req, (char *)fb->buf, fb->len);
      esp_camera_fb_return(fb);
      Serial.printf("image sended.\n");
    }
  }
  return ESP_OK;
}

esp_err_t post_handler(httpd_req_t *req){
  char data_[100];
  int recvSz = min(req->content_len, sizeof(data_));
  int ret = httpd_req_recv(req, data_, recvSz);
  if (ret <= 0){
    if (ret == HTTPD_SOCK_ERR_TIMEOUT){
      httpd_resp_send_408(req);
    }
    return ESP_FAIL;
  }
  else{
    Serial.println(data_);
  }
  char resp[] = "We got it";
  httpd_resp_send(req, resp, sizeof(resp));
  return ESP_OK;
}

esp_err_t flashON_OFF(httpd_req_t *req){
  char data_[5];
  int recieved = httpd_req_recv(req, data_, 4);
  if (recieved > 0){
    if (String(data_) == "on"){
      analogWrite(4, 200);
    }
    else if (String(data_) == "off"){
      analogWrite(4, 0);
    }
    Serial.println(data_);
  }
  httpd_resp_send(req, NULL, 0);
  return ESP_OK;
}

// CAMERA CONFIGS -----------------
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

void setup() {
//  pinMode(4, OUTPUT);

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
  config.pixel_format = PIXFORMAT_JPEG;
  
  if(psramFound()){
    Serial.println("PSRAM FOUND");
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    Serial.println("PSRAM NOT FOUND");
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  
  // wifi begin
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(600);
  }
  Serial.println();

  // webserver end points
  httpd_uri_t homePage_{
    .uri = "/",
    .method = HTTP_GET,
    .handler = homePage,
    .user_ctx = NULL
  };

  httpd_uri_t adminPage_{
    .uri = "/admin",
    .method = HTTP_GET,
    .handler = adminPage,
    .user_ctx = NULL
  };

  httpd_uri_t loginPage_{
    .uri = "/login",
    .method = HTTP_GET,
    .handler = loginPage,
    .user_ctx = NULL
  };

  httpd_uri_t image_{
    .uri = "/image",
    .method = HTTP_GET,
    .handler = image,
    .user_ctx = NULL
  };
  httpd_uri_t flash_control_{
    .uri = "/flash",
    .method = HTTP_POST,
    .handler = flashON_OFF,
    .user_ctx = NULL
  };
  httpd_uri_t uri_post{
    .uri = "/uri",
    .method = HTTP_POST,
    .handler = post_handler,
    .user_ctx = NULL
  };

  Serial.println("Starting web server");
  httpd_config_t config_http = HTTPD_DEFAULT_CONFIG();
  httpd_handle_t server = NULL;

  if (httpd_start(&server, &config_http) == ESP_OK) {
    httpd_register_uri_handler(server, &homePage_);
    httpd_register_uri_handler(server, &adminPage_);
    httpd_register_uri_handler(server, &loginPage_);
    httpd_register_uri_handler(server, &image_);
    httpd_register_uri_handler(server, &flash_control_);
  }
  Serial.print("Server ip "); Serial.println(WiFi.localIP());
  Serial.printf("Server running on %d port.\n", config_http.server_port);
}

void loop() {
  // put your main code here, to run repeatedly:

}
