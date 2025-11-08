#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "MAX30105.h"
#include "heartRate.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Adafruit_MPU6050 mpu;
MAX30105 particleSensor;

#define PRESSURE_PIN1 34
#define PRESSURE_PIN2 35
#define BUZZER_PIN 25

float SpO2 = 0.0;
int pressure1 = 0;
int pressure2 = 0;
bool fallDetected = false;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // OLED setup
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // MPU6050 setup
  if(!mpu.begin()) {
    Serial.println("MPU6050 not found!");
    for(;;);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);

  // MAX30102 setup
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("MAX30102 not found!");
    for(;;);
  }
  particleSensor.setup(); // default config
}

void loop() {
  // Pressure sensors
  pressure1 = analogRead(PRESSURE_PIN1);
  pressure2 = analogRead(PRESSURE_PIN2);

  // SpO2 sensor
  long irValue = particleSensor.getIR();
  long redValue = particleSensor.getRed();
  SpO2 = (redValue > 0 && irValue > 0) ? (float)redValue / (float)irValue * 100 : 0;

  // Gyroscope for fall detection
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float accelTotal = sqrt(a.acceleration.x * a.acceleration.x +
                          a.acceleration.y * a.acceleration.y +
                          a.acceleration.z * a.acceleration.z);

  if (accelTotal > 18.0 || accelTotal < 2.0) { // threshold for fall
    fallDetected = true;
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);
  } else {
    fallDetected = false;
  }

  // Display data
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("P1:");
  display.print(pressure1);
  display.print("  P2:");
  display.print(pressure2);

  display.setCursor(0, 16);
  display.print("SpO2:");
  display.print(SpO2, 1);
  display.print("%");

  display.setCursor(0, 32);
  display.print("Accel:");
  display.print(accelTotal, 1);

  display.setCursor(0, 48);
  if (fallDetected) {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.print("FALL DETECTED!");
  } else {
    display.print("Status: Normal");
  }

  display.display();

  delay(500);
}
