#ifndef RELAY_H
#define RELAY_H

#include <Arduino.h>

class Relay {
public:
    Relay(String relayName, int togglePin, int onIntervalInSeconds, int offIntervalInSeconds);
    bool is_on;

    unsigned long last_on_time;
    unsigned long last_off_time;

    bool shouldActivate();
    String activate();
    int getRemainingActivationTime();
    void on();
    void off();

    // Getters and setters for intervals
    int getOnInterval() const;
    void setOnInterval(int seconds);
    int getOffInterval() const;
    void setOffInterval(int seconds);

private:
    int toggle_pin;
    int on_interval;
    int off_interval;
    String name;
};

#endif // RELAY_H
