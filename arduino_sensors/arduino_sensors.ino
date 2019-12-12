/* D E F I N I T I O N S */

#include <Wire.h> // I2C Library
#include <Adafruit_TCS34725.h> // TCS34725 Library
#define ADDR_1 0x10
#define ADDR_2 0x20 // (Me)
#define S0 11
#define S1 12
#define S2 4
#define S3 5
#define leftOut 9 // Label 1
#define rightOut 8 // Label 2

/* I2C */
enum { // Commands from Hub
  COLOR_L = 1,
  COLOR_R  = 2
};

int r_L = 0;
int g_L = 0;
int b_L = 0;
int r_R = 0;
int g_R = 0;
int b_R = 0;
int green_L = 0;
int green_R = 0;
char command;

/* S E T U P */

void setup() {
  /* Serial */
  Serial.begin(9600);
  /* I2C */
  command = 0;
  Wire.begin(ADDR_2);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  /* Pin Modes */
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(leftOut, INPUT);
  pinMode(rightOut, INPUT);
  digitalWrite(S0,HIGH); 
  digitalWrite(S1,LOW);
//  calibrateColor();
}

/* M A I N  L O O P */

void loop() { 
  /* Read Left Color Sensor */
  setRed();
  r_L = pulseIn(leftOut, LOW);
  setGreen();
  g_L = pulseIn(leftOut, LOW);
  setBlue();
  b_L = pulseIn(leftOut, LOW);
  /* Left Results */
  green_L = checkGreen(r_L, g_L, b_L);
  Serial.print("Left: ");
  printRGB(r_L, g_L, b_L);
  if (green_L == 1) {
    Serial.println("Green Left!");
  }
  /* Read Right Color Sensor */
  setRed();
  r_R = pulseIn(rightOut, LOW);
  setGreen();
  g_R = pulseIn(rightOut, LOW);
  setBlue();
  b_R = pulseIn(rightOut, LOW);
  /* Right Results */
  green_R = checkGreen(r_R, g_R, b_R);
  Serial.print("Right: ");
  printRGB(r_R, g_R, b_R);
  if (green_R == 1) {
    Serial.println("Green Right!");
  }
  delay(500);
}

/* F U N C T I O N S */

void receiveEvent (int howMany) { // Read from I2C
  command = Wire.read();
}

void requestEvent() { // Send over I2C
  switch (command) {
     case COLOR_L: Wire.write(green_L); break;
     case COLOR_R: Wire.write(green_R); break;
  }
}

void setRed() {
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);
}

void setGreen() {
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
}

void setBlue() {
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
}

void printRGB(int red, int green, int blue) {
  Serial.print("R: ");
  Serial.print(red);
  Serial.print(" ");
  Serial.print("G: ");
  Serial.print(green);
  Serial.print(" ");
  Serial.print("B: ");
  Serial.print(blue);
  Serial.print(" \n");
}

int checkGreen(int red, int green, int blue) { // Manual Calibration
  if (red >= 60 && red <= 110 && green >= 50 && blue >= 50) {
    return 1;
  } else {
    return 0;
  }
}

void waitChar(char C) {
  while(1) {
    if(Serial.available() > 0) {
      if(Serial.read() == (unsigned char) C)
        break;
    }
  }
}

void sortAscend(int arr[], int arrSize) { // Sort Array
  int i, j, k;
  for(i=0; i<arrSize; i++) {   
    for(j=i+1; j<arrSize; j++) {
      if(arr[i] > arr[j]) {
        k  = arr[i];
        arr[i] = arr[j];
        arr[j] = k;
      }
    }
  }
}

void getMinMax() { // Find Color Constraints
  Serial.println("Reading...");
  int reading[500];
  for (int j = 0; j < 500; j++) {
    reading[j] = pulseIn(leftOut, LOW);
    Serial.println(reading[j]);
  }
  sortAscend(reading, 500);
  Serial.println("Maximum: ");
  Serial.println(reading[499]);
  Serial.println(" Minimum: ");
  Serial.println(reading[0]);
}

void calibrateColor() { // Calibration Function
  for (int i = 0; i < 5; i++) {
    Serial.println("Place the next color under the sensor. Enter 'G' to continue.");
    waitChar('G');
    for (int color = 0; color < 3; color++) {
      switch(color) {
        case 0: // Red
          Serial.println("R: Enter 'G' to continue.");
          waitChar('G');
          setRed();
          getMinMax();
          break;
        case 1: // Green
          Serial.println("G: Enter 'G' to continue.");
          waitChar('G');
          setGreen();
          getMinMax();
          break;
        case 2: // Blue
          Serial.println("B: Enter 'G' to continue.");
          waitChar('G');
          setBlue();
          getMinMax();
          break;
      }
    }
  }
}
