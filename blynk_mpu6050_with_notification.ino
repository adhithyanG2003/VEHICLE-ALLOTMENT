// pin d1 d2 
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define BLYNK_PRINT Serial

// Blynk Auth Token and WiFi credentials
char auth[] = "_GN0NcU7VlrAIDTY7UE7kgSC_TJcsZrE"; 
char ssid[] = "123456789"; 
char pass[] = "123456789"; 

// MPU6050 address
const int MPU_addr = 0x68;

// Variables for MPU6050 data
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
int minVal = 265;
int maxVal = 402;
double x, y, z;

// Buzzer pin
const int buzzerPin = D4; // NodeMCU D4

// Flag to control notification
bool tiltDetected = false;

void setup() {
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B); // Power management register
  Wire.write(0);    // Wake up MPU6050
  Wire.endTransmission(true);

  Serial.begin(9600);

  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
}

void loop() {
  readMPU6050();
  calculateAngles();
  sendToBlynk();
  checkTiltAndNotify();

  Blynk.run();
  delay(1000); // 1 second delay
}

// 📦 Read raw data from MPU6050
void readMPU6050() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B); // Start at ACCEL_XOUT_H
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true);

  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();
  Tmp = Wire.read() << 8 | Wire.read();
  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read();
  GyZ = Wire.read() << 8 | Wire.read();
}

// 📐 Calculate angles from accelerometer data
void calculateAngles() {
  int xAng = map(AcX, minVal, maxVal, -90, 90);
  int yAng = map(AcY, minVal, maxVal, -90, 90);
  int zAng = map(AcZ, minVal, maxVal, -90, 90);

  x = RAD_TO_DEG * (atan2(-yAng, -zAng) + PI);
  y = RAD_TO_DEG * (atan2(-xAng, -zAng) + PI);
  z = RAD_TO_DEG * (atan2(-yAng, -xAng) + PI);

  Serial.print("Angle X = "); Serial.println(x);
  Serial.print("Angle Y = "); Serial.println(y);
  Serial.print("Angle Z = "); Serial.println(z);
  Serial.println("-----------------------------------------");
}

// 📡 Send angles to Blynk app
void sendToBlynk() {
  Blynk.virtualWrite(V0, x);
  Blynk.virtualWrite(V1, y);
  Blynk.virtualWrite(V2, z);
}

// 🚨 Check if tilt happened and send notification
void checkTiltAndNotify() {
  if ((x > 260 && z <300 || z > 40 && z < 90 || y >135 && y < 160 ||y < 60 ) && !tiltDetected) {
    digitalWrite(buzzerPin, HIGH); // Turn ON buzzer
    Serial.println("BUZZER ON - Sending Notification...");
    Blynk.logEvent("allotment", "alltoment vehicle."); // Correct Blynk notification
    tiltDetected = true; // Set flag
  } 
  else  {
    digitalWrite(buzzerPin, LOW); // Turn OFF buzzer
    Serial.println("BUZZER OFF");
    tiltDetected = false; // Reset flag
  }
}
