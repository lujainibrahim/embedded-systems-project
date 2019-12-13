/* D E F I N I T I O N S */

#include <Wire.h> // I2C Library
#include <ZumoShield.h> // Zumo Shield Library
#define ADDR_1 0x10 // (Me)
#define ADDR_2 0x20

int greenBool_L = 0; // Received over I2C
int greenBool_R = 0; // Received over I2C

enum { // Commands to Sensors
  COLOR_L = 1,
  COLOR_R  = 2
};

ZumoBuzzer buzzer; // Not Used
ZumoReflectanceSensorArray reflectanceSensors; // Reflectance Sensor
ZumoMotors motors; // Motor Controls
Pushbutton button(ZUMO_BUTTON); // Calibration Button
int lastError = 0; // PID Variable
const int MAX_SPEED = 140; // Maximum Speed

/* S E T U P */

void setup() {
  /* Serial */
  Serial.begin(9600);
    /* I2C */
  Wire.begin(ADDR_1);
  buzzer.play(">g32>>c32"); // Necessary
  /* Zumo Shield */
  reflectanceSensors.init(); // Initialize reflectance sensor
  button.waitForButton(); // Wait for the user button to be pressed and released
  pinMode(13, OUTPUT); // LED
  digitalWrite(13, HIGH); // Turn on LED to indicate we are in calibration mode
  delay(1000);
  int i;
  for(i = 0; i < 80; i++) {
    if ((i > 10 && i <= 30) || (i > 50 && i <= 70))
      motors.setSpeeds(-200, 200);
    else
      motors.setSpeeds(200, -200);
    reflectanceSensors.calibrate();
    delay(20);
  }
  motors.setSpeeds(0,0);
  digitalWrite(13, LOW); // Turn off LED to indicate we are through with calibration
  buzzer.play(">g32>>c32"); // Necessary
  Serial.println("Calibration complete.");
  button.waitForButton(); // Wait for the user button to be pressed and released
  buzzer.play("L16 cdegreg4"); // Necessary
  while(buzzer.isPlaying()); // Necessary
}

/* M A I N  L O O P */

void loop() {
  /* Line Follower */
  unsigned int sensors[6];
  int position = reflectanceSensors.readLine(sensors);
  int error = position - 2500;
  int speedDifference = error / 4 + 6 * (error - lastError);
  lastError = error;
  int m1Speed = MAX_SPEED + speedDifference;
  int m2Speed = MAX_SPEED - speedDifference;
  if (m1Speed < 0)
    m1Speed = 0;
  if (m2Speed < 0)
    m2Speed = 0;
  if (m1Speed > MAX_SPEED)
    m1Speed = MAX_SPEED;
  if (m2Speed > MAX_SPEED)
    m2Speed = MAX_SPEED;
  motors.setSpeeds(m1Speed, m2Speed);
  
  /* Read Color Sensors */
  greenBool_L = readSensor(COLOR_L, 1);
  greenBool_R = readSensor(COLOR_R, 1);
  
  /* Left Turn */
  if (greenBool_L == 1 && greenBool_R == 0) {
    // To Be Calibrated
    stopMove();
    while(1);
  }
  
  /* Right Turn */
  if (greenBool_L == 0 && greenBool_R == 1) {
    // To Be Calibrated
    stopMove();
    while(1);
  }

  /* Turn Around */
  if (greenBool_L == 1 && greenBool_R == 1) {
    // To Be Calibrated
    stopMove();
    while(1);
  }  
}

/* F U N C T I O N S */

void turnMove(int signal, int turnValue) { // ~90° Movement
  int m1Speed, m2Speed;
  if (signal == 0) {
    m1Speed = 0;
    m2Speed = turnValue;
  } else if (signal == 1) {
    m1Speed = turnValue;
    m2Speed = 0;
  }
  motors.setSpeeds(m1Speed, m2Speed);
  delay(800);
}

void reverseMove(int turnValue) { // ~180° Movement
  motors.setSpeeds(-turnValue, 0);
  delay(2600);
  motors.setSpeeds(0, turnValue);
  delay(2100);
}

void stopMove() { // Stop Motors
  motors.setSpeeds(0, 0);
}

int readSensor(const byte command, const int responseSize) { // Read from I2C
  int value = 0;
  Wire.beginTransmission(ADDR_2);
  Wire.write(command);
  int status = Wire.endTransmission();
  if (status != 0) {
    digitalWrite(13, HIGH);
    while(1); // Pause Arduino
  } else {
    digitalWrite(13, LOW);
  }
  Wire.requestFrom(ADDR_2, responseSize, 1); 
  if (Wire.available() != 0) {
    value = Wire.read(); 
  }
  return value;
}
