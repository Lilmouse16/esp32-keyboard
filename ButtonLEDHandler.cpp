#include "ButtonLEDHandler.h"

ButtonLEDHandler::ButtonLEDHandler() 
    : bootButton{BOOT_BUTTON_PIN, HIGH, HIGH, 0}
    , rstButton{RST_BUTTON_PIN, HIGH, HIGH, 0}
    , keyboardActive(false)
    , wifiConnected(false)
    , bluetoothConnected(false)
    , websocketConnected(false) {}

void ButtonLEDHandler::begin() {
    pinMode(bootButton.pin, INPUT_PULLUP);
    pinMode(rstButton.pin, INPUT_PULLUP);
    pinMode(BLUE_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);
    digitalWrite(BLUE_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
}

void ButtonLEDHandler::handleButtonPress(Button& button) {
    int reading = digitalRead(button.pin);
    if (reading != button.lastState) {
        button.lastDebounceTime = millis();
    }

    if ((millis() - button.lastDebounceTime) > DEBOUNCE_DELAY) {
        if (reading != button.currentState) {
            button.currentState = reading;
            if (button.currentState == LOW) {  // Button pressed
                if (&button == &bootButton) {
                    if (!keyboardActive) {
                        keyboardActive = true;
                        indicateSuccess(1);
                    }
                } else if (&button == &rstButton) {
                    keyboardActive = false;
                    wifiConnected = false;
                    bluetoothConnected = false;
                    websocketConnected = false;
                    digitalWrite(BLUE_LED_PIN, LOW);
                    digitalWrite(RED_LED_PIN, LOW);
                }
            }
        }
    }
    button.lastState = reading;
}

void ButtonLEDHandler::update() {
    handleButtonPress(bootButton);
    handleButtonPress(rstButton);
}

bool ButtonLEDHandler::isKeyboardActive() const {
    return keyboardActive;
}

void ButtonLEDHandler::setWiFiConnected(bool connected) {
    wifiConnected = connected;
    if (connected) indicateSuccess(2);
}

void ButtonLEDHandler::setBluetoothConnected(bool connected) {
    bluetoothConnected = connected;
    if (connected) indicateSuccess(3);
}

void ButtonLEDHandler::setWebSocketConnected(bool connected) {
    websocketConnected = connected;
    if (connected) indicateSuccess(4);
}

void ButtonLEDHandler::indicateSuccess(uint8_t pattern) {
    const uint16_t patterns[][3] = {
        {300, 300, 3},  // Startup
        {100, 100, 3},  // WiFi
        {500, 500, 3},  // Bluetooth
        {300, 300, 3}   // WebSocket
    };
    
    for (int i = 0; i < patterns[pattern-1][2]; i++) {
        digitalWrite(BLUE_LED_PIN, HIGH);
        digitalWrite(RED_LED_PIN, HIGH);
        delay(patterns[pattern-1][0]);
        digitalWrite(BLUE_LED_PIN, LOW);
        digitalWrite(RED_LED_PIN, LOW);
        delay(patterns[pattern-1][1]);
    }
}

void ButtonLEDHandler::indicateTypingActivity(bool active) {
    digitalWrite(BLUE_LED_PIN, active);
    digitalWrite(RED_LED_PIN, !active);
}
