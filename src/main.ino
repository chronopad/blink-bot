#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Button setup
const int buttonPins[6] = {12, 13, 14, 25, 26, 27};
int buttonStates[6];
int lastButtonStates[6];
unsigned long lastInteraction = 0;

// Menu setup
const char* menuItems[] = {"A", "B", "C"};
const int menuLength = 3;
int selectedItem = 0;
bool showMenu = false;

// Idle timeout (in ms) before returned to menu
const unsigned long idleTimeout = 5000;

void setup() {
    Serial.begin(115200);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }

    for (int i = 0; i < 6; i++) {
        pinMode(buttonPins[i], INPUT_PULLUP);
        lastButtonStates[i] = HIGH;
    }

    drawHome();
}

void loop() {
    bool anyPressed = false;
    for (int i = 0; i < 6; i++) {
        buttonStates[i] = digitalRead(buttonPins[i]);

        if (buttonStates[i] == LOW && lastButtonStates[i] == HIGH) {
            anyPressed = true;
            lastInteraction = millis();

            if (!showMenu) {
                showMenu = true;
                drawMenu();
            } else {
                if (i == 0) {
                    selectedItem--;
                    if (selectedItem < 0) selectedItem = menuLength - 1;
                    drawMenu();
                } else if (i == 1) {
                    selectedItem++;
                    if (selectedItem >= menuLength) selectedItem = 0;
                    drawMenu();
                }
            }
        }
        lastButtonStates[i] = buttonStates[i];
    }

    if (showMenu && (millis() - lastInteraction > idleTimeout)) {
        showMenu = false;
        drawHome();
    }

    delay(50);
}

void drawHome() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 20);
  display.println("Blink Bot");
  display.setTextSize(1);
  display.setCursor(10, 50);
  display.println("Press any button...");
  display.display();
}

void drawMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Menu:");

  for (int i = 0; i < menuLength; i++) {
    display.setCursor(10, 15 + (i * 10));
    if (i == selectedItem) {
      display.print("> ");
    } else {
      display.print("  ");
    }
    display.println(menuItems[i]);
  }

  display.display();
}