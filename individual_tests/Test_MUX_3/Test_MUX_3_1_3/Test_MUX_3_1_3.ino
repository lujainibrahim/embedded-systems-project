/* D E F I N I T I O N S */

#include <Wire.h> // I2C Library
#include <ZumoShield.h> // Zumo Shield Library
#define ADDR_1 0x10 // (Me)
#define ADDR_2 0x20
#define TURN_BASE_SPEED 100
#define CALIBRATION_SAMPLES 70  // Number of compass readings to take when calibrating
#define CRB_REG_M_2_5GAUSS 0x60 // CRB_REG_M value for magnetometer +/-2.5 gauss full scale
#define CRA_REG_M_220HZ    0x1C // CRA_REG_M value for magnetometer 220 Hz update rate
#define DEVIATION_THRESHOLD 5

int value_L = 0; // Received over I2C
int value_R = 0; // Received over I2C

/* Color Sensor */
enum { // Commands to Sensors
  COLOR_L = 1,
  COLOR_R  = 2
};
int colorRead = 0;
unsigned long previousMillis_L;
unsigned long currentMillis_L;
unsigned long previousMillis_R;
unsigned long currentMillis_R;
unsigned long timeElapsed_L;
unsigned long timeElapsed_R;

/* Zumo Shield */
ZumoBuzzer buzzer; // Not Used
ZumoReflectanceSensorArray reflectanceSensors; // Reflectance Sensor
ZumoMotors motors; // Motor Controls
Pushbutton button(ZUMO_BUTTON); // Calibration Button
int lastError = 0; // PID Variable
int MAX_SPEED = 150;
int MIN_SPEED = -120;

/* Accelerometer */
LSM303 compass;
bool onIncline = 0;
bool goingUp = 0;

/* Ultrasonic Sensor */
const int trigPin = A1;
const int echoPin = 13;
long duration, cm;
int loopTime = 0;

/*  Detect Stationary */
int change = 0;
int totalChange = 0;
int prevError = 0;
int error = 0;
int stationaryCheck = 0;

/* PID Control */
int currentTime = 1;
int elapsedTime = 1;
int previousTime = 1;

/* Gap */
unsigned long previousMillis;
unsigned long currentMillis;
unsigned long timeElapsed;
int errorGap;
int position;
unsigned int sensors[6]; //make global for others too
int gapCounter = 0;
int maxGap = 1000; // subject to change

