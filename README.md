# AeroGuard Customs – *AI-Powered Luggage Scanner for Airport Security*

## 🔍 Demo Video (Original Tap Cart Prototype)

[https://youtu.be/8iXMl7V7iCE](https://youtu.be/8iXMl7V7iCE)

> *(Note: This demo showcases the core pipeline that AeroGuard Customs builds upon—camera image capture, cloud AI model inference, and embedded output display. A luggage-specific version is currently in development.)*

---

## ✈️ Project Overview

**AeroGuard Customs** is a smart embedded system designed to assist customs and airport security personnel by **automatically scanning open or partially open luggage** and detecting **high-value or potentially dangerous items**. The system uses real-time object detection powered by an AI model hosted in the cloud, helping to speed up inspections and reduce revenue loss from undeclared goods.

Originally designed as a smart shopping cart (*Tap Cart*), this project has been **refined and repurposed** to suit the needs of international border control and customs enforcement.

---

## 🛠 Hardware Components

The following hardware components are reused from the original Tap Cart system with minor modifications:

* **ESP32 Wi-Fi Module** – Central controller handling system coordination and output display, also captures and uploads luggage images to cloud storage for AI analysis.
* **Arducam Mega Camera** – Mounted above luggage, captures high-resolution images for object detection.
* **4-Line I2C LCD Display** – Displays detected items, threat levels, and estimated customs duties.
* **RGB LED** – Indicates scan result status. \[Green -> Clear, Yellow -> Dutiable, Red -> Threat]
* **Speaker** – Alerts customs personnel for flagged scans or scan completion.
* **RFID/NFC Reader** – Authenticates authorized customs officers for initiating scans.

---

## 🤖 AI Integration

* Images are captured via the Arducam and sent to **Firebase Cloud Storage** by the ESP32.
* A **Firebase Cloud Function** is triggered on image upload and routes the image to a **Cloud Run API** hosting a **custom-trained TensorFlow object detection model**.
* The AI model identifies **luxury, commercial, or restricted items** in luggage.
* The detection results are returned to the ESP32 microcontroller and displayed with appropriate alerts and risk levels.

---

## 🎯 Project Goals

* Assist customs officers in identifying undeclared dutiable items and prohibited objects.
* Speed up the secondary inspection process through automated AI analysis.
* Provide an embedded and cost-effective alternative to high-end airport scanning systems.
* Ensure modularity and future extensibility for larger luggage or full-suitcase scanning.

---

## 📤 Output Delivery

* Results are shown **on-device** via LCD, LED, and speaker alerts.
* All scans and results are logged in **Firebase Realtime Database** with timestamps and officer IDs.
* Future work includes a mobile/web interface for real-time access and analytics.

---

**Developers**: Abhi Patel & Inderpreet Singh Parmar