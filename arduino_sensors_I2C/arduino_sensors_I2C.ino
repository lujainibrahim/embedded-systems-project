/* D E F I N I T I O N S */

#include <Wire.h> // I2C Library
#include <Adafruit_TCS34725.h> // TCS34725 Library
#define ADDR_1 0x10
#define ADDR_2 0x20 // (Me)
#define TCAADDR 0x70
extern "C" { 
  #include "utility/twi.h"
}

/* I2C */
enum { // Commands from Hub
  COLOR_L = 1,
  COLOR_R  = 2
};

Adafruit_TCS34725 tcs_L = Adafruit_TCS34725(); // Colour Sensor (Left)
Adafruit_TCS34725 tcs_R = Adafruit_TCS34725(); // Colour Sensor (Right)
char command;
char greenBool_L = 0; // Left Sensor
char greenBool_R = 0; // Right Sensor

/* S E T U P */

void setup() {
  /* Serial */
   Serial.begin(9600);
  /* I2C */
  command = 0;
  Wire.begin(ADDR_2);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  /* Colour Sensors */
  tcaselect(2); // Left Sensor
  if (tcs_L.begin()) {
    Serial.println("TCS34725 (Left) detected.");
  } else {
    Serial.println("TCS34725 (Left) not detected.");
    while(1);
  }
  tcaselect(1); // Right Sensor
  if (tcs_R.begin()) {
    Serial.println("TCS34725 (Right) detected.");
  } else {
    Serial.println("TCS34725 (Right) not detected.");
    while(1);
  }
}

void loop() {
  /* Color Sensor (Left) */
  tcaselect(2);
  uint16_t r_L, g_L, b_L, c_L;
  float average_L, red_L, green_L, blue_L;
  tcs_R.getRawData(&r_L, &g_L, &b_L, &c_L);
  average_L = (r_L+g_L+b_L)/3;
  red_L = r_L/average_L;
  green_L = g_L/average_L;
  blue_L = b_L/average_L;
  if ((red_L < 0.95) && (green_L >= 1.3) && (blue_L <= 0.95)) {
    Serial.println("Green (L)!");
    greenBool_L = 1;
  } else {
    Serial.println("Not Green (L)!");
    greenBool_L = 0;
  }
  /* Color Sensor (Right) */
  tcaselect(1);
  uint16_t r_R, g_R, b_R, c_R;
  float average_R, red_R, green_R, blue_R;
  tcs_R.getRawData(&r_R, &g_R, &b_R, &c_R);
  average_R = (r_R+g_R+b_R)/3;
  red_R = r_R/average_R;
  green_R = g_R/average_R;
  blue_R = b_R/average_R;
  if ((red_R < 0.95) && (green_R >= 1.3) && (blue_R <= 0.95)) {
    Serial.println("Green (R)!");
    greenBool_R = 1;
  } else {
    Serial.println("Not Green (R)!");
    greenBool_R = 0;
  }
}

/* F U N C T I O N S */

void receiveEvent (int howMany) {
  command = Wire.read();
}

void requestEvent() {
  switch (command) {
     case COLOR_L: Wire.write(greenBool_L); break;
     case COLOR_R: Wire.write(greenBool_R); break;
  }
}

void tcaselect(uint8_t i) {
  if (i > 7) return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}