/* S E T U P */
void setup() {
  /* Ultrasonic Sensor */
  pinMode(echoPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  /* LED */
  pinMode(6, OUTPUT);
  /* Serial */
  Serial.begin(9600);
    /* I2C */
  Wire.begin(ADDR_1);
  /* Accelerometer */
//  LSM303::vector<int16_t> running_min = {32767, 32767, 32767}, running_max = {-32767, -32767, -32767};
//  unsigned char index;
//  compass.init();
//  compass.enableDefault();
//  compass.writeReg(LSM303::CRB_REG_M, CRB_REG_M_2_5GAUSS); // +/- 2.5 gauss sensitivity to hopefully avoid overflow problems
//  compass.writeReg(LSM303::CRA_REG_M, CRA_REG_M_220HZ);    // 220 Hz compass update rate
//  button.waitForButton();
//  motors.setSpeeds(200, -200);
//  for(index = 0; index < CALIBRATION_SAMPLES; index ++) {
//    compass.read();
//    running_min.x = min(running_min.x, compass.m.x);
//    running_min.y = min(running_min.y, compass.m.y);
//    running_max.x = max(running_max.x, compass.m.x);
//    running_max.y = max(running_max.y, compass.m.y);
//    delay(50);
//  }
//  motors.setSpeeds(0, 0);
//  compass.m_max.x = running_max.x;
//  compass.m_max.y = running_max.y;
//  compass.m_min.x = running_min.x;
//  compass.m_min.y = running_min.y;
  /* Zumo Shield */
  buzzer.play(">g32>>c32"); // Necessary
  reflectanceSensors.init();
  button.waitForButton();
  digitalWrite(6, HIGH); 
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
  digitalWrite(6, LOW);
  buzzer.play(">g32>>c32"); // Necessary
  Serial.println("Calibration complete.");
  button.waitForButton();
  buzzer.play("L16 cdegreg4"); // Necessary
  while(buzzer.isPlaying()); // Necessary
}

/* M A I N  L O O P */

void loop() {
  /* Obstacle and Ramp Incline Check */
  loopTime++;
  stationaryCheck++;
  if (loopTime == 100) { 
    /* Incline and Accelerometer */
//    compass.read();
//    changeSpeedBasedOnIncline();
    /* Ultrasonic Sensor */
//    ultrasonicSensor();
    loopTime = 0;
  }
  
  /* Line Follower */

  position = reflectanceSensors.readLine(sensors);
  prevError = error;
  error = position - 2500;
  change = abs(error) - abs(prevError);
  totalChange += change;
  int speedDifference = error / 4 + 6 * (error - lastError);
  lastError = error;  
  int m1Speed = MAX_SPEED + speedDifference;
  int m2Speed = MAX_SPEED - speedDifference;
  if (m1Speed < 0)
    m1Speed = MIN_SPEED;
  if (m2Speed < 0)
    m2Speed = MIN_SPEED;
  if (m1Speed > MAX_SPEED)
    m1Speed = MAX_SPEED;
  if (m2Speed > MAX_SPEED)
    m2Speed = MAX_SPEED;
  
  /* Ramp and Speed Bump Stuck Check */
  if (stationaryCheck == 800) { // Check every 800 loops
//    rampCheck();
//    speedBumpCheck();
    stationaryCheck = 0;
    totalChange = 0;
  }

  /* GAP LOGIC */
  gap();
  
  motors.setSpeeds(m1Speed, m2Speed);
  
  /* Color Sensors Check */
  colorRead++;
  if (colorRead == 15) { // Delay for I2C
    value_L = readSensor(COLOR_L, 1);
    value_R = readSensor(COLOR_R, 1);
    if (value_L == 2) {
      previousMillis_L = millis();
    }
    if (value_R == 2) {
      previousMillis_R = millis();
    }
    /* Left Turn */
    if (value_L == 1 && value_R == 0) {
      currentMillis_L = millis();
      timeElapsed_L = currentMillis_L - previousMillis_L;
      if (timeElapsed_L > 700) {
        int check = colorConfirm(COLOR_R);
        if (check == 0) {
          motors.setSpeeds(150, 150);
          delay(400);
          motors.setSpeeds(-180, 180);
          delay(530);
        } else if (check == 1) {
          motors.setSpeeds(-150, -150);
          delay(250);
          motors.setSpeeds(180, -180);
          delay(1300);
        }
      }
    }
    /* Right Turn */
    if (value_L == 0 && value_R == 1) {
      currentMillis_R = millis();
      timeElapsed_R = currentMillis_R - previousMillis_R;
      if (timeElapsed_R > 700) {
        int check = colorConfirm(COLOR_L);
        if (check == 0) {
          motors.setSpeeds(150, 150);
          delay(400);
          motors.setSpeeds(180, -180);
          delay(530);
        } else if (check == 1) {
          motors.setSpeeds(-150, -150);
          delay(250);
          motors.setSpeeds(180, -180);
          delay(1300);
        }
      }
    }  
    /* Turn Around */
    if (value_L == 1 && value_R == 1) {
      motors.setSpeeds(-150, -150);
      delay(250);
      motors.setSpeeds(180, -180);
      delay(1300);
    }
    /* Reset Counter */
    colorRead = 0;
  }
}

/* F U N C T I O N S */

void turnSide(char signal, int t1, int t2, int turnValue) { // ~90° Movement
  int m1Speed, m2Speed;
  if (signal == 'L') {
    m1Speed = 0;
    m2Speed = turnValue;
  } else if (signal == 'R') {
    m1Speed = turnValue;
    m2Speed = 0;
  }
  delay(t1);
  motors.setSpeeds(m1Speed, m2Speed);
  delay(t2);
}

void turnAround(int t1, int t2, int turnValue) { // ~180° Movement
  motors.setSpeeds(-turnValue, 0);
  delay(t1);
  motors.setSpeeds(0, turnValue);
  delay(t2);
}

void stopMove() { // Stop Motors
  motors.setSpeeds(0, 0);
}

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

int colorConfirm(const byte command) {
  stopMove();
  int check = 0;
  for (int i = 1; i < 11; i++) {
    if (i % 2 == 0) { // Delay for I2C
      check = readSensor(command, 1);
      if (check == 1) {
        break;
      }
    }
  }
  return check;
}

/* Speed Bump Check */
void speedBumpCheck() {
    /* Hit a speed bump */
    if (!onIncline && abs(totalChange) < 20) {
        motors.setSpeeds(-100,-100);
        delay(500);
        motors.setSpeeds(100,100);
        delay(700);
        motors.setSpeeds(0, -200);
        delay(450);
        motors.setSpeeds(-200, 0);
        delay(800);
        motors.setSpeeds(100,100);
        delay(300);
        motors.setSpeeds(350, 0);
        delay(450);
        motors.setSpeeds(0, 350);
        delay(500);  
   }
}

/* Ramp or Speedbump Stuck Check */
void rampCheck() {
  if (onIncline && abs(totalChange) < 20) {  // if on incline and reflectance sensor not changing (Stationary)
    motors.setSpeeds(325, 325);
    delay(325);
    prevError = 0;
  }
}

/* Accelerometer */
bool foundIncline() {
  int incline = compass.a.x;
  int inclineCount = 0;
  if (incline > 0) {
    goingUp = true;
  } else {
    goingUp = false;
  }
  if (abs(incline) > 3000) {
    for (int i = 0; i < 10; i++) {
      if (abs(compass.a.x) > 3000) {
        inclineCount++;
      }
    }
    if (inclineCount == 10) {
      digitalWrite(6, HIGH);
      return true;
    }
  } else {
    digitalWrite(6, LOW);
    return false;
  }
}

/* Change Speed Based on Incline */
void changeSpeedBasedOnIncline() {
  if (foundIncline()) {
    if (goingUp) {
      MAX_SPEED = 180;
    } else {
      MAX_SPEED = 40;
    }
    onIncline = true;
  } else {
    MAX_SPEED = 140;
    onIncline = false;
  }
}

/* Microseconds to Centimeters Conversion */
long microsecondsToCentimeters(long microseconds) {
  return microseconds / 29 / 2;
}

/* Ultrasonic Sensor */
void ultrasonicSensor() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(1);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(8);
    digitalWrite(trigPin, LOW);
//    pinMode(echoPin, INPUT);
    duration = pulseIn(echoPin, HIGH);
    cm = microsecondsToCentimeters(duration);
    Serial.print(cm);
    if (cm < 18) {
      digitalWrite(6, HIGH);
      Stop();
      delay(100000);
//      delay(100);
//      moveRight();
//      delay(700);
//      moveForward();
//      delay(500);
//      moveLeft();
//      delay(650);
//      moveForward();
//      delay(300);
//      moveLeft();
//      delay(500);
//      moveForward();
//      delay(100);
      digitalWrite(6, LOW);
    } else {
      digitalWrite(6, LOW);
    }
}

