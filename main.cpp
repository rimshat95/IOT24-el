#include <Arduino.h>
#include "LEDControl.h"

// Create an instance of LEDControl
LEDControl ledControl;

void setup() {
  // Initialize serial communication at 9600 baud
  Serial.begin(9600);
  
  // Allow serial to initialize
  delay(100);
  
  // Print startup message
  Serial.println("Starting LED Control System...");
  
  // Initialize the LED control system
  ledControl.begin();
}

void loop() {
  // Update LED states
  ledControl.update();
  
  // Check for serial commands
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    ledControl.handleSerialCommand(command);
  }
  
  // Small delay to prevent CPU hogging
  delay(10);
}
