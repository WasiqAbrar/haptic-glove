#include <Wire.h>

#define LSM303_ACCEL  0x19
#define HMC5883L_MAG  0x1E
#define ITG3200_GYRO  0x69

const int motorPins[5] = {3, 5, 6, 9, 10};

float gyroOffsetX = 0;
float gyroOffsetY = 0;
float gyroOffsetZ = 0;

float filteredRoll  = 0;
float filteredPitch = 0;
unsigned long lastTime = 0;
String cmdBuffer = "";

void initSensors() {
  Wire.beginTransmission(LSM303_ACCEL);
  Wire.write(0x20); Wire.write(0x57);
  Wire.endTransmission();
  Wire.beginTransmission(ITG3200_GYRO);
  Wire.write(0x3E); Wire.write(0x00);
  Wire.endTransmission();
  Wire.beginTransmission(ITG3200_GYRO);
  Wire.write(0x16); Wire.write(0x18);
  Wire.endTransmission();
  Wire.beginTransmission(HMC5883L_MAG);
  Wire.write(0x00); Wire.write(0x70);
  Wire.endTransmission();
  Wire.beginTransmission(HMC5883L_MAG);
  Wire.write(0x01); Wire.write(0xA0);
  Wire.endTransmission();
  Wire.beginTransmission(HMC5883L_MAG);
  Wire.write(0x02); Wire.write(0x00);
  Wire.endTransmission();
}

bool readAccel(float &ax, float &ay, float &az) {
  Wire.beginTransmission(LSM303_ACCEL);
  Wire.write(0x28 | 0x80);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(LSM303_ACCEL, 6) != 6) return false;
  int16_t x = (Wire.read() | (Wire.read() << 8)) >> 4;
  int16_t y = (Wire.read() | (Wire.read() << 8)) >> 4;
  int16_t z = (Wire.read() | (Wire.read() << 8)) >> 4;
  // invert all axes for upside-down mounting
  ax = -(x * 0.001 * 9.81);
  ay = -(y * 0.001 * 9.81);
  az = -(z * 0.001 * 9.81);
  return true;
}

bool readGyroRaw(float &gx, float &gy, float &gz) {
  Wire.beginTransmission(ITG3200_GYRO);
  Wire.write(0x1D);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(ITG3200_GYRO, 6) != 6) return false;
  int16_t x = (Wire.read() << 8) | Wire.read();
  int16_t y = (Wire.read() << 8) | Wire.read();
  int16_t z = (Wire.read() << 8) | Wire.read();
  gx = x / 14.375;
  gy = y / 14.375;
  gz = z / 14.375;
  return true;
}

bool readGyro(float &gx, float &gy, float &gz) {
  if (!readGyroRaw(gx, gy, gz)) return false;
  gx -= gyroOffsetX;
  gy -= gyroOffsetY;
  gz -= gyroOffsetZ;
  // invert gyro axes for upside-down mounting
  gx = -gx;
  gy = -gy;
  return true;
}

void calibrateGyro() {
  Serial.println("Calibrating...");
  float sumX = 0, sumY = 0, sumZ = 0;
  int valid = 0;
  for (int i = 0; i < 100; i++) {
    float gx, gy, gz;
    if (readGyroRaw(gx, gy, gz)) {
      sumX += gx; sumY += gy; sumZ += gz;
      valid++;
    }
    delay(10);
  }
  if (valid > 0) {
    gyroOffsetX = sumX / valid;
    gyroOffsetY = sumY / valid;
    gyroOffsetZ = sumZ / valid;
  }
  Serial.println("Done!");
}

void setMotors(bool on) {
  for (int i = 0; i < 5; i++)
    digitalWrite(motorPins[i], on ? HIGH : LOW);
}

void handleSerialCommands() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n') {
      cmdBuffer.trim();
      if      (cmdBuffer == "ON")    setMotors(true);
      else if (cmdBuffer == "OFF")   setMotors(false);
      else if (cmdBuffer == "PULSE") {
        setMotors(true); delay(150); setMotors(false);
      }
      cmdBuffer = "";
    } else {
      cmdBuffer += c;
    }
  }
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setWireTimeout(25000, true);
  Wire.setClock(100000);
  initSensors();
  calibrateGyro();
  lastTime = millis();
  for (int i = 0; i < 5; i++)
    pinMode(motorPins[i], OUTPUT);
  Serial.println("READY");
}

void loop() {
  handleSerialCommands();

  if (Wire.getWireTimeoutFlag()) {
    Wire.clearWireTimeoutFlag();
    Wire.begin();
    Wire.setWireTimeout(25000, true);
    Wire.setClock(100000);
    initSensors();
    Serial.print("R:"); Serial.print(filteredRoll);
    Serial.print(",P:"); Serial.print(filteredPitch);
    Serial.println();
    return;
  }

  float ax, ay, az, gx, gy, gz;
  bool accelOk = readAccel(ax, ay, az);
  bool gyroOk  = readGyro(gx, gy, gz);

  unsigned long now = millis();
  float dt = (now - lastTime) / 1000.0;
  if (dt > 0.1) dt = 0.1;
  lastTime = now;

  if (accelOk && gyroOk) {
    // correct atan2 formula for upside-down IMU
    // with all axes inverted, these formulas now produce
    // correct roll and pitch when flat = 0,0
    float accelRoll  = atan2(ay, az) * 180.0 / PI;
    float accelPitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;

    filteredRoll  = 0.95 * (filteredRoll  + gy * dt) + 0.05 * accelRoll;
    filteredPitch = 0.95 * (filteredPitch + gx * dt) + 0.05 * accelPitch;
  }

  Serial.print("R:"); Serial.print(filteredRoll);
  Serial.print(",P:"); Serial.print(filteredPitch);
  Serial.println();

  delay(20);
}