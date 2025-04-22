#include "LEDControl.h"
#include "Arduino.h"  // Required for PlatformIO-style separation

// Initialize the static instance pointer
LEDControl* LEDControl::instance = nullptr;

// Static ISR function that will be called by the interrupt
void LEDControl::resetButtonISR() {
    if (instance != nullptr) {
        instance->resetSystem();
    }
}


LEDControl::LEDControl() {
    // Initialize default states
    blinkState = false;
    for (int i = 0; i < 3; i++) {
        ledOverride[i] = false;
        ledState[i] = false;
        buttonsEnabled[i] = true;
    }
    usePotForBlue = true;
    blueLedPower = 255;
    previousMillis = 0;
}

void LEDControl::begin() {
    // Set pin modes
    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(BLUE_LED, OUTPUT);
    pinMode(RGB_R, OUTPUT);
    pinMode(RGB_G, OUTPUT);
    pinMode(RGB_B, OUTPUT);

    pinMode(RED_BUTTON, INPUT_PULLUP);
    pinMode(GREEN_BUTTON, INPUT_PULLUP);
    pinMode(BLUE_BUTTON, INPUT_PULLUP);
    pinMode(RESET_BUTTON, INPUT_PULLUP);

    instance = this;

    // Direct pin testing
    /*Serial.println("Testing individual LEDs:");
    
    Serial.println("Testing RED LED");
    digitalWrite(RED_LED, HIGH);
    delay(500);
    digitalWrite(RED_LED, LOW);
    
    Serial.println("Testing GREEN LED");
    digitalWrite(GREEN_LED, HIGH);
    delay(500);
    digitalWrite(GREEN_LED, LOW);*/
    
    /*Serial.println("Testing BLUE LED");
    digitalWrite(BLUE_LED, HIGH);
    delay(500);
    digitalWrite(BLUE_LED, LOW);*/
    Serial.println("Testing blue LED directly...");
    for (int i = 0; i < 3; i++) {
        digitalWrite(BLUE_LED, HIGH);
        delay(200);
        digitalWrite(BLUE_LED, LOW);
        delay(200);
    }

    // Attach interrupt for reset button
    attachInterrupt(digitalPinToInterrupt(RESET_BUTTON), resetButtonISR, FALLING);

    Serial.println("System ready. Type 'help' for commands.");
}

void LEDControl::update() {
    checkButtons();
    updateBlink();
    updateRGB();
}

void LEDControl::checkButtons() {
    // Array of button pins for easier iteration
    const int buttonPins[3] = {RED_BUTTON, GREEN_BUTTON, BLUE_BUTTON};
    
    // Check each button
    for (int i = 0; i < 3; i++) {
        // Skip if button is disabled
        if (!buttonsEnabled[i]) continue;
        
        // Read the current button state
        int reading = digitalRead(buttonPins[i]);
        
        // Check if the button state changed
        if (reading != lastButtonState[i]) {
            // Reset the debounce timer
            lastDebounceTime[i] = millis();
            
            // Debug message
            Serial.print("Button ");
            Serial.print(i);
            Serial.print(" (pin ");
            Serial.print(buttonPins[i]);
            Serial.println(") state changed");
        }
        
        // If enough time has passed since the last state change
        if ((millis() - lastDebounceTime[i]) > debounceDelay) {
            // If the button state has changed
            if (reading != buttonState[i]) {
                buttonState[i] = reading;
                
                // If the button is pressed (LOW due to INPUT_PULLUP)
                if (buttonState[i] == LOW) {
                    Serial.print("Button ");
                    Serial.print(i);
                    Serial.println(" pressed");
                    
                    // Toggle LED override
                    ledOverride[i] = true;
                    
                    // Toggle LED state
                    ledState[i] = !ledState[i];
                    
                    Serial.print("LED ");
                    Serial.print(i);
                    Serial.print(" state: ");
                    Serial.println(ledState[i] ? "ON" : "OFF");
                    
                    // Update the LED state
                    if (i == 0) {
                        digitalWrite(RED_LED, ledState[i]);
                    } else if (i == 1) {
                        digitalWrite(GREEN_LED, ledState[i]);
                    } else if (i == 2) {
                        if (ledState[i]) {
                            int brightness = usePotForBlue ? 
                                map(analogRead(BLUE_POT), 0, 1023, 0, 255) : blueLedPower;
                            analogWrite(BLUE_LED, brightness);
                        } else {
                            analogWrite(BLUE_LED, 0);
                        }
                    }
                }
            }
        }
        
        // Save the current reading for the next iteration
        lastButtonState[i] = reading;
    }
}



void LEDControl::updateBlink() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= blinkInterval) {
        previousMillis = currentMillis;
        blinkState = !blinkState;
        
        Serial.print("Blink state: ");
        Serial.println(blinkState ? "ON" : "OFF");
        Serial.print("Blue LED override: ");
        Serial.println(ledOverride[2] ? "YES" : "NO");

        // Red LED
        if (!ledOverride[0]) {
            digitalWrite(RED_LED, blinkState);
        }
        
        // Green LED
        if (!ledOverride[1]) {
            digitalWrite(GREEN_LED, blinkState);
        }
        
        // Blue LED
        if (!ledOverride[2]) {
            Serial.println("Blue LED should be blinking");
            if (blinkState) {
                Serial.println("Blue LED should be ON");
                // Use digitalWrite for testing
                digitalWrite(BLUE_LED, HIGH);
            } else {
                Serial.println("Blue LED should be OFF");
                digitalWrite(BLUE_LED, LOW);
            }
        }
    }
}





