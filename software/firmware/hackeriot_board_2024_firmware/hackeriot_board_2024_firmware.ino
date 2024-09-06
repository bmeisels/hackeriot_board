#include "Adafruit_NeoPixel.h"
#include "Adafruit_EEPROM_I2C.h"
#include "Adafruit_Debounce.h"


#define PREV_BUTTON_PIN PA_0
#define NEXT_BUTTON_PIN PA_1
#define NEOPIXEL_PIN PB_7
#define NUMPIXELS 4
#define EEPROM_ADDRESS 0x50
#define DEBOUNCE_DELAY 10

Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_EEPROM_I2C i2ceeprom;
Adafruit_Debounce button_previous(PREV_BUTTON_PIN, LOW);
Adafruit_Debounce button_next(NEXT_BUTTON_PIN, LOW);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); // PA2 - TX, PA3 - RX
  Serial.println("booted hackeriot board");
  button_previous.begin();
  button_next.begin();

  pixels.begin();

  pixels.fill(Adafruit_NeoPixel::Color(0, 255, 0), 0 ,4);

  if (i2ceeprom.begin(EEPROM_ADDRESS)) {
    Serial.println("Found I2C EEPROM");
  } else {
    Serial.println("I2C EEPROM not identified ... check your connections?\r\n");
    while (1) delay(10);
  }


  Serial.println("Setup Test complete");
  Serial.println("Please press the buttons");
}

void loop() {
  button_previous.update();
  button_next.update();

  if (button_previous.justPressed()) {
    Serial.println("Previous Button was just pressed!");
  }
  
  if (button_next.justPressed()) {
    Serial.println("Next Button was just pressed!");
  }


  delay(DEBOUNCE_DELAY);
}
