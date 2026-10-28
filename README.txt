PRESSURE MONITOR FOR A PELLET BURNER
====================================

A custom monitoring system for a pellet burner. An ESP32 reads differential
pressure from the flue draft with a Sensirion SDP610 sensor and sends each
reading over Wi-Fi to a Flask server. The data is stored in SQLite with
SQLAlchemy and displayed as a live Streamlit chart that updates every few
seconds. The goal is to tune the chimney draft damper for stable combustion
and to see how draft, wind and burn cycles interact.


PROJECT STRUCTURE
-----------------
main.py            Flask server. Receives POST requests at /data and writes
                   readings to the database.
models.py          SQLAlchemy database models and engine setup.
data_viz.py        Streamlit dashboard. Reads the database and draws the chart.
heatsystem.db      SQLite database with the recorded readings.
requirements.txt   Python packages used by the project.
arduino/           ESP32 firmware (Arduino sketches).


HARDWARE
--------
- ESP32 development board
- Sensirion SDP610-25Pa differential pressure sensor (I2C)
- 2 x 4.7 kOhm pull-up resistors (SDA and SCL to 3.3 V)
- Flue tap and tubing (High port to flue, Low port open to room air)

Wiring:
  Sensor VDD    -> 3.3 V   (not 5 V)
  Sensor GND    -> GND
  Sensor Data   -> GPIO 21 (SDA)
  Sensor Clock  -> GPIO 22 (SCL)


SETUP
-----
1. Create and activate a virtual environment:
     python3 -m venv .venv
     source .venv/bin/activate

2. Install the requirements:
     pip install -r requirements.txt

3. Set the Wi-Fi and server values in the ESP32 sketch:
     arduino/Flask_ESP32_Pressure_Sensor/Flask_ESP32_Pressure_Sensor.ino

     const char* WIFI_SSID     = "YOUR_SSID";
     const char* WIFI_PASSWORD = "YOUR_PASSWORD";
     const char* SERVER_URL    = "http://YOUR_SERVER_IP:5003/data";

   Keep real values out of version control.

4. Flash the sketch to the ESP32 with Arduino IDE.


RUNNING
-------
Start the Flask receiver (in one terminal):
     python main.py

Start the Streamlit dashboard (in a second terminal):
     streamlit run data_viz.py --server.address 0.0.0.0

Open the dashboard from any device on the same network at:
     http://<computer-ip>:8501


NOTES
-----
- The Streamlit and Flask servers listen on 0.0.0.0 so other devices on the
  LAN can reach them. Use this only on a trusted network.
- Flask debug mode is off by default. Do not enable it on a network-facing
  server.
- The timestamps are stored in UTC. The dashboard shows them as recorded.


SAFETY
------
This project reads a sensor on a live solid-fuel heating system. It does not
control the burner and must not be used to replace the burner's safety
devices. The flashback protection on the feed chute must stay in place at
all times. Do not rely on this project for carbon monoxide detection; use a
certified CO alarm.


