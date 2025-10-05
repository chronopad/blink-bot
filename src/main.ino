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

// Menu and states
const char* menuItems[] = {"Timer", "Stopwatch", "Info"};
const int menuLength = 3;
int selectedItem = 0;

bool showHome = true;
bool showMenu = false;
bool showTimer = false;
bool showStopwatch = false;
bool showInfo = false;

unsigned long lastInteraction = 0;
const unsigned long idleTimeout = 5000; 

// Timer variables
unsigned long _timerBegin = 0;
bool timerRunning = false;

// Stopwatch variables
unsigned long _stopwatchBegin = 0;
bool stopwatchRunning = false;

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
            
            if (showHome) {
                showHome = false;
                showMenu = true;
                drawMenu();
            } else if (showMenu) {
                if (i == 0) {
                    selectedItem--;
                    if (selectedItem < 0) selectedItem = menuLength - 1;
                    drawMenu();
                } else if (i == 1) {
                    selectedItem++;
                    if (selectedItem >= menuLength) selectedItem = 0;
                    drawMenu();
                } else if (i == 2) {
                    selectMenuItem(selectedItem);
                } else if (i == 3) {
                    showMenu = false;
                    showHome = true;
                    drawHome();
                }
            } else {
                // Item Mode: Timer
                if (showTimer) {
                    // Button 3: Toggle start / stop
                    if (i == 2) {
                        timerRunning = !timerRunning;
                        if (timerRunning) _timerBegin = millis();
                    } 
                    // Button 4: Back to menu
                    else if (i == 3) {
                        showTimer = false;
                        showMenu = true;
                        drawMenu();
                    }
                }
                
                // Item Mode: Stopwatch
                else if (showStopwatch) {
                    // Button 3: Toggle start / stop
                    if (i == 2) {
                        stopwatchRunning = !stopwatchRunning;
                        if (stopwatchRunning) _stopwatchBegin = millis();
                    }
                    // Button 4: Back to menu
                    else if (i == 3) {
                        showStopwatch = false;
                        showMenu = true;
                        drawMenu();
                    }
                }
                
                // Item Mode: Info
                else if (showInfo) {
                    // Button 4: Back to menu
                    if (i == 3) {
                        showInfo = false;
                        showMenu = true;
                        drawMenu();
                    }
                }
            }
        }
        lastButtonStates[i] = buttonStates[i];
    }
    
    // Functionality: Idle returns to home
    if ((showMenu || showTimer || showStopwatch || showInfo) && (millis() - lastInteraction > idleTimeout)) {
        showMenu = showTimer = showStopwatch = showInfo = false;
        showHome = true;
        selectedItem = 0;
        drawHome();
    }
    
    // Functionality: Item modes
    updateTimer();
    updateStopwatch();
    
    delay(10);
}

void selectMenuItem(int index) {
    showMenu = false;
    
    switch (index) {
        case 0:
        showTimer = true;
        drawTimer();
        break;
        case 1:
        showStopwatch = true;
        drawStopwatch();
        break;
        case 2:
        showInfo = true;
        drawInfo();
        break;
    }
}

void updateTimer() {
    if (showTimer && timerRunning) drawTimer();
}

void updateStopwatch() {
    if (showStopwatch && stopwatchRunning) drawStopwatch();
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

void drawTimer() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("TIMER (Btn3:Start/Stop, Btn4:Back)");
    
    unsigned long elapsed = 0;
    if (timerRunning) elapsed = (millis() - _timerBegin) / 1000;
    
    display.setTextSize(2);
    display.setCursor(40, 30);
    display.print(elapsed);
    display.println("s");
    display.display();
}

void drawStopwatch() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("STOPWATCH (Btn3:Start/Stop, Btn4:Back)");
    
    unsigned long elapsed = 0;
    if (stopwatchRunning) elapsed = (millis() - _stopwatchBegin) / 1000;
    
    display.setTextSize(2);
    display.setCursor(40, 30);
    display.print(elapsed);
    display.println("s");
    display.display();
}

void drawInfo() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("INFO");
    display.setCursor(10, 15);
    display.println("Arduino OLED UI Demo");
    display.setCursor(10, 25);
    display.println("Timer / Stopwatch / Info");
    display.setCursor(10, 45);
    display.display();
}