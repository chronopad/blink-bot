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

// Buzzer setup
#define BUZZER_PIN 32
#define NOTE_A4 440
#define NOTE_C5 523
#define NOTE_F5 698

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
bool timerFinished = false;
int timerMinutes = 0; 
int timerSeconds = 10; 
unsigned long timerTotalSeconds = timerMinutes * 60 + timerSeconds;
bool buzzerPlayedTimerFinished = false;

// Stopwatch variables
unsigned long _stopwatchBegin = 0;
bool stopwatchRunning = false;

void setup() {
    Serial.begin(115200);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }

    pinMode(BUZZER_PIN, OUTPUT);
    
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
                } else if (i == 4) {
                    selectMenuItem(selectedItem);
                } else if (i == 5) { 
                    showMenu = false;
                    showHome = true;
                    drawHome();
                }
            } else {
                // Item Mode: Timer
                if (showTimer) {
                    // Button 5: Toggle start / stop
                    if (i == 4) { 
                        if (!timerFinished) {
                            timerRunning = !timerRunning;
                            if (timerRunning) {
                                _timerBegin = millis();
                                buzzerBeepStart();
                            } else {
                                buzzerBeepStop();
                            }
                        }
                    } 
                    // Button 6: Back to menu
                    else if (i == 5) { 
                        timerRunning = false;
                        timerFinished = false;
                        buzzerPlayedTimerFinished = false;
                        showTimer = false;
                        showMenu = true;
                        drawMenu();
                    }
                }
                
                // Item Mode: Stopwatch
                else if (showStopwatch) {
                    // Button 5: Toggle start / stop
                    if (i == 4) {
                        stopwatchRunning = !stopwatchRunning;
                        if (stopwatchRunning) {
                            _stopwatchBegin = millis();
                            buzzerBeepStart();
                        } else {
                            buzzerBeepStop();
                        }
                    }
                    // Button 6: Back to menu
                    else if (i == 5) {
                        showStopwatch = false;
                        showMenu = true;
                        drawMenu();
                    }
                }
                
                // Item Mode: Info
                else if (showInfo) {
                    // Button 6: Back to menu
                    if (i == 5) { 
                        showInfo = false;
                        showMenu = true;
                        drawMenu();
                    }
                }
            }
        }
        lastButtonStates[i] = buttonStates[i];
    }
    
    // Return to home after idle timeout
    if ((showMenu) && (millis() - lastInteraction > idleTimeout)) {
        showMenu = false;
        showHome = true;
        selectedItem = 0;
        drawHome();
    }
    
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

// ----- BUZZER SECTION -----

void buzzerBeepStart() {
    tone(BUZZER_PIN, NOTE_C5);
    delay(100);
    noTone(BUZZER_PIN);
}

void buzzerBeepStop() {
    tone(BUZZER_PIN, NOTE_A4);
    delay(100);
    noTone(BUZZER_PIN);
}

void buzzerTimerFinished() {
    for (int i = 0; i < 3; i++) {
        tone(BUZZER_PIN, NOTE_F5);
        delay(150);
        noTone(BUZZER_PIN);
        delay(150);
    }
}

// ----- MENU ITEMS SECTION -----

void updateTimer() {
    if (showTimer && timerRunning) drawTimer();

    if (showTimer && timerFinished && !buzzerPlayedTimerFinished) {
        buzzerTimerFinished();
        buzzerPlayedTimerFinished = true;
    }
}

void updateStopwatch() {
    if (showStopwatch && stopwatchRunning) drawStopwatch();
}

// ----- DISPLAY SECTION -----

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
    display.println("TIMER (Btn5:Start/Stop, Btn6:Back)");
    
    unsigned long elapsed = 0;
    if (timerRunning) elapsed = (millis() - _timerBegin) / 1000;

    long remaining = timerTotalSeconds - elapsed;
    if (remaining <= 0) {
        remaining = 0;
        timerRunning = false;
        timerFinished = true;
    }

    int minutes = remaining / 60;
    int seconds = remaining % 60;

    char timeStr[6];
    sprintf(timeStr, "%02d:%02d", minutes, seconds);

    display.setTextSize(3);
    display.setCursor(25, 25);
    display.print(timeStr);

    if (timerFinished) {
        display.setTextSize(1);
        display.setCursor(40, 55);
        display.print("DONE!");
    }

    display.display();
}

void drawStopwatch() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("STOPWATCH (Btn5:Start/Stop, Btn6:Back)");

    unsigned long elapsed = 0;
    if (stopwatchRunning) elapsed = millis() - _stopwatchBegin;

    int minutes = (elapsed / 60000) % 60;
    int seconds = (elapsed / 1000) % 60;
    int centiseconds = (elapsed / 10) % 100;

    char timeStr[10];
    sprintf(timeStr, "%02d:%02d:%02d", minutes, seconds, centiseconds);

    display.setTextSize(2);
    display.setCursor(15, 30);
    display.print(timeStr);
    display.display();
}

void drawInfo() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("INFO");
    display.setCursor(10, 15);
    display.println("Blink Bot");
    display.setCursor(10, 25);
    display.println("Lorem ipsum..");
    display.setCursor(10, 45);
    display.display();
}
