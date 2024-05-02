#ifndef LED_H
#define LED_H

#include <Arduino.h>

class LED {
public:
    LED(const String& name, int pin, int blinkInterval);
    void setBlinkInterval(int interval);
    void blink();
    void on();
    void off();

private:
    String name;
    int pin;
    int blinkInterval; // Blink interval in seconds
    unsigned long lastToggleTime;
    bool isOn;
};

#endif // LED_H
