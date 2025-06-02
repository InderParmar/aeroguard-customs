# AeroGuard Customs: Functional and Non-Functional Requirements

## Functional Requirements:

1. **User Authentication:**
   * Secure access limited to authorized customs officers.

2. **Image Capture and Processing:**

   * Capture high-resolution images of open or partially open luggage using ESP32 with OV2640/OV5460 camera.
   * Immediate storage and uploading of captured images to Firebase Cloud Storage.

3. **Embedded AI Object Detection:**

   * Detect and classify items such as electronics, luxury goods, liquids, sharp objects, hazardous items.
   * Real-time image analysis using YoloV8 AI model.

4. **Customs Metadata Integration:**

   * Identification of items with automated classification into taxable or hazardous categories.
   * Cross-reference with customs database for duty estimation and risk assessment.

5. **Real-time Feedback and UI:**

   * On screen Smart display to show identified items, threat levels, and duty estimation.
   * Auditory and visual feedback (LED, speaker alerts).

6. **Logging and Compliance:**

   * Secure logging of each inspection with timestamps, item details, and duty assessments in Firebase Realtime Database.

---

## Non-Functional Requirements:

1. **Performance:**

   * Real-time detection and UI response (target <5 seconds per luggage).
   * High accuracy (>90%) in detection to avoid false positives or negatives.

2. **Security:**

   * Strict access controls (Firebase security rules).
   * Encryption of image uploads and communications via HTTPS/TLS.
   * Secure storage with restricted access.

3. **Usability:**

   * Simple, intuitive UI suitable for customs officers.
   * Minimal training required for operators.

4. **Scalability:**

   * Modular software/hardware architecture enabling easy updates and scalability for larger deployments.

5. **Reliability and Robustness:**

   * Low error rate in activation, scanning, uploading, and detection.
   * Reliable operation in variable lighting conditions.

---

# High-Level System Design:

The system is divided into clearly defined modules for simplicity and scalability:

* **Embedded System Module (ESP32 with OV2640/OV5460)**
  * Camera Capture
  * Image Upload (Firebase via HTTPS)

* **Cloud Module (Firebase Cloud + Cloud Run)**

  * Image Storage
  * AI Inference and Classification (YoloV8)
  * Duty and risk calculation based on customs database

* **UI and Feedback Module**

  * LCD Display (item details, duty estimates, threats)
  * Speaker/LED (alerts)

---

# Low-Level Design (Module Breakdown):

### 1. **Embedded Module (ESP32)**

* **Input:**

  * RFID/NFC reader via SPI.
* **Output:**

  * UART communication with display unit.
  * HTTPS upload to Firebase Cloud Storage.
* **Process:**

  * Image buffer handling, camera resolution management (QQVGA to VGA), and memory optimization.

### 2. **Camera Module (OV2640/OV5460)**

* **Input:**

  * Camera initialization commands.
* **Output:**

  * JPEG images stored in ESP32 buffer.
* **Process:**

  * Adjustable resolution, brightness, exposure for consistent image quality.

### 3. **AI & Cloud Module (Firebase + Cloud Run)**

* **Input:**

  * JPEG images from ESP32.
* **Output:**

  * JSON response containing detected items, categories, threat levels, duty estimations.
* **Process:**

  * Model inference using YoloV8 on Flask API.
  * Database queries for customs tariff information.

### 4. **Feedback and UI Module (LCD/Speaker/LED)**

* **Input:**

  * UART commands from ESP32.
* **Output:**

  * Visual display on LCD.
  * Auditory alerts (speaker), status LEDs.
* **Process:**

  * Parse UART messages and provide intuitive feedback.

---

# UML Diagrams:

### Use Case Diagram:

```
Customs Officer
   ├─ Capture Luggage Image
   ├─ Upload Image to Cloud
   ├─ Receive AI Classification Results
   ├─ Review Duty/Risk Assessment on LCD
   ├─ Log Results Securely
```

### Sequence Diagram:

```
ESP32 → Camera (OV2640/OV5460): Capture Image
Camera → ESP32: Image Data (JPEG)
ESP32 → Firebase: Upload Image via HTTPS
Firebase → Cloud Run (AI Model): Trigger Prediction
Cloud Run → Firebase: Prediction Result (JSON)
Firebase → ESP32: Send Result
ESP32 → LCD/Speaker: Display Result, Alert Officer
ESP32 → Firebase: Log Event (DB)
```

### Component Diagram:

```
[Camera Module] ← SPI/I2C → [ESP32]
[ESP32] ← HTTPS → [Firebase Cloud Storage]
[Firebase Cloud Storage] → [Cloud Run (Flask API + AI Model)]
[ESP32] ← UART → [LCD, LEDs, Speaker]
```

---

# Workflow Overview:

1. **Image Capture**: ESP32 initiates camera capture of luggage.
2. **Cloud Upload**: Captured image uploaded securely to Firebase.
3. **AI Inference**: Cloud-based AI identifies items and computes duties/threat level.
4. **Feedback**: Results are displayed clearly on LCD and communicated via alerts.
5. **Logging**: Transaction details are securely logged for compliance/auditing purposes.

---

# Security Considerations:

* Encrypted HTTPS communication.
* Role-based access control for data viewing and interactions.
* Secure Firebase storage with controlled access.
* Audit logs for compliance and accountability.

---

### Testing Strategy:

* **Unit Tests**: Validate individual hardware components, model efficiency & results, Flask API and ESP32 software modules.
* **Integration Tests**: API + Esp32 integration, API + model capabilities and Verify full system pipeline (end-to-end testing from RFID tap to LCD output).
* **Regression Tests**: Continuous API checks after model updates.
* **Real-world Scenario Tests**: Physically test luggage scans in realistic environments.

---

### Project Tracking & Management:

* GitHub Repository: [AeroGuard Customs Repo](https://github.com/InderParmar/aeroguard-customs)
* Tools Used: Trello for task management; Arduino IDE, VS Code, Jupyter for software development.

