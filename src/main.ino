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
const char* menuItems[] = {"Timer", "Stopwatch", "Pomodoro", "Info"};
const int menuLength = 4;
int selectedItem = 0;

bool showHome = true;
bool showMenu = false;
bool showTimer = false;
bool showStopwatch = false;
bool showInfo = false;

unsigned long lastInteraction = 0;
const unsigned long idleTimeout = 10000;

// Submenu variables
bool showSubMenu = false;
int subMenuIndex = 0;
int subMenuLength = 2;
const char* subMenuItems[] = {"CONTINUE", "RESET"};

// Timer variables
unsigned long _timerBegin = 0;
bool timerRunning = false;
bool timerFinished = false;
int timerMinutes = 1;
int timerSeconds = 0;
unsigned long timerTotalSeconds = timerMinutes * 60 + timerSeconds;
bool buzzerPlayedTimerFinished = false;
unsigned long timerPausedSeconds = timerTotalSeconds;

// Timer edit pointer
enum TimerEditField { EDIT_MINUTE, EDIT_SECOND };
TimerEditField timerEditField = EDIT_MINUTE;

unsigned long lastBlinkTime = 0;
bool blinkVisible = true;
const unsigned long blinkInterval = 500;

// Stopwatch variables
unsigned long _stopwatchBegin = 0;
bool stopwatchRunning = false;
unsigned long stopwatchPausedMillis = 0;

// Pomodoro variables
bool showPomodoro = false;
enum PomodoroPhase { POMODORO_WORK, POMODORO_BREAK };
PomodoroPhase pomodoroPhase = POMODORO_WORK;

// const int pomodoroWorkMinutes = 25;
// const int pomodoroBreakMinutes = 5;
// unsigned long pomodoroWorkSeconds = pomodoroWorkMinutes * 60;
// unsigned long pomodoroBreakSeconds = pomodoroBreakMinutes * 60;
unsigned long pomodoroWorkSeconds = 10;
unsigned long pomodoroBreakSeconds = 5;