void LEDControl::updateRGB() {
    // Read potentiometers and set RGB LED
    analogWrite(RGB_R, map(analogRead(RGB_POT_R), 0, 1023, 0, 255));
    analogWrite(RGB_G, map(analogRead(RGB_POT_G), 0, 1023, 0, 255));
    analogWrite(RGB_B, map(analogRead(RGB_POT_B), 0, 1023, 0, 255));
}

void LEDControl::resetSystem() {
    // Reset all states
    for (int i = 0; i < 3; i++) {
        ledOverride[i] = false;
        ledState[i] = false;
        buttonsEnabled[i] = true;
    }
    usePotForBlue = true;
    blueLedPower = 255;
    previousMillis = millis();

    // Turn off all LEDs
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    analogWrite(BLUE_LED, 0);
    analogWrite(RGB_R, 0);
    analogWrite(RGB_G, 0);
    analogWrite(RGB_B, 0);

    Serial.println("System reset to default state.");
}

void LEDControl::handleSerialCommand(const String &command) {
    String cmd = command;
    cmd.trim();
    cmd.toLowerCase();

    if (cmd.startsWith("disable button ")) {
        int btn = cmd.substring(14).toInt();
        if (btn >= 1 && btn <= 3) {
            buttonsEnabled[btn-1] = false;
            Serial.print("Button ");
            Serial.print(btn);
            Serial.println(" disabled");
        } else {
            Serial.println("Invalid button number. Use 1-3");
        }
    }
    else if (cmd.startsWith("enable button ")) {
        int btn = cmd.substring(13).toInt();
        if (btn >= 1 && btn <= 3) {
            buttonsEnabled[btn-1] = true;
            Serial.print("Button ");
            Serial.print(btn);
            Serial.println(" enabled");
        } else {
            Serial.println("Invalid button number. Use 1-3");
        }
    }
    else if (cmd == "ledon 1") {
        ledOverride[0] = true;
        ledState[0] = true;
        digitalWrite(RED_LED, HIGH);
        Serial.println("Red LED turned ON");
    }
    else if (cmd == "ledon 2") {
        ledOverride[1] = true;
        ledState[1] = true;
        digitalWrite(GREEN_LED, HIGH);
        Serial.println("Green LED turned ON");
    }
    else if (cmd == "ledon 3") {
        ledOverride[2] = true;
        ledState[2] = true;
        int brightness = usePotForBlue ? 
            map(analogRead(BLUE_POT), 0, 1023, 0, 255) : blueLedPower;
        analogWrite(BLUE_LED, brightness);
        Serial.println("Blue LED turned ON");
    }
    else if (cmd == "ledoff 1") {
        ledOverride[0] = true;
        ledState[0] = false;
        digitalWrite(RED_LED, LOW);
        Serial.println("Red LED turned OFF");
    }
    else if (cmd == "ledoff 2") {
        ledOverride[1] = true;
        ledState[1] = false;
        digitalWrite(GREEN_LED, LOW);
        Serial.println("Green LED turned OFF");
    }
    else if (cmd == "ledoff 3") {
        ledOverride[2] = true;
        ledState[2] = false;
        analogWrite(BLUE_LED, 0);
        Serial.println("Blue LED turned OFF");
    }
    else if (cmd.startsWith("ledpower ")) {
        int power = cmd.substring(9).toInt();
        if (power >= 0 && power <= 10) {
            blueLedPower = map(power, 0, 10, 0, 255);
            usePotForBlue = false;
            
            if (ledState[2] && ledOverride[2]) {
                analogWrite(BLUE_LED, blueLedPower);
            }
            
            Serial.print("Blue LED power set to ");
            Serial.println(power);
        } else if (power == -1) {
            usePotForBlue = true;
            Serial.println("Blue LED now controlled by potentiometer");
            
            if (ledState[2] && ledOverride[2]) {
                int brightness = map(analogRead(BLUE_POT), 0, 1023, 0, 255);
                analogWrite(BLUE_LED, brightness);
            }
        } else {
            Serial.println("Invalid power level. Use 0-10 or -1");
        }
    }
    else if (cmd == "reset") {
        resetSystem();
    }
    else if (cmd == "status") {
        printStatus();
    }
    else if (cmd == "help") {
        printHelp();
    }
    else {
        Serial.println("Unknown command. Type 'help' for options.");
    }
}


void LEDControl::printStatus() {
    Serial.println("=== System Status ===");
    Serial.print("Red LED: "); Serial.println(ledState[0] ? "ON" : "OFF");
    Serial.print("Green LED: "); Serial.println(ledState[1] ? "ON" : "OFF");
    Serial.print("Blue LED: "); Serial.print(ledState[2] ? "ON" : "OFF");
    Serial.println(usePotForBlue ? " (Potentiometer-controlled)" : " (Manual power)");
}

void LEDControl::printHelp() {
    Serial.println("Available commands:");
    Serial.println("  enable button [1-3]");
    Serial.println("  disable button [1-3]");
    Serial.println("  ledon [1-3]");
    Serial.println("  ledoff [1-3]");
    Serial.println("  ledpower [0-10]");
    Serial.println("  status");
    Serial.println("  reset");
}