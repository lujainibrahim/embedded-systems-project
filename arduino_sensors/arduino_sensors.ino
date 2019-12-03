#include <Wire.h> // I2C Library
#include <Adafruit_TCS34725.h> // TCS34725 Library
#define ADDR_1 0x10
#define ADDR_2 0x20 // (Me)

Adafruit_TCS34725 tcs_R = Adafruit_TCS34725(); // Colour Sensor (Right)
char command;
int greenBool_L = 0; // Left Sensor
int greenBool_R = 0; // Right Sensor

enum { // Commands from Hub
  COLOUR_L = 1,
  COLOUR_R  = 2
};

void setup() {
  /* Serial */
   Serial.begin(9600);
  /* I2C */
  command = 0;
  Wire.begin(ADDR_2);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  /* Colour Sensors */
  if (tcs_R.begin()) {
    Serial.println("TCS34725 (Right) detected.");
  } else {
    Serial.println("TCS34725 (Right) not detected.");
    while (1);
  }
}

void loop() {
  /* Colour Sensor (Left) */
  
  /* Colour Sensor (Right) */
  uint16_t r, g, b, c;
  float average, red, green, blue;
  tcs_R.getRawData(&r, &g, &b, &c);
  average= (r+g+b)/3;
  red= r/average;
  green= g/average;
  blue= b/average;
  if ((red < 0.95) && (green >= 1.3) && (blue <= 0.95)) {
    greenBool_R = 1;
    Serial.println("Green!");
  } else {
    greenBool_R = 0;
  }
}

void receiveEvent (int howMany) {
  command = Wire.read();
}

void requestEvent() {
  switch (command) {
     case COLOUR_L: Wire.write(greenBool_L); break;   // send our ID 
     case COLOUR_R: Wire.write(greenBool_R); break;  // send A0 value
     }
  }
