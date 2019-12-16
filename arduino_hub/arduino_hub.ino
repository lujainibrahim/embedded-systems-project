/* D E F I N I T I O N S */

#include <Wire.h> // I2C Library
#include <ZumoShield.h> // Zumo Shield Library
#define ADDR_1 0x10 // (Me)
#define ADDR_2 0x20
#define TURN_BASE_SPEED 150 // Base speed when turning
#define CALIBRATION_SAMPLES 70  // Number of compass readings to take when calibrating
#define CRB_REG_M_2_5GAUSS 0x60 // CRB_REG_M value for magnetometer +/-2.5 gauss full scale
#define CRA_REG_M_220HZ    0x1C // CRA_REG_M value for magnetometer 220 Hz update rate
#define DEVIATION_THRESHOLD 5 // Allowed deviation relative to target angle that must be achieved

/* Color Sensors */
int value_L = 0; // Received over I2C
int value_R = 0; // Received over I2C
enum { // Commands to Sensors
  COLOR_L = 1,
  COLOR_R  = 2
};

/* Zumo Shield */
ZumoBuzzer buzzer;
ZumoReflectanceSensorArray reflectanceSensors;
ZumoMotors motors;
Pushbutton button(ZUMO_BUTTON);
int MAX_SPEED = 150;

/* Accelerometer */
LSM303 compass;

/* Ultrasonic Sensor */
const int trigPin = A1;
const int echoPin = 13;
long duration, cm;

/* PID Controller */
int Kp = 1/4;
int Kd = 6;
int Ki = 1;
int error = 0;
int lastError = 0;
int cumError = 0;

/* Counters */
int colorRead = 0; // Read Counter
int loopTime = 0;

/* S E T U P */

void setup() {
  /* Ultrasonic Sensor */
  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);
  /* LED */
  pinMode(6, OUTPUT);
  /* Serial */
//  Serial.begin(9600);
    /* I2C */
  Wire.begin(ADDR_1);
  /* Accelerometer */
  LSM303::vector<int16_t> running_min = {32767, 32767, 32767}, running_max = {-32767, -32767, -32767};
  unsigned char index;
  compass.init();
  compass.enableDefault();
  compass.writeReg(LSM303::CRB_REG_M, CRB_REG_M_2_5GAUSS); // +/- 2.5 Gauss Sensitivity
  compass.writeReg(LSM303::CRA_REG_M, CRA_REG_M_220HZ); // 220 Hz Compass update rate
  button.waitForButton();
  motors.setSpeeds(200, -200);
  for(index = 0; index < CALIBRATION_SAMPLES; index ++) {
    compass.read();
    running_min.x = min(running_min.x, compass.m.x);
    running_min.y = min(running_min.y, compass.m.y);
    running_max.x = max(running_max.x, compass.m.x);
    running_max.y = max(running_max.y, compass.m.y);
    delay(50);
  }
  motors.setSpeeds(0, 0);
  compass.m_max.x = running_max.x;
  compass.m_max.y = running_max.y;
  compass.m_min.x = running_min.x;
  compass.m_min.y = running_min.y;
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

/* L O O P */

void loop() {
  /* Line Follower */
  loopTime++;
  if (loopTime == 100) { 
    // P L A C E H O L D E R
    loopTime = 0;
  }
  if (loopTime % 2 == 0) {
    cumError = 0;
  }
  unsigned int sensors[6];
  int position = reflectanceSensors.readLine(sensors);
  error = position - 2500;
  cumError += error; 
  int speedDifference = error / 4 + 6 * (error - lastError);
//  int speedDifference = Kp*error + Kd*(error - lastError) + Ki*cumError;
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
  
  /* Color Sensors Check */
  colorRead++;
  if (colorRead == 50) {
    value_L = readSensor(COLOR_L, 1);
    value_R = readSensor(COLOR_R, 1);
    /* Left Turn */
    if (value_L == 1 && value_R == 0) {
      int check = colorConfirm(COLOR_R);
      if (check == 0) {
        // P L A C E H O L D E R
        motors.setSpeeds(125, 125);
        delay(450);
        turnAngle(-89);
      } else if (check == 1) {
        // P L A C E H O L D E R
        motors.setSpeeds(-125, -125);
        delay(450);
        turnAngle(179);
      }
    }
    /* Right Turn */
    if (value_L == 0 && value_R == 1) {
      int check = colorConfirm(COLOR_L);
      if (check == 0) {
        // P L A C E H O L D E R
        motors.setSpeeds(125, 125);
        delay(450);
        turnAngle(89);
      } else if (check == 1) {
        // P L A C E H O L D E R
        motors.setSpeeds(-125, -125);
        delay(450);
        turnAngle(179);
      }
    }  
    /* Turn Around */
    if (value_L == 1 && value_R == 1) {
      // P L A C E H O L D E R
      motors.setSpeeds(-125, -125);
      delay(450);
      turnAngle(179);
    }
    /* Reset Counter */
    colorRead = 0;
  }
}

/* F U N C T I O N S */

/* Hard-Coded Movements */

void stopMove() {
  motors.setSpeeds(0, 0);
}

/* Color Sensors */

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

/* Accelerometer (Turns) */

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
        stopMove();
        delay(100);
        target_heading = fmod(averageHeading() + angle, 360);
        check = 1;
      } else {
        stopMove();
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
