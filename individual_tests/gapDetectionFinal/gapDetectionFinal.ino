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

#include <ZumoShield.h>

ZumoBuzzer buzzer;
ZumoReflectanceSensorArray reflectanceSensors;
ZumoMotors motors;
Pushbutton button(ZUMO_BUTTON);
int lastError = 0;

// This is the maximum speed the motors will be allowed to turn.
// (400 lets the motors go at top speed; decrease to impose a speed limit)
const int MAX_SPEED = 200;
int gapCounter = 0;
int maxGap = 250; // subject to change
int MIN_SPEED = - 100;

#define TURN_BASE_SPEED 100
#define CALIBRATION_SAMPLES 70  // Number of compass readings to take when calibrating
#define CRB_REG_M_2_5GAUSS 0x60 // CRB_REG_M value for magnetometer +/-2.5 gauss full scale
#define CRA_REG_M_220HZ    0x1C // CRA_REG_M value for magnetometer 220 Hz update rate
#define DEVIATION_THRESHOLD 5

/* Accelerometer */
LSM303 compass;
bool onIncline = 0;
bool goingUp = 0;

/* Gap */
unsigned long previousMillis;
unsigned long currentMillis;
unsigned long timeElapsed;
unsigned long newTimeElapsed = 0;
bool blackFound = 0;
int notBlackFoundCounter = 0;
int errorGap;
int error;
//int position;
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
  /* GAP LOGIC */
  gap();

  
  // Get the position of the line.  Note that we *must* provide the "sensors"
  // argument to readLine() here, even though we are not interested in the
  // individual sensor readingsv
  position = reflectanceSensors.readLine(sensors);

  // Our "error" is how far we are away from the center of the line, which
  // corresponds to position 2500.
  error = position - 2500;
//  Serial.print("error:  ");
//  Serial.println(error);





  
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
    m1Speed = MIN_SPEED;
  if (m2Speed < 0)
    m2Speed = MIN_SPEED;
  if (m1Speed > MAX_SPEED)
    m1Speed = MAX_SPEED;
  if (m2Speed > MAX_SPEED)
    m2Speed = MAX_SPEED;

  motors.setSpeeds(m1Speed, m2Speed);
}

void gap() {
  if (abs(error) == 2500) { // detects white
    if (gapCounter == 0) { // calculate first Time
      previousMillis = millis(); // millis() returns an unsigned long.
    }
    gapCounter++;
    
    if (gapCounter > maxGap) { // detected a Gap
//      Serial.println("greater max gap");

      gapCounter = 0;
      currentMillis = millis(); // grab current time
      timeElapsed = currentMillis - previousMillis;  
//      Serial.println(timeElapsed);
      motors.setSpeeds(0,0);
      delay(500);
      
      /* Go back to edge of black line*/
      
      /* determine if to go left or right */
      if (error > 0) { // turned right
//          Serial.println("error > 0");
        // make it turn left backwards
          motors.setSpeeds(-MAX_SPEED,-MIN_SPEED);
          delay(timeElapsed);
          motors.setSpeeds(0,0); 
          delay(200);
          motors.setSpeeds(100,100);
          delay(300);
          motors.setSpeeds(0,0); 
          delay(200);

          position = reflectanceSensors.readLine(sensors);
          errorGap = position - 2500;
          
          /* Check other direction as well */
//            Serial.println("check other direction");
//            Serial.print("newTimeElapsed: ");
//            Serial.print(newTimeElapsed);
//            Serial.print("abs(errorGap): ");
//            Serial.print(abs(errorGap));

          previousMillis = millis(); // millis() returns an unsigned long.
            while (newTimeElapsed < timeElapsed &&  abs(errorGap) == 2500){ // while on white, keep going left
//              Serial.println("in while loop");
//              Serial.print("newTimeElapsed: ");
//              Serial.print(newTimeElapsed);
//              Serial.print("abs(errorGap): ");
//              Serial.print(abs(errorGap));
              
              currentMillis = millis(); // grab current time
              newTimeElapsed = currentMillis - previousMillis;
              motors.setSpeeds(MIN_SPEED, MAX_SPEED);
              position = reflectanceSensors.readLine(sensors);
              errorGap = position - 2500;
          }
          motors.setSpeeds(0, 0);
          delay(500);

          if (abs(errorGap) != 2500){
            blackFound = true;
          }
          else {
            // make it turn right backwards
            motors.setSpeeds(-MIN_SPEED,-MAX_SPEED);
            delay(timeElapsed);
            motors.setSpeeds(0,0); 
            delay(200);
            motors.setSpeeds(100,100);
            delay(300);
            motors.setSpeeds(0,0); 
            delay(200);
          }
          
      }
      else if (error < 0) { // turned left
        // make it turn right backwards
        motors.setSpeeds(-MIN_SPEED,-MAX_SPEED);
        delay(timeElapsed);
        motors.setSpeeds(0,0); 
        delay(200);
        motors.setSpeeds(100,100);
        delay(300);
        motors.setSpeeds(0,0); 
        delay(200);
       /* Check other direction as well */

        position = reflectanceSensors.readLine(sensors);
        errorGap = position - 2500;

        previousMillis = millis(); // millis() returns an unsigned long.
        while (newTimeElapsed < timeElapsed && abs(errorGap) == 2500){ // while on white, keep going left
          currentMillis = millis(); // grab current time
          newTimeElapsed = currentMillis - previousMillis;
          motors.setSpeeds(MAX_SPEED, MIN_SPEED);
          position = reflectanceSensors.readLine(sensors);
          errorGap = position - 2500;
        }
         motors.setSpeeds(0, 0);
         delay(500);
         
        if (abs(errorGap) != 2500){
          blackFound = true;
        }
        else {
          // make it turn left backwards
            motors.setSpeeds(-MAX_SPEED,-MIN_SPEED);
            delay(timeElapsed);
            motors.setSpeeds(0,0);
            delay(200);
            motors.setSpeeds(100,100);
            delay(300);
            motors.setSpeeds(0,0); 
            delay(200);
          }
      }
       
       /* Go Forward Slightly */
       motors.setSpeeds(100,100);      
       delay(300);

      if (!blackFound) { // Gap detected, go straight
        notBlackFoundCounter++;
        if (notBlackFoundCounter > 2){
          maxGap = 600;        
        }
        
        position = reflectanceSensors.readLine(sensors);
        errorGap = position - 2500;

        while (abs(errorGap)==2500){ // while on white, keep going forward
          motors.setSpeeds(150, 150);
          position = reflectanceSensors.readLine(sensors);
          errorGap = position - 2500;
        }
      }
      
      gapCounter = 0;
      blackFound = false;
      newTimeElapsed = 0;
    }
  }
  else {
    gapCounter = 0;
    blackFound = false;
    newTimeElapsed = 0;
  }
}
