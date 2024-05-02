#include <Preferences.h>
#include <WebServer.h>
// #include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
// #include "SSD1306Wire.h"
// #include "HT_DisplayUI.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <credentials.h>
#include <PriorityQueue.h>
#include <Relay.h>
#include <LED.h>

// Pins
#define LED_PIN 2
#define VEXT_PIN 18
#define SDA_PIN 17
#define SCL_PIN 21

static const uint8_t SDA_OLED = 4;
static const uint8_t SCL_OLED = 15;
static const uint8_t RST_OLED = 16;


// OLED display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3c ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

TwoWire customWire = TwoWire(0);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &customWire, OLED_RESET);

Preferences preferences;
WebServer server(80);

// Priority Queue for scrolling messages
PriorityQueue pq;

// Add a relay
Relay relay("Pump", VEXT_PIN, 15000, 5000);

// Add an LED
LED led("LED", LED_PIN, 500);

void connectToWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PW);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

String millisToReadableTime(unsigned long milliseconds) {
  unsigned long seconds = milliseconds / 1000;
  unsigned long minutes = seconds / 60;
  seconds = seconds % 60;

  String readableTime = String(minutes) + " min " + String(seconds) + " sec";
  return readableTime;
}

String millisToSeconds(unsigned long milliseconds) {
  unsigned long seconds = milliseconds / 1000;
  return String(seconds) + " sec";
}

void printTime() {
  // Get current date and time
  time_t now = time(nullptr);
  struct tm *timeinfo;
  timeinfo = localtime(&now);

  // Print current date and time
  Serial.print("Current date and time: ");
  Serial.print(asctime(timeinfo));
}

void setPreferenceTime(const char* key, int time) {
  preferences.putInt(key, time);
  preferences.end();
}

int getPreferenceTime(const char* key) {
  return preferences.getInt(key);
}

void setPreferenceHandler() {
  // Print a diagnostic message
  Serial.println("POST Request Received");

  // Print the URI of the request
  Serial.print("URI: ");
  Serial.println(server.uri());

  // Print all of the arguments  
  for (int i = 0; i < server.args(); i++) {
    Serial.print(server.argName(i));
    Serial.print(": ");
    Serial.println(server.arg(i));
  }

  // Extract the type of preference to set
  String type = server.uri().substring(1);
  // Extract the value to set
  String value = server.arg("value");

  // Check if type or value have been provided
  if (type == "" || value == "") {
    server.send(400, "text/plain", "Bad Request: Missing type or value argument");
    return;
  }
  
  // Set the preference based on the type
  if (type == "setRelayOnTime") {
    setPreferenceTime("relayOnTime", value.toInt());
    relay.setOnInterval(value.toInt());
  } else if (type == "setRelayOffTime") {
    setPreferenceTime("relayOffTime", value.toInt());
    relay.setOffInterval(value.toInt());
  }
  
  server.send(200, "text/plain", "OK - Changes will take effect after next reboot or sleep cycle");
}

void getPrefereceHandler() {
  // Print a diagnostic message
  Serial.println("GET Request Received");

  if (server.uri() == "/getRelayOnTime") {
    server.send(200, "text/plain", String(relay.getOnInterval() + " seconds"));
  } else if (server.uri() == "/getrelayOffTime") {
    server.send(200, "text/plain", String(relay.getOffInterval() + " seconds"));
  } else {
    server.send(400, "text/plain", "Bad Request: Missing value argument");
  }
}

void setupWebServer() {
  // Define routes
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/plain", "ESP32 Web Server");
  });


  server.on("/setRelayOnTime", HTTP_POST, setPreferenceHandler);
  server.on("/getRelayOnTime", HTTP_GET, getPrefereceHandler);
  server.on("/setrelayOffTime", HTTP_POST, setPreferenceHandler);
  server.on("/getrelayOffTime", HTTP_GET, getPrefereceHandler);


  server.begin();
}

