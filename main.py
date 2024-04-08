from machine import Pin, deepsleep
import time

# Relay control GPIO pin (update based on your wiring)
relayPin = Pin(4, Pin.OUT) # Use GPIO 4 (D2 on some ESP8266 boards)

def triggerRelay():
    """Triggers the relay for 10 minutes then goes to deep sleep for 50 minutes."""
    relayPin.value(1)  # Turn on the relay
    # time.sleep(600000)  # Relay on for 10 minutes (600 seconds or 600,000 milliseconds)
    print('Relay on for 10 minutes')
    time.sleep(6000)  # Relay on for 10 minutes (600 seconds or 600,000 milliseconds)
    relayPin.value(0)  # Turn off the relay
    print('Relay off, Deep sleep for 50 minutes')
    # Go to deep sleep for 50 minutes (3000 seconds or 3,000,000 milliseconds)
    # The ESP8266 will wake up after this period, effectively resetting the script
    deepsleep(3000)
    # deepsleep(3000000)

triggerRelay()
