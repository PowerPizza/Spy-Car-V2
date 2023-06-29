#include "esp_http_server.h"
#include <WiFi.h>
#include "esp_camera.h"
#include <analogWrite.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

char *ssid = "HKUCAK";
char *password = "PROPLAYERS";

esp_err_t homePage(httpd_req_t *req){
  char resp[] = "Welcome to home /image and /live";
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_send(req, resp, sizeof(resp));
  return ESP_OK;
}

esp_err_t image(httpd_req_t *req){
  camera_fb_t *fb = NULL;
  fb = esp_camera_fb_get();
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  
  if (!fb){
    esp_camera_fb_return(fb);
    char * resp_msg = "failed to capture image!";
    httpd_resp_send(req, resp_msg, sizeof(resp_msg));
    return ESP_FAIL;
  }
  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_send_chunk(req, (char *)fb->buf, fb->len);
  esp_camera_fb_return(fb);
  httpd_resp_send_chunk(req, NULL, 0);
  return ESP_OK;
}

esp_err_t live(httpd_req_t *req){
  char * boundry = "\r\n--frame\r\n";
  char * fileType = "Content-Type: image/jpeg\r\n\r\n";
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  
  while (true){
    fb = esp_camera_fb_get();
    if (!fb){
      esp_camera_fb_return(fb);
      char resp[] = "failed to capture image!";
      httpd_resp_send(req, resp, sizeof(resp));
      return ESP_FAIL;
    }
    
    else{
      res = httpd_resp_set_type(req, "multipart/x-mixed-replace;boundary=\"--frame\"");
      res = httpd_resp_send_chunk(req, boundry, strlen(boundry));
      res = httpd_resp_send_chunk(req, fileType, strlen(fileType));
      res = httpd_resp_send_chunk(req, (char *)fb->buf, fb->len);
      esp_camera_fb_return(fb);
    }
    if (res != ESP_OK){
      break;
    }
  }
  return ESP_OK;
}

esp_err_t change_settings(httpd_req_t *req){
  char data_[100];
  int recv_sz = min(req->content_len, sizeof(data_));
  int rcv_status = httpd_req_recv(req, data_, recv_sz);
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  
  if (rcv_status <= 0){
    char resp_[] = "Fail";
    httpd_resp_send(req, resp_, sizeof(resp_));
    return ESP_FAIL;
  }
  else{
    String new_data = data_;
    // settings pattern -- [resolution, spacial_effect, filters, brightness, contrast, saturation, quality]
    String settings[] = {"null", "null", "null", "null", "null", "null", "null"};
    int extract_from = 0;
    int idx = 0;
    while (idx != 7){
      settings[idx] = new_data.substring(extract_from, new_data.indexOf(',', extract_from));
      extract_from += settings[idx].length()+1;
      idx += 1;
    }

    sensor_t * s = esp_camera_sensor_get();

    if (settings[0] == "QVGA"){
      s->set_framesize(s, FRAMESIZE_QVGA);
    }
    else if (settings[0] == "CIF"){
      s->set_framesize(s, FRAMESIZE_CIF);
    }
    else if (settings[0] == "VGA"){
      s->set_framesize(s, FRAMESIZE_VGA);
    }
    else if (settings[0] == "SVGA"){
      s->set_framesize(s, FRAMESIZE_SVGA);
    }
    else if (settings[0] == "XGA"){
      s->set_framesize(s, FRAMESIZE_XGA);
    }
    else if (settings[0] == "SXGA"){
      s->set_framesize(s, FRAMESIZE_SXGA);
    }
    else if (settings[0] == "UXGA"){
      s->set_framesize(s, FRAMESIZE_UXGA);
    }
    s->set_special_effect(s, settings[1].toInt());
    s->set_wb_mode(s, settings[2].toInt());
    s->set_brightness(s, settings[3].toInt());
    s->set_contrast(s, settings[4].toInt());
    s->set_saturation(s, settings[5].toInt());
    s->set_quality(s, settings[6].toInt());
  }
  char resp_[] = "OK";
  httpd_resp_send(req, resp_, sizeof(resp_));
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

void ctc_WiFi(){
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED){
    Serial.println("connecting");
    delay(600);
  }
  Serial.println("READY");
}

int port = 80;
esp_err_t cam_connected;
void setup() {
  Serial.begin(115200);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
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
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 30;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  cam_connected = esp_camera_init(&config);
  if (cam_connected!= ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", cam_connected);
    return;
  }
  
  // wifi begin
  ctc_WiFi();

  // webserver end points
  httpd_uri_t homePage_{
    .uri = "/",
    .method = HTTP_GET,
    .handler = homePage,
    .user_ctx = NULL
  };

  httpd_uri_t live_{
    .uri = "/live",
    .method = HTTP_GET,
    .handler = live,
    .user_ctx = NULL
  };

  httpd_uri_t image_{
    .uri = "/image",
    .method = HTTP_GET,
    .handler = image,
    .user_ctx = NULL
  };

  httpd_uri_t change_settings_{
    .uri = "/change_settings",
    .method = HTTP_POST,
    .handler = change_settings,
    .user_ctx = NULL
  };

  httpd_config_t config_http = HTTPD_DEFAULT_CONFIG();
  httpd_handle_t server = NULL;

  if (httpd_start(&server, &config_http) == ESP_OK) {
    httpd_register_uri_handler(server, &homePage_);
    httpd_register_uri_handler(server, &live_);
    httpd_register_uri_handler(server, &image_);
    httpd_register_uri_handler(server, &change_settings_);
  }
  port = config_http.server_port;
//  Serial.print("<IP>http://"); Serial.print(WiFi.localIP()); Serial.printf(":%d</IP>", config_http.server_port);
}

void loop() {
  if (cam_connected != ESP_OK){
    Serial.printf("Camera init failed with error 0x%x\n", cam_connected);
    delay(600);
    return;
  }
  else if (WiFi.status() != WL_CONNECTED){
    ctc_WiFi();
  }
  Serial.print("<IP>http://"); Serial.print(WiFi.localIP()); Serial.printf(":%d</IP>\n", port);
  delay(600);
}