void updateProgressBar(int progress, String currentStepText) {
  display.clearDisplay(); // Clear the display

  display.setTextSize(1); // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0); // Start at top-left corner
  display.println(currentStepText); // Display the current step text
  
  // Draw the progress bar background
  int progressBarWidth = 120;
  int progressBarHeight = 10;
  int progressBarX = (SCREEN_WIDTH - progressBarWidth) / 2; // Center the progress bar
  int progressBarY = 20; // Position on the Y axis
  display.drawRect(progressBarX, progressBarY, progressBarWidth, progressBarHeight, WHITE);

  // Calculate the width of the filled part
  int fillWidth = (progress * progressBarWidth) / 100;
  display.fillRect(progressBarX, progressBarY, fillWidth, progressBarHeight, WHITE);

  display.display(); // Update the display with the new graphics
  delay(250); // Delay for visibility
}



void updateDisplay(String message, String titles[], String texts[]) {
  display.clearDisplay();
  display.setTextSize(2); // Larger for scrolling text
  display.setTextColor(WHITE); // Draw white text
  display.setTextWrap(false); // Turn off text wrapping
  display.setFont(); // Set font size (pixel height

  // Scrolling Text
  static int x = SCREEN_WIDTH;
  // String message = "scrolling text ...scrolling text ...scrolling text ...";

  // Calculate width of the text to reset x correctly
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(message, 0, 0, &x1, &y1, &w, &h); // Get bounds of the entire string

  display.setCursor(x, 0); // Set cursor to start position
  display.print(message);
  x -= 1; // Move text left by 2 pixels

  if (x < -w) {
    x = SCREEN_WIDTH; // Reset x after text has scrolled off
    pq.pop(); // Remove the message from the queue
  }

  // Grid Dimensions
  int cellWidth = SCREEN_WIDTH / 2;
  // int y_offset = h + 7; // Start y position right below the scrolling text
  int y_offset = h; // Start y position right below the scrolling text
  int cellHeight = (SCREEN_HEIGHT - y_offset) / 2; // Adjusted for the scrolling text height

  // String titles[4] = {"TITLE 1", "TITLE 2", "TITLE 3", "TITLE 4"};
  // String texts[4] = {"TEXT 1", "TEXT 2", "TEXT 3", "TEXT 4"};
  display.setTextSize(1); // Smaller text for grid
  for(int i = 0; i < 4; i++) {
    int col = i % 2;
    int row = i / 2;
    int cellX = col * cellWidth;
    int cellY = y_offset + row * cellHeight;

    // Draw cell border
    display.drawRect(cellX, cellY, cellWidth, cellHeight, WHITE);

    // Calculate text positioning for centering
    display.getTextBounds(titles[i], 0, 0, &x1, &y1, &w, &h);
    display.setCursor(cellX + (cellWidth - w) / 2, cellY + (cellHeight - h) / 2 - 6);
    display.println(titles[i]);

    display.getTextBounds(texts[i], 0, 0, &x1, &y1, &w, &h);
    display.setCursor(cellX + (cellWidth - w) / 2, cellY + (cellHeight + h) / 2 - 2);
    display.println(texts[i]);
  }

  display.display();
}

void setup() {

  // display.init();
  // display.setFont(ArialMT_Plain_10);
  // display.setTextAlignment(TEXT_ALIGN_LEFT);
  // display.flipScreenVertically();

  customWire.begin(SDA_PIN, SCL_PIN); // Initialize the customWire object with your specified pins

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Check your display's I2C address
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.display();
  delay(2000); // Pause for 2 seconds
  display.clearDisplay();

  Serial.begin(115200); 
  Serial.println("System Start");

  // Initialize Preferences
  preferences.begin("app-settings", false); // Open Preferences with namespace. RW-mode
  // Override Preferences for testing
  // setPreferenceTime("relayOnTime", 15);
  // setPreferenceTime("relayOffTimeSec", 5);

  // Attempt to load saved settings
  relay.setOnInterval(preferences.getInt("relayOnTime", 5)); // Default to 5 seconds if not set
  relay.setOffInterval(preferences.getInt("relayOffTime", 5)); // Default to 5 seconds if not set



  // Print the saved settings
  Serial.print("Relay On Time: ");
  Serial.println(relay.getOnInterval());
  Serial.print("Deep Sleep Time: ");
  Serial.println(relay.getOffInterval());

  // Initialize Buttons
  pinMode(LED_PIN, OUTPUT);
  pinMode(VEXT_PIN, OUTPUT);

  updateProgressBar(25, "Initializing...");
  updateProgressBar(50, "Connecting to WiFi...");
  // Connect to WiFi & set time
  connectToWifi();

  updateProgressBar(75, "NTP Sync...");
  configTime(-8, 0, "pool.ntp.org");
  printTime();

  // Setup the Web Server so we can access it during runtime
  updateProgressBar(100, "REST Server...");
  setupWebServer();
   
  Serial.println("Waking Up");
}



