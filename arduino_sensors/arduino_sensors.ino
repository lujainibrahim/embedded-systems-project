#include <Wire.h> // I2C Library
#include <Adafruit_TCS34725.h> // TCS34725 Library
#define ADDR_1 0x10
#define ADDR_2 0x20 // (Me)

/* I2C */
enum { // Commands from Hub
  COLOUR_L = 1,
  COLOUR_R  = 2
};

/* Colour Sensor (Right) */
Adafruit_TCS34725 tcs_R = Adafruit_TCS34725(); // Colour Sensor (Right)
char command;
int greenBool_L = 0; // Left Sensor
int greenBool_R = 0; // Right Sensor

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
  uint16_t r_R, g_R, b_R, c_R;
  float average_R, red_R, green_R, blue_R;
  tcs_R.getRawData(&r_R, &g_R, &b_R, &c_R);
  average_R = (r_R+g_R+b_R)/3;
  red_R = r_R/average_R;
  green_R = g_R/average_R;
  blue_R = b_R/average_R;
  if ((red_R < 0.95) && (green_R >= 1.3) && (blue_R <= 0.95)) {
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
     case COLOUR_L: Wire.write(greenBool_L); break;
     case COLOUR_R: Wire.write(greenBool_R); break;
  }
}
