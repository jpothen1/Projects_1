#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#ifndef oled_h
#define oled_h

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SSD1306_I2C_ADDRESS 0x3C
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void oled_setup(){
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false)) { // Change address as needed
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(WHITE);
}

#endif