unsigned long pomodoroStartMillis = 0;
unsigned long pomodoroRemainingSeconds = pomodoroWorkSeconds;
bool pomodoroRunning = false;
bool pomodoroFinished = false;
bool buzzerPlayedPomodoroFinished = false;

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
                    // Timer configuration logic (only when timer is not running or finished)
                    if (!timerRunning && !timerFinished && !showSubMenu) {
                        if (i == 2) { // Left
                            timerEditField = EDIT_MINUTE;
                            drawTimer();
                        } else if (i == 3) { // Right
                            timerEditField = EDIT_SECOND;
                            drawTimer();
                        } else if (i == 0) { // Increase
                            if (timerEditField == EDIT_MINUTE) {
                                timerMinutes = (timerMinutes + 1) % 60;
                            } else if (timerEditField == EDIT_SECOND) {
                                timerSeconds = (timerSeconds + 1) % 60;
                            }
                            timerTotalSeconds = timerMinutes * 60 + timerSeconds;
                            timerPausedSeconds = timerTotalSeconds;
                            drawTimer();
                        } else if (i == 1) { // Decrease
                            if (timerEditField == EDIT_MINUTE) {
                                timerMinutes = (timerMinutes == 0) ? 59 : timerMinutes - 1;
                            } else if (timerEditField == EDIT_SECOND) {
                                timerSeconds = (timerSeconds == 0) ? 59 : timerSeconds - 1;
                            }
                            timerTotalSeconds = timerMinutes * 60 + timerSeconds;
                            timerPausedSeconds = timerTotalSeconds;
                            drawTimer();
                        }
                    }


                    if (showSubMenu) {
                        if (i == 2) {
                            subMenuIndex--;
                            if (subMenuIndex < 0) subMenuIndex = subMenuLength - 1;
                            drawTimer();
                        } else if (i == 3) {
                            subMenuIndex++;
                            if (subMenuIndex >= subMenuLength) subMenuIndex = 0;
                            drawTimer();
                        } else if (i == 4) {
                            if (subMenuIndex == 0) {
                                // Continue
                                timerRunning = true;
                                _timerBegin = millis() - (timerTotalSeconds - timerPausedSeconds) * 1000;
                                showSubMenu = false;
                                buzzerStartStop(true);
                            } else {
                                // Reset
                                timerRunning = false;
                                timerFinished = false;
                                buzzerPlayedTimerFinished = false;
                                timerPausedSeconds = timerTotalSeconds;  // Reset to full time
                                showSubMenu = false;
                                drawTimer();
                                buzzerStartStop(false);
                            }
                        } else if (i == 5) {
                            timerRunning = false;
                            timerFinished = false;
                            buzzerPlayedTimerFinished = false;
                            timerPausedSeconds = timerTotalSeconds;
                            _timerBegin = 0;
                            showSubMenu = false;
                            showTimer = false;
                            showMenu = true;
                            buzzerStartStop(false);
                            drawMenu();
                        }
                    } else {
                        if (i == 4) {
                            if (timerFinished) {
                                timerRunning = false;
                                timerFinished = false;
                                buzzerPlayedTimerFinished = false;
                                timerPausedSeconds = timerTotalSeconds;
                                _timerBegin = 0;
                                drawTimer();
                                buzzerStartStop(false);
                            } else {
                                if (timerRunning) {
                                    // Pause timer
                                    timerPausedSeconds = getRemainingTimer();
                                    timerRunning = false;
                                    showSubMenu = true;
                                    subMenuIndex = 0;
                                    buzzerStartStop(false);
                                    drawTimer();
                                } else {
                                    // Start or resume
                                    if (timerPausedSeconds == 0 || timerPausedSeconds == timerTotalSeconds) {
                                        _timerBegin = millis();
                                        timerPausedSeconds = timerTotalSeconds;
                                    } else {
                                        _timerBegin = millis() - (timerTotalSeconds - timerPausedSeconds) * 1000;
                                    }
                                    timerRunning = true;
                                    timerFinished = false;
                                    buzzerStartStop(true);
                                }
                            }
                        } else if (i == 5) {
                            timerRunning = false;
                            timerFinished = false;
                            buzzerPlayedTimerFinished = false;
                            timerPausedSeconds = timerTotalSeconds;
                            showTimer = false;
                            showMenu = true;
                            drawMenu();
                        }
                    }
                }

                // Item Mode: Stopwatch
                else if (showStopwatch) {
                    if (showSubMenu) {
                        if (i == 2) {
                            subMenuIndex--;
                            if (subMenuIndex < 0) subMenuIndex = subMenuLength - 1;
                            drawStopwatch();
                        } else if (i == 3) {
                            subMenuIndex++;
                            if (subMenuIndex >= subMenuLength) subMenuIndex = 0;
                            drawStopwatch();
                        } else if (i == 4) {
                            if (subMenuIndex == 0) {
                                stopwatchRunning = true;
                                _stopwatchBegin = millis() - stopwatchPausedMillis;
                                showSubMenu = false;
                                buzzerStartStop(true);
                            } else {
                                stopwatchRunning = false;
                                _stopwatchBegin = 0;
                                stopwatchPausedMillis = 0;
                                showSubMenu = false;
                                buzzerStartStop(false);
                                drawStopwatch();
                            }
                        } else if (i == 5) {
                            stopwatchRunning = false;
                            stopwatchPausedMillis = 0;
                            _stopwatchBegin = 0;
                            showSubMenu = false;
                            showStopwatch = false;
                            showMenu = true;
                            buzzerStartStop(false);
                            drawMenu();
                        }
                    } else {
                        if (i == 4) {
                            if (stopwatchRunning) {
                                stopwatchPausedMillis = getElapsedStopwatch();
                                stopwatchRunning = false;
                                showSubMenu = true;
                                subMenuIndex = 0;
                                buzzerStartStop(false);
                                drawStopwatch();
                            } else {
                                stopwatchRunning = true;
                                _stopwatchBegin = millis() - stopwatchPausedMillis;
                                buzzerStartStop(true);
                            }
                        } else if (i == 5) {
                            stopwatchRunning = false;
                            stopwatchPausedMillis = 0;
                            showStopwatch = false;
                            showMenu = true;
                            drawMenu();
                        }
                    }
                }

                // Item Mode: Pomodoro
                else if (showPomodoro) {
                    if (showSubMenu) {
                        // Navigate submenu
                        if (i == 2) { // Left
                            subMenuIndex--;
                            if (subMenuIndex < 0) subMenuIndex = subMenuLength - 1;
                            drawPomodoro();
                        } else if (i == 3) { // Right
                            subMenuIndex++;
                            if (subMenuIndex >= subMenuLength) subMenuIndex = 0;
                            drawPomodoro();
                        } else if (i == 4) { // Select
                            if (subMenuIndex == 0) { // Continue
                                unsigned long total = (pomodoroPhase == POMODORO_WORK) ? pomodoroWorkSeconds : pomodoroBreakSeconds;
                                pomodoroStartMillis = millis() - ((total - pomodoroRemainingSeconds) * 1000);
                                pomodoroRunning = true;
                                showSubMenu = false;
                                buzzerStartStop(true);
                            } else { // Reset
                                pomodoroPhase = POMODORO_WORK;
                                pomodoroRemainingSeconds = pomodoroWorkSeconds;
                                pomodoroRunning = false;
                                pomodoroFinished = false;
                                showSubMenu = false;
                                drawPomodoro();
                            }
                        } else if (i == 5) { // Back
                            pomodoroRunning = false;
                            pomodoroFinished = false;
                            showSubMenu = false;
                            showPomodoro = false;
                            showMenu = true;
                            drawMenu();
                        }
                    } else {
                        if (i == 4) { // Start / Pause
                            if (pomodoroRunning) {
                                pomodoroRemainingSeconds = getRemainingPomodoro();
                                pomodoroRunning = false;
                                showSubMenu = true;
                                subMenuIndex = 0;
                                buzzerStartStop(false);
                                drawPomodoro();
                            } else {
                                unsigned long total = (pomodoroPhase == POMODORO_WORK) ? pomodoroWorkSeconds : pomodoroBreakSeconds;
                                pomodoroStartMillis = millis() - ((total - pomodoroRemainingSeconds) * 1000);
                                pomodoroRunning = true;
                                pomodoroFinished = false;
                                buzzerStartStop(true);
                                drawPomodoro();
                            }
                        } else if (i == 5) { // Back
                            pomodoroRunning = false;
                            pomodoroFinished = false;
                            showPomodoro = false;
                            showMenu = true;
                            drawMenu();
                        }
                    }
                }


                // Item Mode: Info
                else if (showInfo) {
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

    if ((showMenu) && (millis() - lastInteraction > idleTimeout)) {
        showMenu = false;
        showHome = true;
        selectedItem = 0;
        drawHome();
    }

    updateTimer();
    updateStopwatch();
    updatePomodoro();

    delay(10);
}

