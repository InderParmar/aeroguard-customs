# AeroGuard Customs: Functional and Non-Functional Requirements

## Business Case 

### Executive Summary

AeroGuard Customs addresses a growing demand for enhanced efficiency, security, and accuracy in customs inspections and duty estimations at international airports and border crossings. Leveraging embedded AI technology integrated into a portable hardware system, AeroGuard Customs identifies and categorizes hazardous items (guns and knives) in passenger luggage, significantly streamlining inspection processes, reducing operational costs, and mitigating risks associated with human error and revenue loss.

### Problem Statement

Traditional customs inspection processes rely heavily on manual checking and passenger declarations, which are prone to errors, underreporting, and inconsistent enforcement. This leads to lost revenue, security risks, long passenger wait times, and inefficient utilization of customs resources.

### Project Scope Evolution

The project originally began as Tap Cart, designed as a smart shopping cart leveraging embedded AI and cloud services to automate the retail checkout process. It utilized an ESP32 camera module, Firebase cloud integration, and a YoloV8 object detection model to recognize items in a shopping cart and calculate their total price, providing a frictionless shopping experience.

However, after guidance from the program chair and professors emphasizing alignment with the theme of assisting immigrants or vulnerable travelers, the project evolved into AeroGuard Customs. This pivot retains the technical architecture and engineering foundation of the original Tap Cart system, significantly shifting its functional purpose from retail checkout to airport security and customs inspection.

### Proposed Solution

AeroGuard Customs introduces an AI-driven embedded system that automatically scans luggage contents using high-resolution imaging and advanced object-detection models. Key features include:

  - Real-time luggage scanning with an ESP32-based embedded system and OV2640/OV5460 camera.

  - Cloud-based AI detection (TensorFlow/YoloV8) to categorize items as taxable, restricted, or hazardous.

  - Immediate computation of estimated customs duties and automated logging.

### Strategic Alignment

The project aligns with airport authorities' strategic goals by:

 - Enhancing operational efficiency through automation.

- Improving accuracy and compliance in duty assessments.

- Strengthening security by accurately identifying hazardous or prohibited items.
 
- Improving traveler experience by reducing wait times and inspection intrusiveness.

### Benefits and Opportunities

- Operational Efficiency: Reduction in manual labor, inspection time, and queue lengths.

- Revenue Protection: Enhanced duty collection accuracy and reduced evasion.

- Security Enhancement: Real-time identification of security threats.

- Scalability and Adaptability: Modular design adaptable to various international compliance standards and airport sizes.

### Financial Analysis

- Initial Investment: Relatively low-cost setup, leveraging existing embedded hardware (ESP32) and cloud infrastructure (Firebase).

- Cost Savings: Significant reduction in labor costs and decreased losses due to duty evasion.

- Revenue Increase: Improved accuracy in duty collection potentially increases revenue significantly.


---

## Functional Requirements:


1. **Image Capture and Processing:**

   * Capture high-resolution images of open or partially open luggage using ESP32 with OV2640/OV5460 camera.
   * Immediate storage and uploading of captured images to Firebase Cloud Storage.

2. **Embedded AI Object Detection:**

   * Detect and classify items such as electronics, luxury goods, liquids, sharp objects, hazardous items.
   * Real-time image analysis using YoloV8 AI model.

3. **Customs Metadata Integration:**

   * Identification of items with automated classification into taxable or hazardous categories.
   * Cross-reference with customs database for duty estimation and risk assessment.

4. **Real-time Feedback and UI:**

   * On screen Smart display to show identified items, threat levels, and duty estimation.
   * Auditory and visual feedback (LED, speaker alerts).

5. **Logging and Compliance:**

   * Secure logging of each inspection with timestamps, item details, and duty assessments in Firebase Realtime Database.

---

## Non-Functional Requirements:

1. **Performance:**

   * Real-time detection and UI response (target <3 seconds per luggage).
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

  * Button.
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
   ├─ Authenticate via RFID
   ├─ Capture Luggage Image
   ├─ Upload Image to Cloud
   ├─ Receive AI Classification Results
   ├─ Review Duty/Risk Assessment on LCD
   ├─ Log Results Securely
```

### Sequence Diagram:

```
Officer → Button 
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


