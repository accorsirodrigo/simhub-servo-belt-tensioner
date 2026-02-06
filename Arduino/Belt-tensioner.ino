#include <Servo.h>
#include <EEPROM.h>

#define LED 17
#define SERVO_LEFT 2
#define SERVO_RIGHT 3
#define EEPROM_LADD 0
#define EEPROM_RADD 1
#define MASK_7BIT 0x7F
#define IDLE_TIMEOUT 1000000UL

Servo servo_left, servo_right;
unsigned long last_entry = millis(), timeout = 0;
int next = HIGH;
byte ladd = 63, radd = 65;
byte led_state = 0;
const byte LED_TIMING[3][2] = {{30,30}, {180,40}, {40,180}};

void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  
  byte eeprom_ladd = EEPROM.read(EEPROM_LADD);
  byte eeprom_radd = EEPROM.read(EEPROM_RADD);
  
  if (eeprom_ladd != 255) ladd = eeprom_ladd;
  if (eeprom_radd != 255) radd = eeprom_radd;
  
  servo_left.attach(SERVO_LEFT);
  servo_right.attach(SERVO_RIGHT);
  servo_left.write(ladd);
  servo_right.write(radd);
  
  Serial.begin(19200);
  delay(100);
  Serial.println("Belt tensioner servo connected");
}

void loop() {
  unsigned long current_millis = millis();
  if (last_entry < current_millis) {
    if (timeout > IDLE_TIMEOUT) {
      led_state = 0;
      timeout = 0;
    }
    next = !next;
    last_entry = current_millis + LED_TIMING[led_state][next];
    digitalWrite(LED, next);
    if (led_state == 0) delay(30);
  }
  if (Serial.available()) {
    byte received = Serial.read();
    if (received < 2) {
      while (!Serial.available()) delay(1);
      byte add = Serial.read();
      if (received & 1) {
        radd = add;
        EEPROM.update(EEPROM_RADD, radd);
      } else {
        ladd = add;
        EEPROM.update(EEPROM_LADD, ladd);
      }
    } else {
      (received & 1) ? servo_right.write((received & MASK_7BIT) + radd) : servo_left.write((received & MASK_7BIT) + ladd);
      led_state = (received & 1) ? 2 : 1;
    }
    timeout = 0;
  } else timeout++;
}