#include <Wire.h> // I2C Library
#include <Adafruit_TCS34725.h> // TCS34725 Library
#define ADDR_1 0x10
#define ADDR_2 0x20 // (Me)

/* I2C */
enum { // Commands from Hub
  COLOUR_L = 1,
  COLOUR_R  = 2
};

#define S0 4
#define S1 5
#define S2 6
#define S3 3
#define sensor1Out 9
#define sensor2Out 8
int b1 = 0;
int b2 = 0;
int r1 = 0;
int r2 = 0;
int g1 = 0;
int g2 = 0;

// Serial Plotter
int LeftDetected = 0;
int RightDetected = 0;
int loopCount = 0;

char command;
int result1=0;
int result2=0;

void setup() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensor1Out, INPUT);
  pinMode(sensor2Out, INPUT);
  digitalWrite(S0,HIGH);   // Setting frequency-scaling to 20%
  digitalWrite(S1,LOW);   // Setting frequency-scaling to 20%
  /* Serial */
   Serial.begin(9600);
  /* I2C */
  command = 0;
  Wire.begin(ADDR_2);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
}

void loop() { 
  /* plotting left and right green square */
//  loopCount++;
//  if (loopCount == 20) {
//    loopCount = 0;
//  Serial.print("LeftDetected:");
//  Serial.print(LeftDetected);
//  Serial.print(", ");
//  Serial.print("RightDetected:");
//  Serial.println(RightDetected);
//  LeftDetected = 0;
//  RightDetected = 0;
//  }


  
  // Green 1
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
  g1 = pulseIn(sensor1Out, LOW);
  
//  Serial.print("G1= ");
//  Serial.print(g1); 
//  Serial.print("  "); 
//  
  // Blue 1
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  b1 = pulseIn(sensor1Out, LOW);
  
//  Serial.print("B1= ");
//  Serial.print(b1);  
//  Serial.print("  "); 

  // Red 1
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);
  r1 = pulseIn(sensor1Out, LOW);

//  Serial.print("R1= ");
//  Serial.print(r1);
//  Serial.print("  "); 
//  Serial.println("  ");
//  delay(500);

  // Color result 1  
  if(g1>=70 & g1<=100 & r1>=70 & r1<=120 & b1>=70){
    Serial.println("Green 1");
    LeftDetected++;
    result1=1;
  } else {
    result1=0;
  }

  // Green 2
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
  g2 = pulseIn(sensor2Out, LOW);
  
//  Serial.print("G2= ");
//  Serial.print(g2);
//  Serial.print("  ");
//    
  // Blue 2
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  b2 = pulseIn(sensor2Out, LOW);
  
//  Serial.print("B2= ");
//  Serial.print(b2);
//  Serial.print("  ");
//     
  // Red 2
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);
  r2 = pulseIn(sensor2Out, LOW);
//  
//  Serial.print("R2= ");
//  Serial.print(r2); 
//   Serial.print("  "); 
//   Serial.println("  ");
//   delay(500);   

  // Color result 2
  if(g2>=70 & g2<=100 & r2>=70 & r2<=120 & b2>=70){
    Serial.println("Green 2");
    RightDetected++;
    result2=1;
  } else {
    result2=0;
  }
}

void receiveEvent (int howMany) {
  command = Wire.read();
}

void requestEvent() {
  switch (command) {
     case COLOUR_L: Wire.write(result1); break;
     case COLOUR_R: Wire.write(result2); break;
  }
}
