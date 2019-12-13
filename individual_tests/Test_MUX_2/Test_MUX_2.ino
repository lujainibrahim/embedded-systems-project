#include <Wire.h>
#include <Adafruit_TCS34725.h>
#define TCAADDR 0x70

Adafruit_TCS34725 tcs_L = Adafruit_TCS34725();
Adafruit_TCS34725 tcs_R = Adafruit_TCS34725();
int greenBool_L = 0;
int greenBool_R = 0;

void tcaselect(uint8_t i) {
  if (i > 7) return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  tcaselect(1);
  if (tcs_L.begin()) {
    Serial.println("TCS34725 (Left) detected.");
  } else {
    Serial.println("TCS34725 (Left) not detected.");
    while(1);
  }
  tcaselect(2);
  if (tcs_R.begin()) {
    Serial.println("TCS34725 (Right) detected.");
  } else {
    Serial.println("TCS34725 (Right) not detected.");
    while(1);
  }
}

void loop() {
  tcaselect(1);
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

  tcaselect(2);
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
