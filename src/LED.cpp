#include "LED.h"

LED::LED(const String& name, int pin, int blinkInterval)
: name(name), pin(pin), blinkInterval(blinkInterval), lastToggleTime(0), isOn(false) {
    pinMode(pin, OUTPUT);  // Set the pin as output
    digitalWrite(pin, LOW); // Start with the LED off
}

void LED::setBlinkInterval(int milliseconds) {
    blinkInterval = milliseconds;
}

void LED::blink() {
    unsigned long currentTime = millis();
    if ((currentTime - lastToggleTime) >= blinkInterval) {
        if (isOn) {
            digitalWrite(pin, LOW);
            isOn = false;
        } else {
            digitalWrite(pin, HIGH);
            isOn = true;
        }
        lastToggleTime = currentTime; // Update the last toggle time
    }
}

void LED::on() {
    digitalWrite(pin, HIGH);
    isOn = true;
}

void LED::off() {
    digitalWrite(pin, LOW);
    isOn = false;
}

