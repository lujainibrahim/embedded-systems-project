/* D E F I N I T I O N S */

#include <Wire.h> // I2C Library
#include <Adafruit_TCS34725.h> // TCS34725 Library
#define ADDR_1 0x10
#define ADDR_2 0x20 // (Me)
#define TCAADDR 0x70

/* I2C */
enum { // Commands from Hub
  COLOR_L = 1,
  COLOR_R  = 2
};

Adafruit_TCS34725 tcs_L = Adafruit_TCS34725(); // Colour Sensor (Left)
Adafruit_TCS34725 tcs_R = Adafruit_TCS34725(); // Colour Sensor (Right)
char command;
int value_L = 0; // Left Sensor
int value_R = 0; // Right Sensor

/* S E T U P */

void setup() {
  pinMode(13, OUTPUT);
  /* Serial */
//   Serial.begin(9600);
  /* I2C */
  command = 0;
  Wire.begin(ADDR_2);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  /* Colour Sensors */
  tcaselect(2); // Left Sensor
  if (tcs_L.begin()) {
//    Serial.println("TCS34725 (Left) detected.");
  } else {
//    Serial.println("TCS34725 (Left) not detected.");
    while(1);
  }
  tcaselect(1); // Right Sensor
  if (tcs_R.begin()) {
//    Serial.println("TCS34725 (Right) detected.");
  } else {
//    Serial.println("TCS34725 (Right) not detected.");
    while(1);
  }
}

void loop() {
  /* Color Sensor (Left) */
  tcaselect(2);
  uint16_t r_L, g_L, b_L, c_L, lux_L;
  float average_L, red_L, green_L, blue_L;
  tcs_L.getRawData(&r_L, &g_L, &b_L, &c_L);
  average_L = (r_L+g_L+b_L)/3;
  red_L = r_L/average_L;
  green_L = g_L/average_L;
  blue_L = b_L/average_L;
  lux_L = tcs_L.calculateLux(r_L, g_L, b_L);
  if ((red_L <= 0.95) && (green_L >= 1.3) && (blue_L <= 1.2)) {
//    Serial.println("Green (L)!");
    value_L = 1;
  } else if ((red_L >= 0.85) && (green_L <= 1.2) && (blue_L >= 0.95) && (lux_L <= 25)) {
//    Serial.println("Black (L)!");
     value_L = 2;
  } else {
//    Serial.println("Nothing.");
    value_L = 0;
  }
  /* Color Sensor (Right) */
  tcaselect(1);
  uint16_t r_R, g_R, b_R, c_R, lux_R;
  float average_R, red_R, green_R, blue_R;
  tcs_R.getRawData(&r_R, &g_R, &b_R, &c_R);
  average_R = (r_R+g_R+b_R)/3;
  red_R = r_R/average_R;
  green_R = g_R/average_R;
  blue_R = b_R/average_R;
  lux_R = tcs_R.calculateLux(r_R, g_R, b_R);
  if ((red_R <= 0.95) && (green_R >= 1.3) && (blue_R <= 1.2)) {
//    Serial.println("Green (R)!");
    value_R = 1;
  } else if ((red_R >= 0.85) && (green_R <= 1.2) && (blue_R >= 0.95) && (lux_R <= 25)) {
//    Serial.println("Black (R)!");
     value_R = 2;
  } else {
//    Serial.println("Nothing.");
    value_R = 0;
  }
  if (value_L == 2 && value_R == 2) {
    digitalWrite(13, HIGH);
  } else {
    digitalWrite(13, LOW);
  }
//  Serial.print(lux_R);
//  Serial.print(" ");
//  Serial.print(red_R);
//  Serial.print(" ");
//  Serial.print(green_R);
//  Serial.print(" ");
//  Serial.println(blue_R);
//  delay(1000);
}

/* F U N C T I O N S */

void receiveEvent (int howMany) {
  command = Wire.read();
}

void requestEvent() {
  switch (command) {
     case COLOR_L: Wire.write(value_L); break;
     case COLOR_R: Wire.write(value_R); break;
  }
}

void tcaselect(uint8_t i) {
  if (i > 7) return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}
