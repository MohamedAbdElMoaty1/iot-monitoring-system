# 🚀 IoT Safety Monitoring System

Real-time IoT safety monitoring system built on Raspberry Pi using Flask and C++ for detecting gas leaks, flame, and high temperature conditions.

---

## 📌 Overview

This project is a complete IoT-based safety system designed to monitor environmental hazards in real-time.

It uses a Raspberry Pi to collect sensor data and send it to a Flask backend server, which processes, stores, and streams the data live.

---

## ⚙️ System Architecture

* **Raspberry Pi (Client)**

  * Reads sensors (Temperature, Humidity)
  * Detects gas & flame events
  * Controls LEDs and buzzer
  * Sends data to server (JSON over HTTP)

* **Flask Server (Backend)**

  * Receives sensor data
  * Stores it in SQLite database
  * Provides REST APIs
  * Streams real-time updates using Socket.IO

---

## 🧠 Features

* 🔥 Flame detection
* 🧪 Gas detection
* 🌡️ Temperature monitoring
* 🚨 Real-time alarm system
* 💾 Data storage (SQLite)
* 📡 Live updates (SocketIO)
* 🔘 Manual reset button
* 💡 LED & Buzzer control

---

## 🛠️ Technologies

* Python (Flask, SocketIO)
* C++ (Embedded system)
* Raspberry Pi
* SQLite
* Linux (sysfs, input events)

---

## 📂 Project Structure

iot-monitoring-system/
│
├── app.py
├── sensor_system.cpp
├── database.db
├── templates/
│   └── dashboard.html
└── README.md

---

## 🚀 How to Run

### 1️⃣ Run Server

pip install flask flask-socketio
python app.py

Server runs on: http://localhost:5000

---

### 2️⃣ Run on Raspberry Pi

Compile:

g++ sensor_system.cpp -o sensor

Run:

./sensor

---

## 🔐 API Key

X-API-KEY: 12345

⚠️ Use environment variables in production.

---

## 📡 API Endpoints

POST /sensor-data
GET /api/data
POST /api/event
GET /api/events

---

## 📊 Example JSON

{
"temperature": 30.5,
"humidity": 40,
"gas": 0,
"flame": 1,
"alarm": 1,
"timestamp": "2026-01-01 12:00:00"
}

---

## 🎯 Future Work

* Cloud deployment
* Mobile notifications
* Advanced dashboard
* Docker support

---

## 👨‍💻 Author

Mohamed Abd ElMoaty

---

## ⭐ Note

This project demonstrates real integration between embedded systems and backend development.
