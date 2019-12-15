/*
 * Demo line-following code for the Pololu Zumo Robot
 *
 * This code will follow a black line on a white background, using a
 * PID-based algorithm.  It works decently on courses with smooth, 6"
 * radius curves and has been tested with Zumos using 30:1 HP and
 * 75:1 HP motors.  Modifications might be required for it to work
 * well on different courses or with different motors.
 *
 * https://www.pololu.com/catalog/product/2506
 * https://www.pololu.com
 * https://forum.pololu.com
 */

#include <Wire.h>
#include <ZumoShield.h>
#include <LSM303.h>

ZumoBuzzer buzzer;
ZumoReflectanceSensorArray reflectanceSensors;
ZumoMotors motors;
Pushbutton button(ZUMO_BUTTON);
int lastError = 0;
int linefollowed = 0;
LSM303 compass;

int m1Speed;
int m2Speed;

// This is the maximum speed the motors will be allowed to turn.
// (400 lets the motors go at top speed; decrease to impose a speed limit)
int MAX_SPEED = 140;


/* Accelerometer */
bool onIncline = 0;
bool goingUp = 0;

/* Ultrasonic Sensor */
const int trigPin = A1;
const int echoPin = 13;
long duration, cm;
int looptime = 0;

/* Test */
int maximumPosition = 0;
int minimumPosition = 3000;

/*  Detect Stationary     */
int change = 0;
int totalChange = 0;
int prevError = 0;
int error = 0;
int stationaryCheck = 0;
 
void setup()
{
  /* Accelerometer */
 
  /* LED */
  pinMode(6, OUTPUT);

  /* Ultrasonic Sensor */
  pinMode(trigPin, OUTPUT);
  Serial.begin(9600);
  Wire.begin();
  compass.init();
  compass.enableDefault();

  // Play a little welcome song
  buzzer.play(">g32>>c32");

  // Initialize the reflectance sensors module
  reflectanceSensors.init();

  // Wait for the user button to be pressed and released
  button.waitForButton();

  // Turn on LED to indicate we are in calibration mode
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  // Wait 1 second and then begin automatic sensor calibration
  // by rotating in place to sweep the sensors over the line
  delay(1000);
  int i;
  for(i = 0; i < 80; i++)
  {
    if ((i > 10 && i <= 30) || (i > 50 && i <= 70))
      motors.setSpeeds(-200, 200);
    else
      motors.setSpeeds(200, -200);
    reflectanceSensors.calibrate();

    // Since our counter runs to 80, the total delay will be
    // 80*20 = 1600 ms.
    delay(20);
  }
  motors.setSpeeds(0,0);

  // Turn off LED to indicate we are through with calibration
  digitalWrite(13, LOW);
  buzzer.play(">g32>>c32");

  // Wait for the user button to be pressed and released
  button.waitForButton();

  // Play music and wait for it to finish before we start driving.
  buzzer.play("L16 cdegreg4");
  while(buzzer.isPlaying());
}

