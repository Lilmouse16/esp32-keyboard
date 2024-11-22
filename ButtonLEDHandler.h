#ifndef BUTTONLEDHANDLER_H
#define BUTTONLEDHANDLER_H

#include <Arduino.h>

// Pin definitions
#define BOOT_BUTTON_PIN 0
#define RST_BUTTON_PIN  22
#define BLUE_LED_PIN    2
#define RED_LED_PIN     4

// Debounce configuration 
#define DEBOUNCE_DELAY  50

class ButtonLEDHandler {
public:
    ButtonLEDHandler();
    void begin();
    void update();
    bool isKeyboardActive() const;
    void setWiFiConnected(bool connected);
    void setBluetoothConnected(bool connected);
    void setWebSocketConnected(bool connected);
    void indicateTypingActivity(bool active);

private:
    struct Button {
        uint8_t pin;
        bool lastState;
        bool currentState;
        unsigned long lastDebounceTime;
    };

    Button bootButton;
    Button rstButton;
    bool keyboardActive;
    bool wifiConnected;
    bool bluetoothConnected;
    bool websocketConnected;

    void handleButtonPress(Button& button);
    void indicateSuccess(uint8_t pattern);
};

#endif // BUTTONLEDHANDLER_H
