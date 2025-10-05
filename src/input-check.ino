#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int buttonPins[5] = {12, 13, 14, 25, 26};
int buttonStates[5];

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("5 Button Test Ready");
  display.display();

  for (int i = 0; i < 5; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP); 
  }
}

void loop() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Button States:");

  for (int i = 0; i < 5; i++) {
    buttonStates[i] = digitalRead(buttonPins[i]);
    bool pressed = (buttonStates[i] == LOW);

    display.setCursor(0, 12 + (i * 10));
    display.print("B");
    display.print(i + 1);
    display.print(": ");
    display.println(pressed ? "Pressed" : "Released");
  }

  display.display();
  delay(50);
}
