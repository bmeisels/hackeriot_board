#define EEPROM_INIT

#include <at24c04.h>

#include "Adafruit_NeoPixel.h"
#include "Adafruit_Debounce.h"

#ifdef I2C_SCAN
#include <Wire.h>
#endif

#define NEXT_BUTTON_PIN PA0
#define PREV_BUTTON_PIN PA1
#define NEOPIXEL_PIN PB7
#define NUMPIXELS 4
#define DEBOUNCE_DELAY 10

#define COLOR_CYAN 0x00FFFF
#define COLOR_PURPLE 0xFF00FF
#define COLOR_YELLOW 0xFFFF00
#define COLOR_WHITE 0xFFFFFF
#define COLOR_RED 0xFF0000

static const uint32_t boot_colors[] = { COLOR_PURPLE, COLOR_CYAN, COLOR_YELLOW, COLOR_WHITE };

#define LINE_BUFFER_SIZE 80

//#define EEPROM_MAGIC 'HHRW'
#define EEPROM_MAGIC 0x48485257UL

struct eeprom_header {
  uint32_t magic;
  byte palette_length;
  byte palette_curr;  // not used since we do not write the eeprom normally
};

// global variables
AT24C04 eeprom(AT24C04_ADDRESS_0);
Adafruit_Debounce button_previous(PREV_BUTTON_PIN, LOW);
Adafruit_Debounce button_next(NEXT_BUTTON_PIN, LOW);
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN);
byte alt_mode = 0;
byte palette_length = 0;
byte palette_curr = 0;
uint32_t color_curr = 0;

void pixels_boot_sequence(uint32_t ms, uint32_t color) {
  byte i;
  for (i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, color);
    pixels.show();
    delay(ms);
  }
  for (i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, 0);
    pixels.show();
    delay(ms);
  }
  for (i = 0; i < NUMPIXELS; i++) {
    pixels.clear();
    pixels.show();
    delay(ms);

    pixels.fill(color, 0, NUMPIXELS);
    pixels.show();
    delay(ms);
  }
}

void pixels_blink_infinitely(uint32_t ms, byte idx, uint32_t color) {
  for (;;) {
    pixels.setPixelColor(idx, color);
    pixels.show();
    delay(ms);
    pixels.setPixelColor(idx, 0);
    pixels.show();
    delay(ms);
  }
}

#ifdef I2C_SCAN
byte i2c_scan() {
  byte nDevices = 0;

  for (byte addr = 1; addr < 127; ++addr) {
    /* the return value of Write.endTransmission 
       tells us whether a device replied with ACK */

    Wire.beginTransmission(addr);
    byte error = Wire.endTransmission();

    if (!error) {
      nDevices++;
      Serial.print("I2C device found");
    } else if (error == 4) {
      Serial.print("Unknown error");
    } else continue;

    Serial.print(" at address 0x");
    if (addr < 16)
      Serial.print("0");
    Serial.println(addr, HEX);
  }
  if (!nDevices) Serial.println("No I2C devices found");

  return nDevices;
}
#endif

#ifdef EEPROM_INIT
void eeprom_init() {
  static const byte colors[] = {
    // from https://www.color-hex.com/color-palette/1050148
    0xff, 0x8d, 0x1f,  // orange
    0x56, 0xff, 0xd0,  // cyan
    0xfd, 0x39, 0x9b,  // hot pink
    0xcc, 0xff, 0x37,  // light green
    0x9c, 0x42, 0xff,  // purple

    // from https://www.color-hex.com/color-palette/1038606
    0xff, 0x14, 0x93,  // neon pink
    0x48, 0xdb, 0x48,  // neon green
    0xb1, 0x6e, 0xec,  // neon purple
    0xff, 0xab, 0x33,  // neon orange
    0x2b, 0xcd, 0xff,  // neon light blue
  };

  static const struct eeprom_header initial = {
    .magic = EEPROM_MAGIC,
    .palette_length = (sizeof colors) / (3 * sizeof *colors),
    .palette_curr = 0
  };
  eeprom.put(0x0, initial);
  if (!eeprom.getLastError())
    eeprom.put(sizeof initial, colors);

  if (eeprom.getLastError()) {
    Serial.println("Cannot initialize I2C EEPROM; aborting");
    pixels_blink_infinitely(250, 1, COLOR_RED);
  }
}
#endif

void eeprom_read_color() {
  byte rgb[3];
  uint16_t addr = sizeof(struct eeprom_header) + 3 * palette_curr;
  if (sizeof rgb != eeprom.readBuffer(addr, rgb, sizeof rgb)) {
    Serial.println("Cannot read current color; aborting");
    pixels_blink_infinitely(250, 3, COLOR_RED);
  }
  color_curr = pixels.Color(rgb[0], rgb[1], rgb[2]);
  Serial.print("Current color is ");
  Serial.println(color_curr, HEX);
}


