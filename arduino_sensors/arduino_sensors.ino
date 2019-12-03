#include <Wire.h> // I2C Library
#include <Adafruit_TCS34725.h> // TCS34725 Library
#define ADDR_1 0x10
#define ADDR_2 0x20 // (Me)

Adafruit_TCS34725 tcs = Adafruit_TCS34725(); // Colour Sensor
int greenBool = 0;

void setup() {
   Serial.begin(9600);

  /* I2C */
  Wire.begin(ADDR_2);
    if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
}

void loop() {

  uint16_t r, g, b, c, colorTemp, lux,X;
  float average, red, green, blue;
  tcs.getRawData(&r, &g, &b, &c);
  average= (r+g+b)/3;
  red= r/average;
  green= g/average;
  blue= b/average;
  colorTemp = tcs.calculateColorTemperature_dn40(r, g, b, c);
  lux = tcs.calculateLux(r, g, b);
  if ((red<0.95) && (green>=1.3) && (blue<=0.95)) {
    greenBool = 1;
  } else {
    greenBool = 0;
  }
   if ( (red>1.4) && (green<0.9) &&(blue<0.9))
  {
    Serial.print ("red");
  }
  else if ((red<0.95) && (green>=1.3) && (blue<=0.95))
  {
    Serial.print ("green");
    Serial.println();
  }
  else if ((red<0.8) && (green<1.2) && (blue>1.2))
  {
    Serial.print ("blue");
  }
  Wire.beginTransmission(ADDR_1);
  Wire.write(greenBool);
  int response = Wire.endTransmission();
  if (response != 0) {
    Serial.print("Error!\n");
    Serial.print(response);
    Serial.println();
  }
}
