#include <WiFi.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <BleKeyboard.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "WiFiCredentials.h"
#include "ButtonLEDHandler.h"

BleKeyboard bleKeyboard(keyboardName);
WebSocketsServer webSocket(81);
WebServer server(80);
ButtonLEDHandler buttonLED;

// Control variables
bool sending = false;
String cachedText = "";
unsigned long sendInterval = 1000;
unsigned long lastSendTime = 0;
unsigned long typingDuration = 0;
unsigned long startTime = 0;

String fetchGoogleDoc(const String& url, const String& token) {
    HTTPClient http;
    String docId = url.substring(url.indexOf("/d/") + 3);
    docId = docId.substring(0, docId.indexOf("/"));
    
    String exportUrl = "https://docs.google.com/document/d/" + docId + "/export?format=txt";
    http.begin(exportUrl);
    http.addHeader("Authorization", "Bearer " + token);
    
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        return http.getString();
    }
    return "";
}

void handleRoot() {
    File file = SPIFFS.open("/index.html", "r");
    if (!file) {
        server.send(404, "text/plain", "File not found");
        return;
    }
    server.streamFile(file, "text/html");
    file.close();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    if (type == WStype_TEXT) {
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);

        String command = doc["command"];
        if (command == "start") {
            String url = doc["url"];
            String token = doc["token"];
            int rate = doc["rate"];
            typingDuration = doc["duration"].as<unsigned long>() * 1000;
            sendInterval = 1000 / rate;
            
            cachedText = fetchGoogleDoc(url, token);
            if (!cachedText.isEmpty()) {
                sending = true;
                startTime = millis();
                buttonLED.indicateTypingActivity(true);
            }
        } else if (command == "stop") {
            sending = false;
            buttonLED.indicateTypingActivity(false);
        }
    }
}

void setup() {
    Serial.begin(115200);
    buttonLED.begin();

    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    buttonLED.setWiFiConnected(true);

    // Initialize BLE Keyboard
    bleKeyboard.begin();
    buttonLED.setBluetoothConnected(true);

    // Start servers
    server.on("/", handleRoot);
    server.begin();
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    buttonLED.setWebSocketConnected(true);
}

void loop() {
    buttonLED.update();
    server.handleClient();
    webSocket.loop();

    if (sending && bleKeyboard.isConnected() && buttonLED.isKeyboardActive()) {
        // Check duration limit
        if (typingDuration > 0 && (millis() - startTime) > typingDuration) {
            sending = false;
            buttonLED.indicateTypingActivity(false);
            return;
        }

        unsigned long currentTime = millis();
        if (currentTime - lastSendTime >= sendInterval) {
            static size_t position = 0;
            
            if (position < cachedText.length()) {
                // Send word by word
                int nextSpace = cachedText.indexOf(' ', position);
                if (nextSpace == -1) nextSpace = cachedText.length();
                
                String word = cachedText.substring(position, nextSpace + 1);
                bleKeyboard.print(word);
                position = nextSpace + 1;
            } else {
                sending = false;
                buttonLED.indicateTypingActivity(false);
                position = 0;  // Reset for next time
            }
            
            lastSendTime = currentTime;
        }
    }
}

