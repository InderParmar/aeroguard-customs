// ESP32 + OV2640 + Firebase uploader with push-button start/stop
#define CONFIG_CAMERA_TASK_STACK_SIZE 16384  // increase camera driver task stack

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp_camera.h"
#include <driver/rmt.h>
#include "secrets.h"

#ifndef WIFI_SSID
#error "Create secrets.h with WIFI_SSID, WIFI_PASS, API_KEY, FB_EMAIL, FB_PASSWORD, BUCKET_NAME"
#endif

// Push-button (4-leg tactile). Wire one side to GND, the opposite side to GPIO12.
#define BUTTON_PIN 12

// Camera <-> ESP32 wiring
#define PWDN_GPIO_NUM   -1
#define RESET_GPIO_NUM  -1
#define XCLK_GPIO_NUM   14
#define SIOD_GPIO_NUM   21
#define SIOC_GPIO_NUM   22
#define Y2_GPIO_NUM     4
#define Y3_GPIO_NUM     5
#define Y4_GPIO_NUM     18
#define Y5_GPIO_NUM     19
#define Y6_GPIO_NUM     26
#define Y7_GPIO_NUM     27
#define Y8_GPIO_NUM     16
#define Y9_GPIO_NUM     13
#define VSYNC_GPIO_NUM  25
#define HREF_GPIO_NUM   23
#define PCLK_GPIO_NUM   32

// Generate ~20 MHz XCLK on GPIO14 via RMT
static void init_xclk_rmt() {
  rmt_config_t cfg;
  cfg.channel       = RMT_CHANNEL_0;
  cfg.gpio_num      = (gpio_num_t)XCLK_GPIO_NUM;
  cfg.clk_div       = 1;          // 80 MHz / 1
  cfg.mem_block_num = 1;
  cfg.rmt_mode      = RMT_MODE_TX;
  cfg.tx_config.loop_en        = true;
  cfg.tx_config.carrier_en     = false;
  cfg.tx_config.idle_output_en = true;
  cfg.tx_config.idle_level     = RMT_IDLE_LEVEL_LOW;

  rmt_config(&cfg);
  rmt_driver_install(cfg.channel, 0, 0);

  // 2 ticks high + 2 ticks low => 80 MHz / 4 = 20 MHz square
  rmt_item32_t item = { .duration0 = 2, .level0 = 1,
                        .duration1 = 2, .level1 = 0 };
  rmt_write_items(cfg.channel, &item, 1, true);
}

// Camera driver config
static camera_config_t camera_config = {
  .pin_pwdn       = PWDN_GPIO_NUM,
  .pin_reset      = RESET_GPIO_NUM,
  .pin_xclk       = -1,                  // RMT provides XCLK
  .pin_sccb_sda   = SIOD_GPIO_NUM,
  .pin_sccb_scl   = SIOC_GPIO_NUM,
  .pin_d7         = Y9_GPIO_NUM,
  .pin_d6         = Y8_GPIO_NUM,
  .pin_d5         = Y7_GPIO_NUM,
  .pin_d4         = Y6_GPIO_NUM,
  .pin_d3         = Y5_GPIO_NUM,
  .pin_d2         = Y4_GPIO_NUM,
  .pin_d1         = Y3_GPIO_NUM,
  .pin_d0         = Y2_GPIO_NUM,
  .pin_vsync      = VSYNC_GPIO_NUM,
  .pin_href       = HREF_GPIO_NUM,
  .pin_pclk       = PCLK_GPIO_NUM,
  .xclk_freq_hz   = 0,                  
  .ledc_timer     = LEDC_TIMER_0,    
  .ledc_channel   = LEDC_CHANNEL_0,      
  .pixel_format   = PIXFORMAT_JPEG,
  .frame_size     = FRAMESIZE_VGA,
  .jpeg_quality   = 12,
  .fb_count       = 1,
  .fb_location    = CAMERA_FB_IN_DRAM,   // more stable without PSRAM
  .grab_mode      = CAMERA_GRAB_LATEST
};

static String idToken;

// Sign in to Firebase to get idToken
static String firebaseSignIn() {
  HTTPClient http;
  String url = String("https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=") + API_KEY;

  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  DynamicJsonDocument payload(256);
  payload["email"]             = FB_EMAIL;
  payload["password"]          = FB_PASSWORD;
  payload["returnSecureToken"] = true;

  String body;
  serializeJson(payload, body);

  int code = http.POST(body);
  if (code == 200) {
    DynamicJsonDocument resp(512);
    deserializeJson(resp, http.getString());
    http.end();
    return resp["idToken"].as<String>();
  }

  Serial.printf("Auth failed, HTTP %d\n", code);
  http.end();
  return "";
}

// Capture a frame and upload to Firebase Storage
static void captureAndUploadImage() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Frame capture failed");
    return;
  }

  String filename      = "img_" + String(millis()) + ".jpg";
  String folderAndFile = "images%2F" + filename;
  String uploadURL     = String("https://firebasestorage.googleapis.com/v0/b/")
                         + BUCKET_NAME
                         + "/o?uploadType=media&name="
                         + folderAndFile;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, uploadURL);
  http.addHeader("Content-Type", "image/jpeg");
  http.addHeader("Authorization", "Bearer " + idToken);

  int resp = http.sendRequest("POST", fb->buf, fb->len);
  if (resp == 200) {
    Serial.println("Image uploaded");
  } else {
    Serial.printf("Upload failed, HTTP %d\n", resp);
  }

  http.end();
  esp_camera_fb_return(fb);
}

// Capture loop control
static bool capturing = false;
static bool lastBtn   = HIGH;          // using INPUT_PULLUP
static unsigned long lastSnap = 0;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Wi-Fi
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println(" ok");

  // Firebase auth
  idToken = firebaseSignIn();
  if (idToken.isEmpty()) {
    Serial.println("No token. Halt.");
    while (true) delay(1000);
  }

  // Camera
  init_xclk_rmt();
  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed (0x%x)\n", err);
    while (true) delay(1000);
  }
  Serial.println("Camera ready");
}

void loop() {
  // Toggle capturing on button press (active-low), with simple debounce.
  bool btn = digitalRead(BUTTON_PIN);
  if (btn != lastBtn) {
    delay(50);
    btn = digitalRead(BUTTON_PIN);
    if (btn != lastBtn && btn == LOW) {
      capturing = !capturing;
      Serial.printf("Capture: %s\n", capturing ? "ON" : "OFF");
      if (capturing) lastSnap = millis();
    }
    lastBtn = btn;
  }

  // If capturing, take a photo every 3 seconds.
  if (capturing && (millis() - lastSnap >= 3000UL)) {
    lastSnap = millis();
    captureAndUploadImage();
  }

  delay(10); // small idle to keep button responsive
}
