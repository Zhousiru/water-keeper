#include <Arduino.h>

// configs
const int PUMPING_DETECT_DELAY = 100;
const int BACKUP_DETECT_DELAY = 100;
const int TANK_DETECT_DELAY = 5 * 1000;
const int TARGET_BLEED = 20 * 1000;

// pins
const int PUMP_PIN = 8;
const int PUMP_LED_PIN = 9;
const int LOW_WATER_LEVEL_LED_PIN = 10;

const int SENSOR_BACKUP_PIN = 11;
const int SENSOR_TANK_PIN = 12;

// global status
bool IS_PUMPING = false;
bool IS_BLEEDING = false;
int REMAIN_BLEED = 0;
int REMAIN_TANK_DETECT_DELAY = 0;

void setup()
{
  // VCC pins(debug)
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  digitalWrite(2, HIGH);
  digitalWrite(3, HIGH);

  pinMode(PUMP_PIN, OUTPUT);
  pinMode(PUMP_LED_PIN, OUTPUT);
  pinMode(LOW_WATER_LEVEL_LED_PIN, OUTPUT);
  pinMode(SENSOR_TANK_PIN, INPUT);
  pinMode(SENSOR_BACKUP_PIN, INPUT);

  Serial.begin(9600); // debug
}

void loop()
{
  bool tank_water = digitalRead(SENSOR_TANK_PIN);
  bool backup_water = digitalRead(SENSOR_BACKUP_PIN);

  if (IS_PUMPING)
  {
    delay(PUMPING_DETECT_DELAY);
    if (IS_BLEEDING)
    {
      REMAIN_BLEED -= PUMPING_DETECT_DELAY;

      Serial.print("REMAIN_BLEED: ");
      Serial.println(REMAIN_BLEED);
    }

    if (!backup_water)
    {
      // backup water is not enough

      Serial.println("stop pumping: backup water is not enough");
      IS_BLEEDING = false;
      IS_PUMPING = false;
      analogWrite(PUMP_LED_PIN, 0);
      digitalWrite(PUMP_PIN, LOW);

      return;
    }

    if (tank_water && !IS_BLEEDING)
    {
      Serial.println("start bleeding: reached the target water level");
      IS_BLEEDING = true;
      REMAIN_BLEED = TARGET_BLEED;
      analogWrite(PUMP_LED_PIN, 32);

      return;
    }

    if (IS_BLEEDING && REMAIN_BLEED <= 0)
    {
      // reached the target bleed

      Serial.println("stop pumping: reached the target bleed");
      IS_BLEEDING = false;
      IS_PUMPING = false;
      analogWrite(PUMP_LED_PIN, 0);
      digitalWrite(PUMP_PIN, LOW);

      return;
    }

    Serial.println("pumping...");
    return; // continue pumping
  }

  // Serial.println("-----------------------------");

  // Serial.print("tank_water: ");
  // Serial.println(tank_water);
  // Serial.print("backup_water: ");
  // Serial.println(backup_water);

  delay(BACKUP_DETECT_DELAY);

  // pause tank detect, until backup_water is true
  if (!backup_water)
  {
    digitalWrite(LOW_WATER_LEVEL_LED_PIN, HIGH);
    return;
  } else {
    digitalWrite(LOW_WATER_LEVEL_LED_PIN, LOW);
    REMAIN_TANK_DETECT_DELAY -= BACKUP_DETECT_DELAY;
  }

  if (REMAIN_TANK_DETECT_DELAY <= 0)
  {
    REMAIN_TANK_DETECT_DELAY = TANK_DETECT_DELAY; // reset
    if (!tank_water)
    {
      Serial.println("start pumping");
      analogWrite(PUMP_LED_PIN, 255);
      digitalWrite(PUMP_PIN, HIGH);
      IS_PUMPING = true;
    }
  }
}
