#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_server.h"

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

#define ssid "vivo 1811"
#define password "123456789"

bool ws_run = false;
int wsLive_val = 0;
int last_wslive_val;

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t stream_httpd = NULL;
httpd_handle_t camera_httpd = NULL;

static const char PROGMEM INDEX_HTML[] = "rawliteral{<html><head><title>My document</title></head><body><img src=\"\" id=\"vdstream\"><script>window.onload = document.getElementById(\"vdstream\").src = window.location.href.slice(0, -1) + \":81/stream\";function cimg(){let xmls = new XMLHttpRequest();xmls.open(\"GET\", \"/img?go=\", true);xmls.send();}</script></body></html>}rawliteral";

static esp_err_t index_handler(httpd_req_t *req){
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

static esp_err_t stream_handler(httpd_req_t *req){
  ws_run = true;
  Serial.print("stream server running on core :");
  Serial.println(xPortGetCoreID());

  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  String part_buf(64);

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK){
    return res;
  }

  while (true){
    ws_run = true;
    fb = esp_camera_fb_get();
    if (!fb){
      Serial.println("Capture failed!");
      res = ESP_FAIL;
    }
    else{
      Serial.println(fb->width);
      Serial.println(fb->height);

      if (fb->width > 200){
        if (fb->format != PIXFORMAT_JPEG){
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if (!jpeg_converted){
            Serial.println("JPG CONVERSION FAIL.");
            res = ESP_FAIL;
          }
        }
      }
      else{
        _jpg_buf_len = fb->len;
        _jpg_buf = fb->buf;
      }
    }

    if (res == ESP_OK){
      size_t hlen = snprintf((char *)part_buf.charAt(0), 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf.charAt(0), hlen);
    }
    if (res == ESP_OK){
      res = httpd_resp_send_chunk(req, (const char*)_jpg_buf, _jpg_buf_len);
    }
    if (res == ESP_OK){
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if (fb){
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    }
    else if (_jpg_buf){
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if (res != ESP_OK){
      break;
    }
    wsLive_val++;
    if (wsLive_val > 999) wsLive_val = 0;
  }
  return res;
}

static esp_err_t cmd_handler(httpd_req_t *req){
  char* buf;
  size_t buf_len;
  char variable[32] = {0,};

  if (buf_len > 1){
    buf = (char *)malloc(buf_len);
    if (!buf){
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK){
      if (httpd_query_key_value(buf, "go", variable, sizeof(variable)) == ESP_OK){
      }
      else{
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
      }
      free(buf);
    }
    else{
      httpd_resp_send_404(req);
      return ESP_FAIL;
    }
  }
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "'");
  return httpd_resp_send(req, NULL, 0);
}


void startCameraWebServer(){
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;

  httpd_uri_t index_uri = {
    .uri = "/",
    .method = "HTTP_GET",
    .handler = index_handler,
    .user_ctx = NULL
  };

  httpd_uri_t stream_uri = {
    .uri = "/stream",
    .method = "HTTP_GET",
    .handler = stream_handler,
    .user_ctx = NULL
  };

  if (httpd_start(&index_httpd, &config) == ESP_OK){
    httpd_register_uri_handler(index_httpd, &index_uri);
  }

  config.server_port += 1;
  config.ctrl_port += 1;

  if (httpd_start(&stream_httpd, &config) == ESP_OK){
    httpd_register_uri_handler(stream_httpd, &stream_httpd);
  }

  Serial.println("");
  Serial.println("Camera running");
  Serial.println(WiFi.localIP());
}

void setup(){
  WRITE_PERI_REG(RTC_CNIL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  Serial.println("Config and init the camera start ---");
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
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 15;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA);

  WiFi.begin(ssid, password);
  digitalWrite(LED_OnBoard, HIGH);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  startCameraServer();

  Serial.print(WiFi.localIP());
}

void loop(){
  delay(10000);
}