void selectMenuItem(int index) {
    showMenu = false;

    switch (index) {
        case 0:
            timerTotalSeconds = timerMinutes * 60 + timerSeconds;
            timerPausedSeconds = timerTotalSeconds;
            showTimer = true;
            drawTimer();
            break;
        case 1:
            showStopwatch = true;
            drawStopwatch();
            break;
        case 2:
            showPomodoro = true;
            pomodoroPhase = POMODORO_WORK;
            pomodoroRemainingSeconds = pomodoroWorkSeconds;
            drawPomodoro();
            break;
        case 3:
            showInfo = true;
            drawInfo();
            break;
    }
}

// ----- BUZZER SECTION -----

void buzzerStartStop(bool isStart) {
    if (isStart) {
        tone(BUZZER_PIN, NOTE_C5);
        delay(100);
        noTone(BUZZER_PIN);
    } else {
        tone(BUZZER_PIN, NOTE_A4);
        delay(100);
        noTone(BUZZER_PIN);
    }
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

unsigned long getRemainingTimer() {
    if (!timerRunning) return timerPausedSeconds;
    unsigned long elapsed = (millis() - _timerBegin) / 1000;
    long remaining = timerTotalSeconds - elapsed;
    return (remaining > 0) ? remaining : 0;
}

unsigned long getElapsedStopwatch() {
    if (!stopwatchRunning) return stopwatchPausedMillis;
    return millis() - _stopwatchBegin;
}

unsigned long getRemainingPomodoro() {
    if (!pomodoroRunning) return pomodoroRemainingSeconds;
    unsigned long elapsed = (millis() - pomodoroStartMillis) / 1000;
    long remaining = ((pomodoroPhase == POMODORO_WORK) ? pomodoroWorkSeconds : pomodoroBreakSeconds) - elapsed;
    return (remaining > 0) ? remaining : 0;
}

void updateTimer() {
    if (showTimer && timerRunning) {
        unsigned long remaining = getRemainingTimer();
        drawTimer();

        if (remaining == 0) {
            timerRunning = false;
            timerFinished = true;
        }
    }

    if (showTimer && timerFinished && !buzzerPlayedTimerFinished) {
        buzzerTimerFinished();
        buzzerPlayedTimerFinished = true;
    }

    if (showTimer && !timerRunning && !timerFinished && !showSubMenu) {
        if (millis() - lastBlinkTime > blinkInterval) {
            blinkVisible = !blinkVisible;
            lastBlinkTime = millis();
            drawTimer();
        }
    }
}

void updateStopwatch() {
    if (showStopwatch && stopwatchRunning) drawStopwatch();
}

void updatePomodoro() {
    if (showPomodoro && pomodoroRunning) {
        unsigned long remaining = getRemainingPomodoro();
        drawPomodoro();

        if (remaining == 0 && !pomodoroFinished) {
            pomodoroRunning = false;
            pomodoroFinished = true;
            buzzerTimerFinished();

            if (pomodoroPhase == POMODORO_WORK) {
                pomodoroPhase = POMODORO_BREAK;
                pomodoroRemainingSeconds = pomodoroBreakSeconds;
            } else {
                pomodoroPhase = POMODORO_WORK;
                pomodoroRemainingSeconds = pomodoroWorkSeconds;
            }

            // Reset start time for next phase
            pomodoroStartMillis = millis();
            drawPomodoro();
        }

    }
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
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);

    unsigned long remaining = getRemainingTimer();

    if (!timerRunning && !timerFinished) remaining = timerPausedSeconds;

    int minutes = remaining / 60;
    int seconds = remaining % 60;

    display.setCursor(10, 20);
    display.printf("%02d:%02d", minutes, seconds);

    if (!timerRunning && !timerFinished && !showSubMenu && blinkVisible) {
        if (timerEditField == EDIT_MINUTE)
            display.fillRect(10, 45, 20, 2, SSD1306_WHITE);
        else if (timerEditField == EDIT_SECOND)
            display.fillRect(40, 45, 20, 2, SSD1306_WHITE);
    }

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("TIMER");

    if (showSubMenu) {
        int y = 52;                 
        int buttonWidth = 60;          
        int buttonHeight = 14;         

        for (int i = 0; i < subMenuLength; i++) {
            int x = i * buttonWidth; 

            if (i == subMenuIndex) {
                display.fillRect(x, y, buttonWidth, buttonHeight, SSD1306_WHITE);
                display.setTextColor(SSD1306_BLACK);
            } else {
                display.drawRect(x, y, buttonWidth, buttonHeight, SSD1306_WHITE);
                display.setTextColor(SSD1306_WHITE);
            }

            int16_t x1, y1;
            uint16_t w, h;
            display.getTextBounds(subMenuItems[i], 0, 0, &x1, &y1, &w, &h); 
            int textX = x + (buttonWidth - w) / 2;
            int textY = y + (buttonHeight - h) / 2;

            display.setCursor(textX, textY);
            display.print(subMenuItems[i]);
        }
    }

    display.display();
}

