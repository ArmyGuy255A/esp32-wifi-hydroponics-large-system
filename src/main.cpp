#include <WiFi.h>
#include <Preferences.h>
#include <WebServer.h>

bool vextOn = false;

Preferences preferences;
WebServer server(80);

// Pins
#define LED_PIN 2
#define VEXT_PIN 18

// WiFi credentials
const char* ssid = "...";
const char* password = "...";

// Configurable times in seconds
int relayOnTimeSec;
int deepSleepTimeSec;

// Activation Information
unsigned long lastActivationTime = 0;
unsigned long lastDeactivationTime = 0;

void connectToWifi() {
  WiFi.begin(ssid, password);

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

void VextON() {
  Serial.println("Vext ON");
  digitalWrite(VEXT_PIN, HIGH);
  vextOn = true;
  lastActivationTime = millis();
  // lastDeactivationTime = 0; // Reset the deactivation timer
}

void VextOFF() {
  Serial.println("Vext OFF");
  digitalWrite(VEXT_PIN, LOW);
  vextOn = false;
  lastDeactivationTime = millis();
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
  } else if (type == "setDeepSleepTime") {
    setPreferenceTime("deepSleepTime", value.toInt());
  }

  server.send(200, "text/plain", "OK - Changes will take effect after next reboot or sleep cycle");
}

void getPrefereceHandler() {
  // Print a diagnostic message
  Serial.println("GET Request Received");

  if (server.uri() == "/getRelayOnTime") {
    server.send(200, "text/plain", String(relayOnTimeSec + " seconds"));
  } else if (server.uri() == "/getDeepSleepTime") {
    server.send(200, "text/plain", String(deepSleepTimeSec + " seconds"));
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
  server.on("/setDeepSleepTime", HTTP_POST, setPreferenceHandler);
  server.on("/getDeepSleepTime", HTTP_GET, getPrefereceHandler);


  server.begin();
}

void setup() {

  Serial.begin(115200); 
  Serial.println("System Start");

  // Initialize Preferences
  preferences.begin("app-settings", false); // Open Preferences with namespace. RW-mode
  // Attempt to load saved settings
  relayOnTimeSec = preferences.getInt("relayOnTime", 60); // Default to 3 seconds if not set
  deepSleepTimeSec = preferences.getInt("deepSleepTime", 5); // Default to 3 seconds if not set

  // Print the saved settings
  Serial.print("Relay On Time: ");
  Serial.println(relayOnTimeSec);
  Serial.print("Deep Sleep Time: ");
  Serial.println(deepSleepTimeSec);

  // Initialize Buttons
  pinMode(LED_PIN, OUTPUT);
  pinMode(VEXT_PIN, OUTPUT);

  // Connect to WiFi & set time
  connectToWifi();
  configTime(-8, 0, "pool.ntp.org");
  printTime();

  // Setup the Web Server so we can access it during runtime
  setupWebServer();
   
  Serial.println("Waking Up");



}


bool relayActivated = false;
unsigned long relayStartTime = 0;
unsigned long nextLEDToggleTime = 0;
int ledBlinkInterval = 1000;

void loop() {

  // Need this to handle client requests on the REST server
  server.handleClient();

  // Add logic to turn on the relay based on the relayOnTimeSec variable
  if (!relayActivated) {
    Serial.println("Relay is ON, LED should be blinking");
    VextON(); // Activate the relay
    digitalWrite(LED_PIN, HIGH); // Initially turn on the LED
    relayActivated = true;
    relayStartTime = millis();
    nextLEDToggleTime = millis() + ledBlinkInterval;
    digitalWrite(LED_PIN, HIGH);
  }

  // Handle LED blinking every 500ms while the relay is on
  if (relayActivated && millis() - relayStartTime <= relayOnTimeSec * 1000) {
    if (millis() >= nextLEDToggleTime) {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Toggle LED state
      nextLEDToggleTime += ledBlinkInterval; // Set the next toggle time to 1 second later
    }
  } else if (relayActivated) {
    VextOFF(); // Turn off the relay
    digitalWrite(LED_PIN, LOW); // Ensure the LED is off
    relayActivated = false; // Reset the relay activated flag
    Serial.println("Going to Sleep");
    ESP.deepSleep(deepSleepTimeSec * 1000000); // Enter deep sleep
  }

}
