#define S0 4
#define S1 5
#define S2 6
#define S3 3
#define sensor1Out 8
#define sensor2Out 9
int frequency_b1 = 0;
int frequency_b2 = 0;
int frequency_g1 = 0;
int frequency_g2 = 0;

void setup() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensor1Out, INPUT);
  pinMode(sensor2Out, INPUT);
  digitalWrite(S0,HIGH);   // Setting frequency-scaling to 20%
  digitalWrite(S1,LOW);   // Setting frequency-scaling to 20%
  Serial.begin(9600);
}
void loop() {
  int g1=frequency_g1;
  int b1=frequency_b1;  
  int g2=frequency_g2;
  int b2=frequency_b2;
  int result1=0;
  int result2=0;
  
  // Green 1 
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
  frequency_g1 = pulseIn(sensor1Out, LOW);

  Serial.print("G1= "); //printing name
  Serial.print(frequency_g1); //printing RED color frequency
  Serial.print("  ");
  delay(1000);
  
  // Blue 1
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  frequency_b1 = pulseIn(sensor1Out, LOW);
 
  Serial.print("B1= "); //printing name
  Serial.print(frequency_b1); //printing RED color frequency
  Serial.println("  ");
  delay(1000);

  // Color result 1  
  if(g1<140 & g1>50 & b1>65){
    Serial.println("Green 1");
    result1=1;
  }
  
  // Green 2
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
  frequency_g2 = pulseIn(sensor2Out, LOW);

  Serial.print("G2= ");
  Serial.print(frequency_g2);
  Serial.print("  ");
  delay(1000);
    
  // Blue 2 
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  frequency_b2 = pulseIn(sensor2Out, LOW);

  Serial.print("B2= ");
  Serial.print(frequency_b2);
  Serial.println("  ");
  delay(1000);

  // Color result 2
  if(g2<140 & g2>50 & b2>65){
   Serial.println("Green 2");
   result2=1;
  }
}
