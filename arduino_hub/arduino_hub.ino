#include <Wire.h> // I2C Library
#include <ZumoShield.h> // Zumo Shield Library
#define ADDR_1 0x10 // (Me)
#define ADDR_2 0x20

int greenBool_L = 0; // Received over I2C
int greenBool_R = 0; // Received over I2C

enum { // Commands to Sensors
  COLOUR_L = 1,
  COLOUR_R  = 2
};

ZumoBuzzer buzzer;
ZumoReflectanceSensorArray reflectanceSensors;
ZumoMotors motors;
Pushbutton button(ZUMO_BUTTON);
int lastError = 0;

const int MAX_SPEED = 400; // Maximum Speed

void readSensor(const byte command, const int responseSize) {
  Wire.beginTransmission(ADDR_2);
  Wire.write(command);
  Wire.endTransmission();
  Wire.requestFrom (ADDR_2, responseSize); 
}

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

void loop() {
  /* Zumo Shield */
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

  /* Colour Sensor (Left) */

  /* Colour Sensor (Right) */
  readSensor(COLOUR_R, 1);
  greenBool_R = Wire.read(); 
  if (greenBool_R == 1) {
    int position_check = reflectanceSensors.readLine(sensors);
    int error_check = position_check - 2500;
    while(error_check < -400) {
      m1Speed = 100; // Fixed Speed
      m2Speed = 0;
      motors.setSpeeds(m1Speed, m2Speed);
      position_check = reflectanceSensors.readLine(sensors);
      error_check = position_check - 2500;
    }
    delay(400);
  }
}
