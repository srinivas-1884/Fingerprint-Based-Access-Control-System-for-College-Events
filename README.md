# Fingerprint-Based Access Control System for College Events

*A Biometric Authentication System Using Arduino, Python & Web Interface*

## Overview

This project implements a complete **Fingerprint-Based Access Control System** designed for secure and automated college event management.
It integrates:

* Arduino + Fingerprint Sensor
* Python Bridge Script for Serial Communication
* Web-Based Dashboard
* JSON Database for User Storage

The system supports **enrollment**, **authentication**, **real-time monitoring**, and **user management**.

---

## Project Structure

```
├── Bridge.py
├── index.html
├── users.json
├── Enrolling.ino
├── verification_code.ino
├── fingerprints_deleting_in_sensors.ino
├── Fingerprint-Based Access Control System for College Events.pdf
├── Fingerprint-Based Access Control System for College Event.pptx
```

### Description of Each File

* **Bridge.py** – Python WebSocket & Serial communication bridge. Handles user enrollment, deletion, live updates, and JSON database sync.


* **index.html** – Web dashboard for enrollment, user list management, device status & logs.


* **users.json** – Lightweight JSON-based database storing enrolled user ID, roll number, and date.


* **Enrolling.ino / verification_code.ino / fingerprints_deleting_in_sensors.ino** – Arduino fingerprint sensor modules for enrollment, verification, and deletion.

* **PDF & PPTX** – Complete project documentation with system design, flowcharts, module descriptions, testing, screenshots, and references.



---

## Features

### 1. Enrollment Module

* Registers new users by capturing fingerprint templates and roll numbers.
* Checks for duplicate roll numbers and duplicate fingerprint templates.
* Updates the JSON database automatically.
  (Explained in PDF Section — Enrollment Module; includes flow diagram and screenshots.)


---

### 2. Authentication Module

* Matches scanned fingerprint with stored templates.
* Displays **Access Granted** or **Denied** on OLED and Web UI.
* Logs events automatically.
  (Images shown in PDF page 5: Access Granted/Denied.)


---

### 3. Real-Time Web Dashboard

Built using HTML, CSS, and JavaScript.

Provides:

* Device connection status
* WebSocket communication
* Enrollment panel
* Live LCD-like log display
* User list table with delete option
* Export CSV button
  (Code from index.html)


---

### 4. Python Communication Bridge

Handles:

* Serial reading/writing
* WebSocket server
* Broadcasting updates
* Managing JSON database
* Duplicate roll number detection
  (Logic visible in `load_users()`, `get_next_id()`, serial event handler)


---

### 5. JSON-Based Database

Stores:

```
[
  {
    "ID_No": 1,
    "roll": "22R11A6739",
    "date": "2025-11-27 19:55:20"
  }
]
```

Lightweight, easy to parse, auto-updated by Bridge.py.


---

## System Architecture

### Hardware

* Arduino Uno / compatible board
* R307/R305 fingerprint sensor
* OLED Display
* LEDs + Buzzer
* Optional servo or relay for gate control

The PDF file (pages 1–3) includes diagrams of:

* System Design
* Fingerprint Analysis
* Proposed System Block Diagram


---

### Software

* Arduino IDE (.ino sketches)
* Python 3 (asyncio, websockets, pySerial)
* HTML/JS frontend

---

## How the System Works

### Step 1 — Enrollment

1. Enter roll number in web UI.
2. Arduino captures fingerprint (two scans).
3. Python bridge receives `ENROLL_SUCCESS:<roll>` message.
4. System assigns the smallest available **ID_No** using `get_next_id()` logic.
5. Saves record to `users.json`.


---

### Step 2 — Authentication

1. Finger is placed on sensor.
2. Sensor matches against stored templates.
3. Display shows success/failure.
4. Web UI logs the event in real-time.
   (Images in PDF Section: Experimental Results — Access Granted/Denied)


---

### Step 3 — User Management

* View all users in a sortable table.
* Delete user → removes JSON entry + sensor template.
* Export CSV list.
  (Buttons shown in index.html UI)


---

## Setup Instructions

### 1. Hardware Setup

* Connect fingerprint sensor to Arduino
* Connect OLED & LEDs
* Upload `.ino` files to Arduino

### 2. Running the Python Bridge

```bash
pip install pyserial websockets
python Bridge.py
```

### 3. Running the Web Dashboard

Open `index.html` in Chrome/Firefox.

---

## Screenshots

Screenshots of UI and hardware are provided in the PDF under *Experimental Results*:

* Web dashboard Home Page
* Enrollment success
* Access granted
* Access denied


---

## References

All references used in the project are provided in the PDF's **References** section (IEEE papers, Arduino docs, biometric research).