void drawStopwatch() {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);

    unsigned long elapsed;
    if (stopwatchRunning) elapsed = millis() - _stopwatchBegin;
    else elapsed = stopwatchPausedMillis;

    unsigned long minutes = (elapsed / 1000) / 60;
    unsigned long seconds = (elapsed / 1000) % 60;
    unsigned long millisecs = (elapsed % 1000) / 10;

    display.setCursor(10, 20);
    display.printf("%02lu:%02lu:%02lu", minutes, seconds, millisecs);

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("STOPWATCH");

    if (showSubMenu) {
        int y = 52;                 
        int buttonWidth = 60;          
        int buttonHeight = 14;         

        for (int i = 0; i < subMenuLength; i++) {
            int x = i * buttonWidth; 

            if (i == subMenuIndex) {
                display.fillRect(x, y, buttonWidth, buttonHeight, SSD1306_WHITE);
                display.setTextColor(SSD1306_BLACK);
            } else {
                display.drawRect(x, y, buttonWidth, buttonHeight, SSD1306_WHITE);
                display.setTextColor(SSD1306_WHITE);
            }

            int16_t x1, y1;
            uint16_t w, h;
            display.getTextBounds(subMenuItems[i], 0, 0, &x1, &y1, &w, &h); 
            int textX = x + (buttonWidth - w) / 2;
            int textY = y + (buttonHeight - h) / 2;

            display.setCursor(textX, textY);
            display.print(subMenuItems[i]);
        }
    }

    display.display();
}

void drawPomodoro() {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);

    unsigned long remaining = getRemainingPomodoro();
    int minutes = remaining / 60;
    int seconds = remaining % 60;

    display.setCursor(10, 20);
    display.printf("%02d:%02d", minutes, seconds);

    display.setTextSize(1);
    display.setCursor(0, 0);
    if (pomodoroPhase == POMODORO_WORK)
        display.print("POMODORO - WORK");
    else
        display.print("POMODORO - BREAK");

    display.setCursor(0, 50);
    if (pomodoroRunning) display.print("Running...");
    else if (pomodoroFinished) display.print("Finished!");

    if (showSubMenu) {
        int y = 52;                 
        int buttonWidth = 60;          
        int buttonHeight = 14;         

        for (int i = 0; i < subMenuLength; i++) {
            int x = i * buttonWidth; 

            if (i == subMenuIndex) {
                display.fillRect(x, y, buttonWidth, buttonHeight, SSD1306_WHITE);
                display.setTextColor(SSD1306_BLACK);
            } else {
                display.drawRect(x, y, buttonWidth, buttonHeight, SSD1306_WHITE);
                display.setTextColor(SSD1306_WHITE);
            }

            int16_t x1, y1;
            uint16_t w, h;
            display.getTextBounds(subMenuItems[i], 0, 0, &x1, &y1, &w, &h); 
            int textX = x + (buttonWidth - w) / 2;
            int textY = y + (buttonHeight - h) / 2;

            display.setCursor(textX, textY);
            display.print(subMenuItems[i]);
        }
    }

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
    display.display();
}
