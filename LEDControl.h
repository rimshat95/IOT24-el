#ifndef LEDCONTROL_H
#define LEDCONTROL_H

#include "Arduino.h"

// Pin definitions
#define RED_LED 13     // Update with your actual pin numbers
#define GREEN_LED 12
#define BLUE_LED 5
#define RGB_R 10
#define RGB_G 11
#define RGB_B 9

#define RED_BUTTON 8   // Update with your actual pin numbers
#define GREEN_BUTTON 7
#define BLUE_BUTTON 6
#define RESET_BUTTON 2 // This should be an interrupt-capable pin

#define BLUE_POT A0    // Update with your actual pin numbers
#define RGB_POT_R A2
#define RGB_POT_G A3
#define RGB_POT_B A4

class LEDControl {
public:
    LEDControl();  // Add constructor declaration
    void begin();
    void update();
    void handleSerialCommand(const String &command);
    void resetSystem();

private:
    // Member variables
    bool blinkState;
    bool ledOverride[3];
    bool ledState[3];
    bool buttonsEnabled[3];
    bool usePotForBlue;
    int blueLedPower;
    unsigned long previousMillis;
    const unsigned long blinkInterval = 2000; // 2 seconds blink interval

    // Button debounce variables
    unsigned long lastDebounceTime[3] = {0, 0, 0};
    int buttonState[3] = {HIGH, HIGH, HIGH};
    int lastButtonState[3] = {HIGH, HIGH, HIGH};
    const unsigned long debounceDelay = 50;

    // Private methods
    static LEDControl* instance;
    static void resetButtonISR();
    void checkButtons();
    void updateBlink();
    void updateRGB();
    void printStatus();
    void printHelp();
};

#endif