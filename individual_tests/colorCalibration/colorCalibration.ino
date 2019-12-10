#define S0 4
#define S1 5
#define S2 6
#define S3 3
#define sensor1Out 8
#define sensor2Out 9

void setup() {
  /* Serial */
  Serial.begin(9600);
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensor1Out, INPUT);
  pinMode(sensor2Out, INPUT);
  digitalWrite(S0,HIGH); 
  digitalWrite(S1,LOW);
  Serial.println("Ready. Enter 'G' to continue.");
  waitChar('G');
  
  // Red
  setRed();
  colorRange();
  Serial.println("Red done! Enter 'G' to continue.");
  waitChar('G');

  // Green
  setGreen();
  colorRange();
  Serial.println("Green done! Enter 'G' to continue.");
  waitChar('G');

  // Blue
  setBlue();
  colorRange();
  Serial.println("Blue done! Enter 'G' to continue.");
  waitChar('G');
}

void loop() {
  Serial.println("Loop.");
}

void setRed() {
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);
}

void setGreen() {
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
}

void setBlue() {
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
}

void waitChar(char C) {
  while(1) {
    if(Serial.available() > 0) {
      if(Serial.read() == (unsigned char) C)
        break;
    }
  }
}

void sortAscend(int arr[], int arrSize) {
  int i, j, k;
  for(i=0; i<arrSize; i++) {   
    for(j=i+1; j<arrSize; j++) {
      if(arr[i] > arr[j]) {
        k  = arr[i];
        arr[i] = arr[j];
        arr[j] = k;
      }
    }
  }
}

void colorRange() {
  for (int i = 0; i < 5; i++) {
    Serial.println("Reading...");
    int reading[500];
    for (int j = 0; j < 500; j++) {
      reading[j] = pulseIn(sensor1Out, LOW);
      Serial.println(reading[j]);
    }
    sortAscend(reading, 500);
    Serial.println("Maximum: ");
    Serial.println(reading[499]);
    Serial.println(" Minimum: ");
    Serial.println(reading[0]);
    Serial.println("Enter 'G' to continue.");
    waitChar('G');
  }
}
