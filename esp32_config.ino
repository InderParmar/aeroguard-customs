//Indepreet Singh Parmar
//Abhi Patel 
//esp32 main file
//esp32_config.ino - THe main program running on ESP32 dev module in order to connect our camera, initialise camera, communicate with firebase cloud and FRDM board simultaneously 

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <HardwareSerial.h>
#include <SPI.h>
#include "Arducam_Mega.h"

// --- CONFIGURATION SECTION ---
// WiFi credentials for cloud communication
const char* ssid = "";
const char* password = "";

// Firebase storage bucket ID (used for direct upload via REST API)
#define FIREBASE_STORAGE_BUCKET "smart-shopping-cart-225a5.firebasestorage.app"

// Access token required to authenticate with Firebase
String ACCESS_TOKEN = "";

// --- CAMERA SETUP ---
const int CS = 5;               // Chip Select pin for Arducam Mega
Arducam_Mega myCAM(CS);        // Camera initialization

// --- UART COMMUNICATION SETUP ---
HardwareSerial MySerial(1);     // Use UART1 with RX=GPIO16, TX=GPIO17 for communication with FRDM board

// --- SYSTEM STATE FLAGS ---
bool waitingForPrediction = false;     // Set true after image is uploaded, until a prediction is received
bool systemActive = false;             // True when FRDM activates the ESP32 to begin scanning
unsigned long lastPredictionCheck = 0; // Last time prediction check was made (to limit requests)
String lastPrediction = "";            // Stores last predicted item
unsigned long lastPredictionTime = 0;  // Timestamp of the last prediction to prevent duplicates


// --- SETUP FUNCTION (runs once on boot) ---
void setup() {
  Serial.begin(115200);                       // For debug output to serial monitor
  MySerial.begin(9600, SERIAL_8N1, 16, 17);   // Initialize UART1 (with correct pins and settings)

  // Initialize the Arducam camera
  Serial.println("Initializing Camera...");
  if (myCAM.begin() != CAM_ERR_SUCCESS) {
    Serial.println("Camera init failed!");
    while(1);  // Halt if camera is not found
  }

  // Set camera parameters
  myCAM.setAutoExposure(0);
  myCAM.setAbsoluteExposure(1500);                  // Exposure time
  myCAM.setImageQuality(HIGH_QUALITY);              // Set image quality
  myCAM.setBrightness(CAM_BRIGHTNESS_LEVEL_4);      // brightness level = 4
  myCAM.setContrast(CAM_CONTRAST_LEVEL_1);          // contrast = 1
  myCAM.setSaturation(CAM_STAURATION_LEVEL_1);      // saturation = 1

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.println("Waiting for activation from FRDM...");
}


// --- IMAGE CAPTURE AND UPLOAD FUNCTION ---
void captureAndUploadImage() {
  Serial.println("Capturing Image...");

  // Prepare camera for capture
  myCAM.reset();
  delay(500); 
  myCAM.setImageQuality(HIGH_QUALITY);
  myCAM.setAutoExposure(0);
  myCAM.setAbsoluteExposure(2000);
  myCAM.setContrast(CAM_CONTRAST_LEVEL_2);
  myCAM.setSaturation(CAM_STAURATION_LEVEL_2);

  // Capture image in JPG format
  myCAM.takePicture(CAM_IMAGE_MODE_VGA, CAM_IMAGE_PIX_FMT_JPG);

  // Determine image size and allocate buffer
  int totalLength = myCAM.getTotalLength();
  uint32_t bufferSize = min((uint32_t)ESP.getFreeHeap() - 5000, (uint32_t)totalLength);
  uint8_t* imageBuffer = (uint8_t*)malloc(bufferSize);
  if (!imageBuffer) {
    Serial.println("Not enough memory to allocate buffer.");
    return;
  }

  // Load image data into the buffer
  for (int i = 0; i < bufferSize; i++) {
    imageBuffer[i] = myCAM.readByte();
  }

  // Setup HTTPS client and upload to Firebase Storage
  WiFiClientSecure client;
  client.setInsecure(); // Accept self-signed certificates

  HTTPClient http;

  // Construct Firebase Storage URL
  String fileName = "image_" + String(millis()) + ".jpg";
  String uploadURL = "https://firebasestorage.googleapis.com/v0/b/" + String(FIREBASE_STORAGE_BUCKET) + 
                    "/o?uploadType=media&name=images%2F" + fileName;

  http.begin(client, uploadURL);
  http.addHeader("Content-Type", "image/jpeg");
  http.addHeader("Authorization", "Bearer " + ACCESS_TOKEN);

  // Perform the POST request
  int httpResponseCode = http.POST(imageBuffer, bufferSize);
  if (httpResponseCode > 0) {
    Serial.println("Image Uploaded Successfully!");
    waitingForPrediction = true;
    lastPredictionCheck = millis();  // Start checking for prediction
  } else {
    Serial.println("Upload Failed!");
    Serial.println("HTTP Code: " + String(httpResponseCode));
    waitingForPrediction = false;
  }

  // Cleanup
  http.end();
  free(imageBuffer);
  myCAM.reset();
}


// --- CHECK FOR PREDICTION FROM FIREBASE DATABASE ---
void checkPrediction() {
  // Limit how often we check
  if (millis() - lastPredictionCheck < 1000) return;
  lastPredictionCheck = millis();

  // Connect securely
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  String url = "https://smart-shopping-cart-225a5-default-rtdb.firebaseio.com/predictions/latest.json";
  http.begin(client, url);

  int httpCode = http.GET();

  if (httpCode == 200) {
    String response = http.getString();

    // Parse JSON response
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, response);
    String result = doc["result"].as<String>();
    long timestamp = doc["timestamp"];

    // If result = new prediction, send it to FRDM
    if (result != lastPrediction || timestamp != lastPredictionTime) {
      lastPrediction = result;
      lastPredictionTime = timestamp;

      Serial.println("Predicted Item: " + result);
      MySerial.print(result + "\n");  // Send to FRDM over UART
      waitingForPrediction = false;   // Reset flag after prediction received
    }
  } else {
    Serial.println("Failed to get prediction! Code: " + String(httpCode));

    // If it’s been a long time without prediction, reset flag
    if (millis() - lastPredictionCheck > 10000) {
      waitingForPrediction = false;
      Serial.println("Prediction timeout, resetting...");
    }
  }

  http.end();
}


// --- MAIN LOOP FUNCTION ---
void loop() {
  // --- Wait for FRDM to activate the system (receiving 'S' after rfid tap) ---
  if (!systemActive && MySerial.available()) {
    char c = MySerial.read();
    if (c == 'S') {
      systemActive = true;
      waitingForPrediction = false;
      lastPrediction = "";
      lastPredictionTime = 0;
      Serial.println("System Activated by FRDM!");
    }
  }

  // --- Wait for FRDM to request a capture (receiving 'C' after psuh button) ---
  if (systemActive && !waitingForPrediction && MySerial.available()) {
    char c = MySerial.read();
    if (c == 'C') {
      Serial.println("FRDM requested image capture");
      captureAndUploadImage(); // Capture and upload the image to Firebase
    }
  }

  // --- Check for prediction result from Firebase if flag is set ---
  if (waitingForPrediction) {
    checkPrediction();
  }
}
