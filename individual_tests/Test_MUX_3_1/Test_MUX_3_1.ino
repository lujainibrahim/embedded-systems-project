/* D E F I N I T I O N S */

#include <Wire.h> // I2C Library
#include <ZumoShield.h> // Zumo Shield Library
#define ADDR_1 0x10 // (Me)
#define ADDR_2 0x20
ZumoMotors motors; // Motor Controls
Pushbutton button(ZUMO_BUTTON); // Calibration Button

int greenBool_L = 0; // Received over I2C
int greenBool_R = 0; // Received over I2C

enum { // Commands to Sensors
  COLOR_L = 1,
  COLOR_R  = 2
};

/* S E T U P */

void setup() {
  pinMode(13, OUTPUT);
  /* Serial */
//  Serial.begin(9600);
    /* I2C */
  Wire.begin(ADDR_1);
  button.waitForButton();
}

/* M A I N  L O O P */

void loop() {
  /* Read Color Sensors */
  greenBool_L = readSensor(COLOR_L, 1);
  greenBool_R = readSensor(COLOR_R, 1);
  /* Left Turn */
  if (greenBool_L == 1 && greenBool_R == 0) {
    motors.setSpeeds(0, 400);
    delay(500);
    motors.setSpeeds(0, 0);
    while(1);
  }
  
  /* Right Turn */
  if (greenBool_L == 0 && greenBool_R == 1) {
    motors.setSpeeds(400, 0);
    delay(500);
    motors.setSpeeds(0, 0);
    while(1);
  }

  /* Turn Around */
  if (greenBool_L == 1 && greenBool_R == 1) {
    motors.setSpeeds(400, 400);
    delay(500);
    motors.setSpeeds(0, 0);
    while(1);
  }
  delay(50);
}

/* F U N C T I O N S */

int readSensor(const byte command, const int responseSize) { // Read from I2C
  int value = 0;
  Wire.beginTransmission(ADDR_2);
  Wire.write(command);
  int status = Wire.endTransmission(0);
  Wire.requestFrom(ADDR_2, responseSize, 1); 
  if (Wire.available() != 0) {
    value = Wire.read(); 
  }
  return value;
}
