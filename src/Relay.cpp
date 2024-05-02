#include "Relay.h"

Relay::Relay(String relayName, int togglePin, int onInterval, int offInterval)
: name(relayName), toggle_pin(togglePin), on_interval(onInterval), off_interval(offInterval), is_on(false), last_on_time(0), last_off_time(0) {
    pinMode(toggle_pin, OUTPUT);
    off(); // Initialize relay to off state
}

int Relay::getRemainingActivationTime() {
    unsigned long current_time = millis();
    if (is_on) {
        return (on_interval - (current_time - last_on_time));
    } else {
        return (off_interval - (current_time - last_off_time));
    }
}

bool Relay::shouldActivate() {
    unsigned long current_time = millis();
    if (is_on && (current_time - last_on_time >= on_interval)) {
        return true;
    } else if (!is_on && (current_time - last_off_time >= off_interval)) {
        return true;
    } else {
        return false;
    }
}

String Relay::activate() {
    if (is_on) {
        off();
        return (name + " Off");
    } else {
        on();
        return (name + " On");
    }
}

void Relay::on() {
    digitalWrite(toggle_pin, HIGH); // Assuming HIGH is the ON state
    is_on = true;
    last_on_time = millis();
}

void Relay::off() {
    digitalWrite(toggle_pin, LOW); // Assuming LOW is the OFF state
    is_on = false;
    last_off_time = millis();
}

// Getter and Setter Implementations
int Relay::getOnInterval() const {
    return on_interval;
}

void Relay::setOnInterval(int seconds) {
    on_interval = seconds * 1000;
}

int Relay::getOffInterval() const {
    return off_interval;
}

void Relay::setOffInterval(int seconds) {
    off_interval = seconds * 1000;
}