/* Motor */

void moveLeft() {
   motors.setSpeeds(0, 200);
}
void moveForward() {
  motors.setSpeeds(200,200);
}
void moveRight() {
    motors.setSpeeds(200, 0);
}
void Stop() {
  motors.setSpeeds(0,0);
}

template <typename T> float heading(LSM303::vector<T> v) {
  float x_scaled =  2.0*(float)(v.x - compass.m_min.x) / ( compass.m_max.x - compass.m_min.x) - 1.0;
  float y_scaled =  2.0*(float)(v.y - compass.m_min.y) / (compass.m_max.y - compass.m_min.y) - 1.0;

  float angle = atan2(y_scaled, x_scaled)*180 / M_PI;
  if (angle < 0)
    angle += 360;
  return angle;
}

float relativeHeading(float heading_from, float heading_to) {
  float relative_heading = heading_to - heading_from;
  if (relative_heading > 180)
    relative_heading -= 360;
  if (relative_heading < -180)
    relative_heading += 360;
  return relative_heading;
}

float averageHeading() {
  LSM303::vector<int32_t> avg = {0, 0, 0};
  for(int i = 0; i < 10; i ++) {
    compass.read();
    avg.x += compass.m.x;
    avg.y += compass.m.y;
  }
  avg.x /= 10.0;
  avg.y /= 10.0;
  return heading(avg);
}

void turnAngle(int angle) {
  int check = 0;
  while(1) {
    float heading, relative_heading;
    int speed;
    static float target_heading = averageHeading();
    heading = averageHeading();
    relative_heading = relativeHeading(heading, target_heading);
    if(abs(relative_heading) < DEVIATION_THRESHOLD) {
      if (check == 0) {
        motors.setSpeeds(0, 0);
        delay(100);
        target_heading = fmod(averageHeading() + angle, 360);
        check = 1;
      } else {
        motors.setSpeeds(125, 125);
        delay(250);
        break;
      }
    } else {
      speed = MAX_SPEED*relative_heading/180;
      if (speed < 0)
        speed -= TURN_BASE_SPEED;
      else
        speed += TURN_BASE_SPEED;
      motors.setSpeeds(speed, -speed);
    }
  }
}

void gap() {
  if (abs(error) == 2500) {
    if (gapCounter == 0) { // calculate first Time
      previousMillis = millis(); // millis() returns an unsigned long.
    }
    gapCounter++;
    
    if (gapCounter > maxGap) { // detected a Gap
      currentMillis = millis(); // grab current time
      timeElapsed = currentMillis - previousMillis;  
      Serial.println(timeElapsed);
      motors.setSpeeds(0,0);
      delay(500);
      
      /* Go back to edge of black line*/
      
      /* determine if to go left or right */
      if (error > 0) { // turned right
        // make it turn left backwards
          motors.setSpeeds(-MAX_SPEED,-MIN_SPEED);
          delay(timeElapsed);
          motors.setSpeeds(0,0);
      }
      
      else if (error < 0) { // turned left
        // make it turn right backwards
          motors.setSpeeds(-MIN_SPEED,-MAX_SPEED);
        delay(timeElapsed);
        motors.setSpeeds(0,0);      
       }
       
       /* Go Forward Slightly */
       motors.setSpeeds(100,100);      
       delay(200);
      
      position = reflectanceSensors.readLine(sensors);
      errorGap = position - 2500;
      while (abs(errorGap)==2500){ // while on white, keep going forward
        motors.setSpeeds(150, 150);
        position = reflectanceSensors.readLine(sensors);
        errorGap = position - 2500;
      }
      gapCounter = 0;
    }
  }
  else {
    gapCounter = 0;
  }
}
