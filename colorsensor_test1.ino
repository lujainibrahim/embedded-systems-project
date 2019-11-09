/*Example code for TCS34725 RGB Color Sensor with Arduino and Adafruit TCS34725 library. More info: https://www.makerguides.com/tcs34725-rgb-color-sensor-arduino-tutorial/ */
// Include the libraries:
#include <Wire.h>
#include <Adafruit_TCS34725.h>
// Define the variables:
uint16_t r, g, b, c, colorTemp, lux;
// Initialise with specific int time and gain values:
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);
void setup()
{
  // Begin Serial communication:
  Serial.begin(9600);
  // Check if the sensor is wired correctly:
  if (tcs.begin()) {
    Serial.println("Found sensor");
  }
  else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
}
void loop()
{
  // Get the raw sensor data for the red, green, blue and clear photodiodes:
  tcs.getRawData(&r, &g, &b, &c);
  // Calculate the color temperature using all the sensor data:
  colorTemp = tcs.calculateColorTemperature_dn40(r, g, b, c);
  // Calculate lux using red, green and blue sensor data:
  lux = tcs.calculateLux(r, g, b);
  // Print the data to the Serial Monitor:
  Serial.print("Color Temp: "); Serial.print(colorTemp, DEC); Serial.print(" K - ");
  Serial.print("Lux: "); Serial.print(lux, DEC); Serial.print(" - ");
  Serial.print("R: "); Serial.print(r, DEC); Serial.print(" ");
  Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
  Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
  Serial.print("C: "); Serial.print(c, DEC); Serial.print(" ");
  Serial.println(" ");
}
