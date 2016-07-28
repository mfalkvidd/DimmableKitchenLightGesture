#include <FastLED.h>
#include <SparkFun_APDS9960.h>

// Pins
#define GREEN_OUTPUT 10
#define BLUE_OUTPUT 11
#define RED_OUTPUT 5
#define WW_OUTPUT 6

// Constants
#define MAX_DISTANCE 20
#define MIN_LEVEL 35

#define GREEN_BASE_LEVEL 50
#define BLUE_BASE_LEVEL 6
#define RED_BASE_LEVEL 255
#define WW_BASE_LEVEL 255

#define LOCK_TIME 1000
#define LOCK_THRESHOLD 8

// Global Variables
SparkFun_APDS9960 apds = SparkFun_APDS9960();
unsigned long last_lock_time = 0;
byte last_lock_level = 0;

void setup() {

  // Initialize Serial port
  Serial.begin(9600);
  Serial.println();
  Serial.println(F("------------------------------------"));
  Serial.println(F("SparkFun APDS-9960 - ProximitySensor"));
  Serial.println(F("------------------------------------"));

  // Initialize APDS-9960 (configure I2C and initial values)
  if ( apds.init() ) {
    Serial.println(F("APDS-9960 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  }

  // Adjust the Proximity sensor gain
  if ( !apds.setProximityGain(PGAIN_2X) ) {
    Serial.println(F("Something went wrong trying to set PGAIN"));
  }

  // Start running the APDS-9960 proximity sensor (no interrupts)
  if ( apds.enableProximitySensor(false) ) {
    Serial.println(F("Proximity sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during sensor init!"));
  }

  pinMode(GREEN_OUTPUT, OUTPUT);
  pinMode(BLUE_OUTPUT, OUTPUT);
  pinMode(RED_OUTPUT, OUTPUT);
  pinMode(WW_OUTPUT, OUTPUT);
  set_light_level(254); // Start with very bright but not max since max is not correct temperature
}

void loop() {
  uint8_t proximity_data = 0;
  if ( !apds.readProximity(proximity_data) ) {
    Serial.println("Error reading proximity value");
  } else {
    if (proximity_data > MAX_DISTANCE) {
      Serial.print("Proximity: ");
      Serial.println(proximity_data);
      set_light_level(proximity_data);
    }
  }
}

void set_light_level (byte level) {
  if (level < MIN_LEVEL) {
    turn_off();
    last_lock_level = level;
    last_lock_time = millis();
  } else {
    if (abs(last_lock_level - level) < LOCK_THRESHOLD) {
      if (millis() - last_lock_time > LOCK_TIME) {
        // Level has been held steady for a while. Blink to acknowledge and sleep to allow time to remove hand from sensor.
        turn_off();
        busy_wait(150);
        adjust(level); // Set level back to selected level
        last_lock_level = 0;
        last_lock_time = 0;
        busy_wait(LOCK_TIME * 2);
      } else {
        adjust(level);
      }
    } else {
      // Level was significantly changed.
      adjust(level);
      last_lock_level = level;
      last_lock_time = millis();
    }
  }
}

void turn_off() {
  analogWrite(GREEN_OUTPUT, 0);
  analogWrite(BLUE_OUTPUT, 0);
  analogWrite(RED_OUTPUT, 0);
  analogWrite(WW_OUTPUT, 0);
}

void adjust(byte level) {
  Serial.print("level:");
  Serial.print(level);
  Serial.print(" adj:");
  if (level == 255) {
    digitalWrite(GREEN_OUTPUT, HIGH);
    digitalWrite(BLUE_OUTPUT, HIGH);
    digitalWrite(RED_OUTPUT, HIGH);
    digitalWrite(WW_OUTPUT, HIGH);
  } else {
    float adjustment = (1.0 + level) / 255.0;
    Serial.print(adjustment);
    
    analogWrite(GREEN_OUTPUT, max(1, GREEN_BASE_LEVEL * adjustment));
    Serial.print(" green:");
    Serial.print(GREEN_BASE_LEVEL * adjustment);
    
    analogWrite(BLUE_OUTPUT, max(1, BLUE_BASE_LEVEL * adjustment));
    Serial.print(" blue:");
    Serial.print(BLUE_BASE_LEVEL * adjustment);
    
    analogWrite(RED_OUTPUT, max(1, RED_BASE_LEVEL * adjustment));
    Serial.print(" red:");
    Serial.print(RED_BASE_LEVEL * adjustment);

    analogWrite(WW_OUTPUT, max(1, WW_BASE_LEVEL * adjustment));
    Serial.print(" ww:");
    Serial.println(WW_BASE_LEVEL * adjustment);
  }
}

void busy_wait(unsigned long delay) {
  unsigned long delay_start = millis();
  while (millis() - delay_start < delay) {
    // Do nothing
  }
}
