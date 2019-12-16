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

ZumoBuzzer buzzer;
ZumoReflectanceSensorArray reflectanceSensors;
ZumoMotors motors;
Pushbutton button(ZUMO_BUTTON);
int lastError = 0;

// This is the maximum speed the motors will be allowed to turn.
// (400 lets the motors go at top speed; decrease to impose a speed limit)
const int MAX_SPEED = 150;
int gapCounter = 0;
int maxGap = 400;

/* Gap */
unsigned long previousMillis;
unsigned long currentMillis;
unsigned long timeElapsed;
int errorGap;
int error;
int position;
unsigned int sensors[6]; //make global for others too


void setup()
{
  Serial.begin(9600);
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

  // Get the position of the line.  Note that we *must* provide the "sensors"
  // argument to readLine() here, even though we are not interested in the
  // individual sensor readings
  position = reflectanceSensors.readLine(sensors);

  // Our "error" is how far we are away from the center of the line, which
  // corresponds to position 2500.
  error = position - 2500;
//  Serial.print("error:  ");
//  Serial.println(error);



  /* GAP LOGIC */
  gap();
  
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
  int m1Speed = MAX_SPEED + speedDifference;
  int m2Speed = MAX_SPEED - speedDifference;

  // Here we constrain our motor speeds to be between 0 and MAX_SPEED.
  // Generally speaking, one motor will always be turning at MAX_SPEED
  // and the other will be at MAX_SPEED-|speedDifference| if that is positive,
  // else it will be stationary.  For some applications, you might want to
  // allow the motor speed to go negative so that it can spin in reverse.
  if (m1Speed < 0)
    m1Speed = 0;
  if (m2Speed < 0)
    m2Speed = 0;
  if (m1Speed > MAX_SPEED)
    m1Speed = MAX_SPEED;
  if (m2Speed > MAX_SPEED)
    m2Speed = MAX_SPEED;

  motors.setSpeeds(m1Speed, m2Speed);
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
      int multiplierGoingBack = 3*maxGap; // constant times edge of gapCounter
      /* determine if to go left or right */
      if (error > 0) { // turned right
        // make it turn left backwards
          motors.setSpeeds(-MAX_SPEED,0);
          delay(timeElapsed);
          motors.setSpeeds(0,0);
      }
      else if (error < 0) { // turned left
        // make it turn right backwards
          motors.setSpeeds(0,-MAX_SPEED);
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
