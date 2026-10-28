// this script combines the ESP32 and Flask Webapp
// includes the Pressure Sensor Data as well.

// include libraries
#include <WiFi.h>
#include<HTTPClient.h>

// Define physical pins matching your layout: Data = 21, Clock = 22
#define PIN_SDA 21
#define PIN_SCL 22
#define SDP6X_I2C_ADDRESS 0x40


const float SCALE_FACTOR = 1200.0; // Scale factor for the 25 Pa variant
const char* ssid = "WIFISSID";
const char* password = "WIFIPASSWORD"; // your network password
const char* SERVER_URL = "http://YOUR_SERVER_IP:5003/data"; // Flask server endpoint



// --- Low Level Software I2C Helpers ---

void i2c_start() {
  pinMode(PIN_SDA, OUTPUT);
  digitalWrite(PIN_SDA, LOW);
  delayMicroseconds(5);
  pinMode(PIN_SCL, OUTPUT);
  digitalWrite(PIN_SCL, LOW);
  delayMicroseconds(5);
}

void i2c_stop() {
  pinMode(PIN_SDA, OUTPUT);
  digitalWrite(PIN_SDA, LOW);
  delayMicroseconds(5);
  pinMode(PIN_SCL, INPUT); // Release SCL to pull-up
  delayMicroseconds(5);
  pinMode(PIN_SDA, INPUT); // Release SDA to pull-up
  delayMicroseconds(5);
}

// Custom wait routine that forces the ESP32 to sit idle while the sensor stretches SCL
bool wait_for_clock_stretch() {
  pinMode(PIN_SCL, INPUT); // Let pull-up handle line state
  unsigned long timeout = millis();
  while (digitalRead(PIN_SCL) == LOW) {
    if ((millis() - timeout) > 150) { // 150ms structural timeout max
      return false; // Stretched too long / Line shorted
    }
    delayMicroseconds(10);
  }
  return true;
}

bool i2c_write_byte(uint8_t byteData) {
  for (int i = 0; i < 8; i++) {
    bool bitVal = (byteData & (0x80 >> i));
    if (bitVal) {
      pinMode(PIN_SDA, INPUT); // Logic HIGH via pull-up
    } else {
      pinMode(PIN_SDA, OUTPUT);
      digitalWrite(PIN_SDA, LOW); // Logic LOW
    }
    delayMicroseconds(5);

    if (!wait_for_clock_stretch()) return false;

    pinMode(PIN_SCL, OUTPUT);
    digitalWrite(PIN_SCL, LOW);
    delayMicroseconds(5);
  }

  // Read ACK bit from slave
  pinMode(PIN_SDA, INPUT);
  if (!wait_for_clock_stretch()) return false;
  bool ack = (digitalRead(PIN_SDA) == LOW);

  pinMode(PIN_SCL, OUTPUT);
  digitalWrite(PIN_SCL, LOW);
  delayMicroseconds(5);
  return ack;
}

uint8_t i2c_read_byte(bool sendAck) {
  uint8_t byteData = 0;
  pinMode(PIN_SDA, INPUT);

  for (int i = 0; i < 8; i++) {
    if (!wait_for_clock_stretch()) return 0;
    if (digitalRead(PIN_SDA) == HIGH) {
      byteData |= (0x80 >> i);
    }
    pinMode(PIN_SCL, OUTPUT);
    digitalWrite(PIN_SCL, LOW);
    delayMicroseconds(5);
  }

  // Send ACK or NACK
  if (sendAck) {
    pinMode(PIN_SDA, OUTPUT);
    digitalWrite(PIN_SDA, LOW); // ACK
  } else {
    pinMode(PIN_SDA, INPUT); // NACK
  }
  delayMicroseconds(5);

  if (!wait_for_clock_stretch()) return 0;

  pinMode(PIN_SCL, OUTPUT);
  digitalWrite(PIN_SCL, LOW);
  pinMode(PIN_SDA, INPUT);
  delayMicroseconds(5);

  return byteData;
}

// --- Main Program Block ---

void setup() {
  Serial.begin(115200);
  //WIFI
  // Connect to WiFi
  WiFi.mode(WIFI_STA); //resets old WIFI settings.
  WiFi.begin(ssid, password);

  // this block waits until WIFI connects
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  // block end

  // shows Wifi connection and IP address
  Serial.print("Connected to WiFi. IP address: ");
  Serial.println(WiFi.localIP());

  // Set default state as Floating inputs (handled by physical 4.7k pullups)
  pinMode(PIN_SDA, INPUT);
  pinMode(PIN_SCL, INPUT);

  delay(1000); // Allow sensor startup stability window
  Serial.println("Native Bit-Bang Driver Active. Polling SDP6x-025Pa...");

}

void loop() {
  // 1. Trigger Measurement Command (Write Mode)
  i2c_start();
  if (!i2c_write_byte((SDP6X_I2C_ADDRESS << 1) | 0)) { // Write Address (0x80)
    Serial.println("Error: Sensor did not ACK address frame.");
    i2c_stop();
    delay(2000);
    return;
  }

  if (!i2c_write_byte(0xF1)) { // Command byte
    Serial.println("Error: Sensor did not ACK command byte.");
    i2c_stop();
    delay(2000);
    return;
  }
  i2c_stop();

  delay(5); // Small buffer pause

  // 2. Read back Data Sequence
  i2c_start();
  if (!i2c_write_byte((SDP6X_I2C_ADDRESS << 1) | 1)) { // Read Address (0x81)
    Serial.println("Error: Read handshake failed.");
    i2c_stop();
    delay(2000);
    return;
  }

  // At this stage, the sensor forces Clock Stretching while processing.
  // The wait_for_clock_stretch function inside i2c_read_byte will hold the line.
  uint8_t msb = i2c_read_byte(true);  // Read MSB, send ACK
  uint8_t lsb = i2c_read_byte(true);  // Read LSB, send ACK
  uint8_t crc = i2c_read_byte(false); // Read CRC, send NACK to finish
  i2c_stop();

  // 3. Translate bit streams into physical pressure values
  int16_t raw_pressure = (int16_t)((msb << 8) | lsb);
  float pressure_pa = (float)raw_pressure / SCALE_FACTOR;

  Serial.print("Pressure: ");
  Serial.print(pressure_pa, 4);
  Serial.println(" Pa");


  // Set up the HTTP transfer of the data
  HTTPClient http; //creates the client object.
  http.begin(SERVER_URL); // sets the address from the constant you already wrote.
  http.addHeader("Content-Type", "application/json"); //tells Flask the body is JSON.
  String payload = "{\"pressure\": " + String(pressure_pa, 4) + "}";
  int httpCode = http.POST(payload);
  Serial.println(httpCode);
  Serial.println(http.getString());
  http.end();

  delay(5000); // Poll once every 5 seconds
}


