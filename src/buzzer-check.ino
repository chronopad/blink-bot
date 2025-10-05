#define BUZZER_PIN 32 

void setup() {
    pinMode(BUZZER_PIN, OUTPUT);
    Serial.begin(115200);
    Serial.println("Buzzer test starting...");

    for (int i = 0; i < 5; i++) {
        Serial.print("Beep ");
        Serial.println(i + 1);
        tone(BUZZER_PIN, 1000); 
        delay(200);
        noTone(BUZZER_PIN);
        delay(300);
    }

    Serial.println("Test complete!");
}

void loop() {

}