void loop() {

  // Need this to handle client requests on the REST server
  server.handleClient();

  // Add logic to turn on the relay based on the relayOnTimeSec variable
  if (relay.shouldActivate()) {
    String result = relay.activate();
    pq.push(result);
  }

  // Handle the LED blinking every 500ms while the relay is on
  if (relay.is_on) {
    led.blink();
    // Set the next toggle time to 1 second later
    pq.push("Pumping Water");
    // Serial.println("Water Pump is ON, LED should be blinking");
    // Serial.print("Time Remaining: ");
    // Serial.println(millisToReadableTime((relayOnTimeSec * 1000) - (millis() - relayStartTime)));
  }

  // if (!relayActivated) {
  //   Serial.println("Relay is ON, LED should be blinking");
  //   VextON(); // Activate the relay
  //   digitalWrite(LED_PIN, HIGH); // Initially turn on the LED
  //   relayActivated = true;
    
  //   relayStartTime = millis();
  //   nextLEDToggleTime = millis() + ledBlinkInterval;
  //   digitalWrite(LED_PIN, HIGH);
  // }

  // // Check if the relay (pump) should be turned on or off.
  // if (!vextOn && (millis() - lastDeactivationTime) >= (triggerTimeValue)) {

  // }

  // // Handle LED blinking every 500ms while the relay is on
  // if (relayActivated && millis() - relayStartTime <= relayOnTimeSec * 1000) {
  //   if (millis() >= nextLEDToggleTime) {
  //     digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Toggle LED state
  //     nextLEDToggleTime += ledBlinkInterval; // Set the next toggle time to 1 second later
  //     pq.push("Pumping Water");
  //     Serial.println("Water Pump is ON, LED should be blinking");
  //     Serial.print("Time Remaining: ");
  //     Serial.println(millisToReadableTime((relayOnTimeSec * 1000) - (millis() - relayStartTime)));
  //   }
  // } else if (relayActivated) {
  //   VextOFF(); // Turn off the relay
  //   digitalWrite(LED_PIN, LOW); // Ensure the LED is off
  //   relayActivated = false; // Reset the relay activated flag
  //   pq.push("Pump Off");
  //   Serial.println("Going to Sleep");
  //   // sleep(relayOffTimeSecSec);
  //   // ESP.deepSleep(relayOffTimeSecSec * 1000000); // Enter deep sleep
  // }


  //   pq.push("Hello");
  // pq.push("World");

  // Titles and values for each of the four fields
  String field1_title = "Voltage";
  String field1_value = "25°C";

  String field2_title = "";
  if (relay.is_on) {
    field2_title = "P. Off In";
  } else {
    field2_title = "P. On In";
  }
  String field2_value = millisToSeconds(relay.getRemainingActivationTime());
  
  String field3_title = "ECC";
  String field3_value = "...";
  String field4_title = "pH";
  String field4_value = "...";

  // String message = pq.top();
  String titles[4] = {field1_title, field2_title, field3_title, field4_title};
  String texts[4] = {field1_value, field2_value, field3_value, field4_value};

  
  updateDisplay(pq.top(), titles, texts);

}