#ifdef UART_CLI
void handle_uart_commands() {
  static bool console_active = false;
  static char line[LINE_BUFFER_SIZE] = "";

  if (!console_active) {
    console_active = true;
    Serial.println("Command interface active!\nType HELP for a list of available commands");
  }

  String line_string = Serial.readStringUntil('\n');
  if (line_string.length() >= LINE_BUFFER_SIZE) {
    Serial.println("Input too long");
    return;
  }
  line_string.toLowerCase();
  line_string.toCharArray(line, LINE_BUFFER_SIZE);
  char *cmd = strtok(line, " ");
  if (!cmd) {
    Serial.println("Cannot read command");
    return;
  }
  if (!strcmp(cmd, "help")) {
    Serial.println("The following commands are available:\nHELP\nLED\n");
    Serial.println("You can for instance type HELP LED for more info on the LED command.");
  } else if (!strcmp(cmd, "led")) {
    Serial.println("Not yet implemented");
  } else {
    Serial.print("Unknown command: ");
    Serial.println(cmd);
  }

  // https://www.norwegiancreations.com/2018/02/creating-a-command-line-interface-in-arduinos-serial-monitor/
  // https://github.com/PulseRain/PulseRainUARTConsole/blob/master/src/PulseRainUARTConsole.cpp
}
#endif

void setup() {
  // setup UART [hardware UART2: PA2-TX, PA3-RX]
  Serial.begin(115200);
  Serial.println("Hackeriot 2024 board starting!");

  // setup prev/next buttons and detect alt mode
  button_previous.begin();
  button_next.begin();

  if (alt_mode = 2 * button_previous.isPressed() + button_next.isPressed()) {
    Serial.print("A button is pressed; setting alternative mode ");
    Serial.println(alt_mode);
  }

  // setup the four neopixels
  pixels.begin();
  pixels.setBrightness(35);
  Serial.println("Pixels boot sequence...");
  pixels_boot_sequence(250, boot_colors[alt_mode]);

  // setup I2C [hardware I2C1: PA9_R-SCL, PA10_R-SDA]
  Wire.begin();

  // read header from AT24C04 eeprom (4 kbit = 512 bytes)
  struct eeprom_header header = {};
  eeprom.get(0, header);
  byte err = eeprom.getLastError();
  if (err) {
    if (err == 2) {  // no answer
#ifdef I2C_SCAN
      Serial.println("No response from I2C EEPROM; scanning...");
      i2c_scan();
#else
      Serial.println("No response from I2C EEPROM; aborting");
#endif
    } else {
      Serial.println("Unknown I2C EEPROM error; aborting");
    }
    pixels_blink_infinitely(250, 0, COLOR_RED);
  } else {  // all OK
    Serial.println("Header read from I2C EEPROM");
  }

  // verify magic
  if (header.magic != EEPROM_MAGIC) {
#ifdef EEPROM_INIT
    Serial.println("Unexpected I2C EEPROM contents; initializing");
    eeprom_init();
#else
    Serial.println("Unexpected I2C EEPROM contents; aborting");
    Serial.println(header.magic, HEX);
    pixels_blink_infinitely(250, 1, COLOR_RED);
#endif
  } else if (alt_mode == 3) {
#ifdef EEPROM_INIT
    Serial.println("Initializing I2C EEPROM");
    eeprom_init();
    alt_mode = 0;
#endif
  }

  // roughly 10, and in any case not more than (512-6)/3=168
  palette_length = header.palette_length;
  Serial.print("Number of colors in I2C EEPROM is ");
  Serial.println(palette_length);

  palette_curr = header.palette_curr;
  if (palette_curr >= palette_length) {
    Serial.println("Invalid current color; aborting");
    pixels_blink_infinitely(250, 2, COLOR_RED);
  }
  eeprom_read_color();

  Serial.println("Setup test complete\nPlease press the buttons, or type in commands");
}

void normal_mode() {
  static uint32_t frame = 0;

  button_previous.update();
  button_next.update();

  if (button_previous.justPressed()) {
    Serial.println("Previous button pressed!");

    if (!palette_curr) palette_curr = palette_length;
    --palette_curr;
    eeprom_read_color();
  }

  if (button_next.justPressed()) {
    Serial.println("Next button pressed!");

    ++palette_curr;
    if (palette_curr == palette_length) palette_curr = 0;
    eeprom_read_color();
  }

  // TODO: rework stupid animation
  pixels.clear();
  //pixels.setPixelColor((frame++ >> 6) % NUMPIXELS, color_curr);
  byte tmp = (frame++ >> 6) % (2 * NUMPIXELS - 2);
  pixels.setPixelColor(tmp < NUMPIXELS ? tmp : 2 * NUMPIXELS - 2 - tmp, color_curr);  // KITT
  pixels.show();

#ifdef UART_CLI
  if (Serial.available()) {
    handle_uart_commands();
  }
#endif

  delay(DEBOUNCE_DELAY);
}

void rainbow() {
  for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256) {
    pixels.rainbow(firstPixelHue);
    pixels.show();
    delay(10);
  }
}

  void theaterChaseRainbow() {
    int firstPixelHue = 0;
    for (int a = 0; a < 30; a++) {
      for (int b = 0; b < 3; b++) {
        pixels.clear();
 
        for (int c = b; c < pixels.numPixels(); c += 3) {
          int hue = firstPixelHue + c * 65536L / pixels.numPixels();
          uint32_t color = pixels.gamma32(pixels.ColorHSV(hue));
          pixels.setPixelColor(c, color);
        }
        pixels.show();
        delay(400);
        firstPixelHue += 65536 / 90;
      }
    }
  }

    void loop() {
      if (alt_mode == 0) {
        normal_mode();
      } else if (alt_mode == 1) {
        rainbow();
      } else if (alt_mode == 2) {
        theaterChaseRainbow();
      }
    }