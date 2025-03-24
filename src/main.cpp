#include <Arduino.h>
#include <Wire.h>
#include <LCD-I2C.h>
#include <Adafruit_VL53L0X.h>

#define LOX1_ADDRESS 0x30
#define DISTANCE_THRESHOLD 150
#define SENSOR_READ_INTERVAL 5

LCD_I2C lcd(0x27, 16, 2);
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

unsigned long startMillis = 0;
unsigned long stopMillis = 0;
unsigned long stopDisplayMillis = 0;
unsigned long elapsedMillis = 0;
unsigned long lastSensorReadMillis = 0;
unsigned long lastTimerUpdateMillis = 0;

bool isRunning = false;
bool isStopped = false;
bool showStopped = false;

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    delay(1);

  Wire.begin();
  lcd.begin(&Wire);
  lcd.display();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Start");

  if (!lox.begin(LOX1_ADDRESS))
  {
    Serial.println(F("Failed to boot VL53L0X"));
    while (1)
      ;
  }
}

void loop()
{
  if (millis() - lastSensorReadMillis >= SENSOR_READ_INTERVAL)
  {
    lastSensorReadMillis = millis();
    int distance = lox.readRange();

    if (distance < DISTANCE_THRESHOLD)
    {
      if (!isRunning && !isStopped)
      {
        isRunning = true;
        startMillis = millis();
        lcd.clear();
      }
      else if (isRunning)
      {
        isRunning = false;
        isStopped = true;
        showStopped = true;
        stopMillis = millis();
        elapsedMillis = stopMillis - startMillis;
        stopDisplayMillis = millis();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Stopped");
      }

      delay(250);
    }
  }

  if (showStopped && millis() - stopDisplayMillis >= 1000)
  {
    showStopped = false;
    lcd.clear();
  }

  if (isRunning && millis() - lastTimerUpdateMillis >= 100)
  {
    lastTimerUpdateMillis = millis();
    elapsedMillis = millis() - startMillis;

    int minutes = (elapsedMillis / 60000) % 60;
    int seconds = (elapsedMillis / 1000) % 60;
    int milliseconds = (elapsedMillis % 1000) / 10;

    lcd.setCursor(0, 0);
    lcd.print("Running...");
    lcd.setCursor(0, 1);

    // Format MM:SS:mm
    if (minutes < 10)
      lcd.print("0");
    lcd.print(minutes);
    lcd.print(":");

    if (seconds < 10)
      lcd.print("0");
    lcd.print(seconds);
    lcd.print(":");

    if (milliseconds < 10)
      lcd.print("0");
    lcd.print(milliseconds);
  }

  if (!showStopped && isStopped)
  {
    int minutes = (elapsedMillis / 60000) % 60;
    int seconds = (elapsedMillis / 1000) % 60;
    int milliseconds = (elapsedMillis % 1000) / 10;

    lcd.setCursor(0, 0);
    lcd.print("Final Time:");
    lcd.setCursor(0, 1);

    // Format MM:SS:mm
    if (minutes < 10)
      lcd.print("0");
    lcd.print(minutes);
    lcd.print(":");

    if (seconds < 10)
      lcd.print("0");
    lcd.print(seconds);
    lcd.print(":");

    if (milliseconds < 10)
      lcd.print("0");
    lcd.print(milliseconds);
  }

  // Send time to Node.js server via Serial
  if (isRunning)
  {
    elapsedMillis = millis() - startMillis;
    int minutes = (elapsedMillis / 60000) % 60;
    int seconds = (elapsedMillis / 1000) % 60;
    int milliseconds = (elapsedMillis % 1000) / 10;

    Serial.print(minutes);
    Serial.print(":");
    Serial.print(seconds);
    Serial.print(":");
    Serial.println(milliseconds);
  }
}