void loop()
{
  looptime++;
  stationaryCheck++;

  if (looptime == 100) { // run once every 100 loops
    /* Incline and Accelerometer */
    compass.read();
    changeSpeedBasedOnIncline();

    /* Ultrasonic Sensor */
    ultrasonicSensor();
   
    looptime = 0;
  }

  /* Zumo Shield Code */
  unsigned int sensors[6];

  // Get the position of the line.  Note that we *must* provide the "sensors"
  // argument to readLine() here, even though we are not interested in the
  // individual sensor readings
  int position = reflectanceSensors.readLine(sensors);
 
  // Our "error" is how far we are away from the center of the line, which
  // corresponds to position 2500.
  prevError = error;
  error = position - 2500;
 
  /* Change should be zero if object is stuck */
  change = abs(error) - abs(prevError);
 
  totalChange += change;
 
  // Get motor speed difference using proportional and derivative PID terms
  // (the integral term is generally not very useful for line following).
  // Here we are using a proportional constant of 1/4 and a derivative
  // constant of 6, which should work decently for many Zumo motor choices.
  // You probably want to use trial and error to tune these constants for
  // your particular Zumo and line course.
  int speedDifference = error / 4 + 6 * (error - lastError);

  lastError = error;

  // Get individual motor speeds.  The sign of speedDifference
  // determines if the robot turns left or right.
  m1Speed = MAX_SPEED + speedDifference;
  m2Speed = MAX_SPEED - speedDifference;

  // Here we constrain our motor speeds to be between 0 and MAX_SPEED.
  // Generally speaking, one motor will always be turning at MAX_SPEED
  // and the other will be at MAX_SPEED-|speedDifference| if that is positive,
  // else it will be stationary.  For some applications, you might want to
  // allow the motor speed to go negative so that it can spin in reverse.
  if (m1Speed < 0) {
    m1Speed = 0;
  }
  if (m2Speed < 0) {
    m2Speed = 0;
  }
  if (m1Speed > MAX_SPEED)
    m1Speed = MAX_SPEED;
  if (m2Speed > MAX_SPEED)
    m2Speed = MAX_SPEED;


 
  if (stationaryCheck == 800) { // Check every 800 loops
    rampCheck();
    speedBumpCheck();

    Serial.println(abs(totalChange));
    stationaryCheck = 0;
    totalChange = 0;
  }

  motors.setSpeeds(m1Speed, m2Speed);
}

/* Ultrasonic Sensor */

long microsecondsToCentimeters(long microseconds)
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
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
    //works
        motors.setSpeeds(350, 0);
        delay(450);
        motors.setSpeeds(0, 350);
        delay(500);
    //
//        motors.setSpeeds(350, 0);
//        delay(450);
//        motors.setSpeeds(0, 350);
//        delay(550);    
  }
}

/* Ramp Check */
void rampCheck() {
    /* Stuck up or down a ramp (or up/down a speedbump) */
    if (onIncline && abs(totalChange) < 20) {  // if on incline and reflectance sensor not changing (Stationary)
        m1Speed = 325;
        m2Speed = 325;
        motors.setSpeeds(m1Speed, m2Speed);
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
  }
  else {
    goingUp = false;
  }
 
  if (abs(incline) > 3000) {
    // to prevent an outlier reading, make sure we get at least five positive readings in a row.
    for (int i = 0; i < 10; i++) {
      if (abs(compass.a.x) > 3000) {
        inclineCount++;
      }
    }
    if (inclineCount == 10) {
    digitalWrite(6, HIGH);
    return true;
    }
  }
  else {
    digitalWrite(6, LOW);
    return false;
  }
}

/* Change Speed Based on Incline */
void changeSpeedBasedOnIncline() {

    if (foundIncline()) {
      if (goingUp) {
      MAX_SPEED = 180;
      }
      else {
      MAX_SPEED = 80;
      }
      onIncline = true;
    }
    else {
      MAX_SPEED = 140;
      onIncline = false;
    }

}

/* Ultrasonic Sensor */
void ultrasonicSensor() {
    // establish variables for duration of the ping,
    // and the distance result in inches and centimeters:

    // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
    // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
    digitalWrite(trigPin, LOW);
    delayMicroseconds(1);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(8);
    digitalWrite(trigPin, LOW);

    // Read the signal from the sensor: a HIGH pulse whose
    // duration is the time (in microseconds) from the sending
    // of the ping to the reception of its echo off of an object.
    pinMode(echoPin, INPUT);
    duration = pulseIn(echoPin, HIGH);

    // convert the time into a distance
    cm = microsecondsToCentimeters(duration);

    if (cm < 18) {
//      digitalWrite(6, HIGH);
      Stop();
      delay(100);
      moveRight();
      delay(700);
      moveForward();
      delay(500);
      moveLeft();
      delay(650);
      moveForward();
      delay(300);
      moveLeft();
      delay(500);
      moveForward();
      delay(100);
      linefollowed = 1;
    }
    else {
      moveForward();
    }
//    digitalWrite(6, LOW);
}

/* Motor */

void moveLeft()
{
   motors.setSpeeds(0, 200);
}
void moveForward()
{
  motors.setSpeeds(200,200);
}
void moveRight()
{
    motors.setSpeeds(200, 0);
}
void Stop()
{
  motors.setSpeeds(0,0);
}
