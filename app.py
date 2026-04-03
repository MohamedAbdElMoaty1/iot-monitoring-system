from flask import Flask, request, jsonify, render_template
from flask_socketio import SocketIO
import sqlite3

app = Flask(__name__) 

# SocketIO 
socketio = SocketIO(app, cors_allowed_origins="*")

DB_NAME = "database.db"  

# SECURITY 
 
API_KEY = "12345" 


# DATABASE  

def init_db():   

    conn = sqlite3.connect(DB_NAME)
    c = conn.cursor()

    # Sensor data table
    c.execute("""
    CREATE TABLE IF NOT EXISTS sensor_data (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        temperature REAL,
        humidity REAL,
        gas REAL,
        flame INTEGER,
        alarm INTEGER,
        timestamp TEXT
    )
    """)

    # Event history table
    c.execute("""
    CREATE TABLE IF NOT EXISTS events (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        event TEXT,
        timestamp TEXT
    )
    """)

    conn.commit()
    conn.close()


init_db()


# HOME

@app.route('/')
def home():
    return "IoT Server Running"


# RECEIVE SENSOR DATA

@app.route('/sensor-data', methods=['POST'])
def receive_data():

    # Security check
    api_key = request.headers.get("X-API-KEY")

    if api_key != API_KEY:
        return jsonify({"error": "Unauthorized"}), 403

    data = request.get_json()

    temperature = data.get("temperature")
    humidity = data.get("humidity")
    gas = data.get("gas")
    flame = data.get("flame")
    alarm = data.get("alarm")
    timestamp = data.get("timestamp")

    conn = sqlite3.connect(DB_NAME)
    c = conn.cursor()

    c.execute("""
    INSERT INTO sensor_data
    (temperature, humidity, gas, flame, alarm, timestamp)
    VALUES (?,?,?,?,?,?)
    """, (temperature, humidity, gas, flame, alarm, timestamp))

    conn.commit()
    conn.close()

    # Broadcast real-time data
    socketio.emit("new_data", {
        "temperature": temperature,
        "humidity": humidity,
        "gas": gas,
        "flame": flame,
        "alarm": alarm,
        "timestamp": timestamp
    })

    return jsonify({"status": "saved"})


#GET SENSOR DATA

@app.route('/api/data')  
def get_data():

    conn = sqlite3.connect(DB_NAME)
    c = conn.cursor()

    c.execute("""
    SELECT temperature, humidity, gas, flame, alarm, timestamp
    FROM sensor_data
    ORDER BY id DESC
    LIMIT 50
    """)

    rows = c.fetchall()
    conn.close()

    data = [
        {
            "temperature": r[0],
            "humidity": r[1],
            "gas": r[2],
            "flame": r[3],
            "alarm": r[4],
            "timestamp": r[5]
        }
        for r in rows
    ]

    return jsonify(data[::-1])


# ADD EVENT

@app.route('/api/event', methods=['POST']) 
def add_event():

    data = request.get_json()

    event = data.get("event")
    timestamp = data.get("timestamp")

    conn = sqlite3.connect(DB_NAME)
    c = conn.cursor()

    c.execute(
        "INSERT INTO events (event, timestamp) VALUES (?,?)",
        (event, timestamp)
    )

    conn.commit()
    conn.close()   

    return jsonify({"status": "event saved"})


# GET EVENTS

@app.route('/api/events')
def get_events():

    conn = sqlite3.connect(DB_NAME)
    c = conn.cursor()

    c.execute("""
    SELECT event, timestamp
    FROM events
    ORDER BY id DESC
    LIMIT 20
    """)

    rows = c.fetchall()
    conn.close()

    events = [
        {
            "event": r[0],
            "timestamp": r[1]
        }
        for r in rows
    ]

    return jsonify(events)


# DASHBOARD

@app.route('/dashboard')
def dashboard():
    return render_template("dashboard.html")


# RUN SERVER

if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5000)
