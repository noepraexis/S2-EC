#include <Arduino.h>
#include <HardwareSerial.h>

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }
    Serial.println("Hello, World!");
}

void loop() {
    Serial.println("Looping...");
    delay(1000);
}
