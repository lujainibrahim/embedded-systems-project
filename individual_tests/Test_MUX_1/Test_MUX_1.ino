#include <Wire.h>
#include <Adafruit_TCS34725.h>
#define TCAADDR 0x70

Adafruit_TCS34725 tcs_L = Adafruit_TCS34725();
int greenBool_L = 0; // Left Sensor

void tcaselect(uint8_t i) {
  if (i > 7) return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  tcaselect(2);
  if (tcs_L.begin()) {
    Serial.println("TCS34725 (Left) detected.");
  } else {
    Serial.println("TCS34725 (Left) not detected.");
    while(1);
  }
}

void loop() {
  tcaselect(2);
  uint16_t r_L, g_L, b_L, c_L;
  float average_L, red_L, green_L, blue_L;
  tcs_L.getRawData(&r_L, &g_L, &b_L, &c_L);
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
}
