#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define SYNC_PIN 18
#define PULSE_PIN_UP 11
#define PULSE_PIN_RIGHT 12
#define PULSE_PIN_LEFT 13
#define PULSE_START 8
#define PULSE_END 9
#define LED_PIN     6
#define LED_COUNT   300
#define SENSOR1 2
#define SENSOR2 3
#define SENSOR_DISTANCE 10 //centimetres
#define LEFT 1
#define RIGHT 0
#define EEG_TRIGGER_SEND 1
#define RED 0
#define GREEN 1
#define LED_START 117 // shifted by 1 left, 59->40, now changed because of switch to 60led/m
#define LED_END 212 // 104->88

#define LED_START_60
#define LED_END_60
  
// sensorFlags and LED selection
volatile int ledSelection;
volatile int colourSelection;
volatile int Sensor1Flag = 0;
volatile int Sensor2Flag = 0;
volatile int m = 0;

//timing variables
volatile long Flag1Time;
volatile long Flag2Time;
volatile int directionFlag = 3; // this is the neutral state, where it is neither left nor right

//randomization variables and initialization
int pumpArray[5];  // create an array of length 10 to store 0s
int kickturnArray[5];   // create an array of length 10 to store 1s
int taskArray1[10];
int taskArray2[10];
int combinedArray[20];

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void arrayStart(){

// this code is for producing the arrays for each task
for (int i = 0; i < 5; i++) {
  pumpArray[i] = 0;
}

// initialize the onesArray with 1s
for (int i = 0; i < 5; i++) {
  kickturnArray[i] = 1;
}
// concatenate the two arrays
for (int i = 0; i < 5; i++) {
  taskArray1[i] = pumpArray[i % 5];
  taskArray1[i + 5] = kickturnArray[i % 5];
  taskArray2[i] = pumpArray[i % 5];
  taskArray2[i + 5] = kickturnArray[i % 5];

}


// shuffle the concatenated array for combined array
for (int i = 0; i < 10; i++) {
  int j;
  do {
    j = random(10);
  } while (j == i);
  int temp = taskArray1[i];
  taskArray1[i] = taskArray1[j];
  taskArray1[j] = temp;
}

for (int i = 0; i < 10; i++) {
  int k;
  do {
    k = random(10);
  } while (k == i);
  int temp = taskArray2[i];
  taskArray2[i] = taskArray2[k];
  taskArray2[k] = temp;
}



for (int i = 0; i < 10; i++)
{
  combinedArray[i*2] = taskArray1[i];
  combinedArray[i*2+1] = taskArray2[i];
}





}

void printArray(int arr[], int size) {
  for (int i = 0; i < size; i++) {
    Serial.println(arr[i]);
  }
}



void startLights(){

  for(int i =LED_START; i <LED_END; i++){
    strip.setPixelColor(i, strip.Color(0, 255, 0)); // Set the color of the current LED to red
    strip.show(); // Update the strip with the new color
   // delay(1); // Delay for a short period to create a visual effect
    
  }
delay(500);
  for(int i =LED_END; i > LED_START-1; i--){
    strip.setPixelColor(i, strip.Color(0, 0, 0)); // turns off LED
    strip.show(); // Update the strip with the new color
    delay(1); // Delay for a short period to create a visual effect
    
  }
}

void endLights(){

    for(int i = LED_START; i < LED_END; i++){
    strip.setPixelColor(i, strip.Color(255, 0, 0)); // Set the color of the current LED to red
    strip.show(); // Update the strip with the new color
}
delay(500); // Delay for a short period to create a visual effect

}


void directionCheck(){
  long deltaTime = (Flag1Time-Flag2Time);
 // Serial.print("Delta Time is ");
  //Serial.println(deltaTime);
  if (deltaTime > 0)
  {
    directionFlag = RIGHT;
  }
  else {
    directionFlag = LEFT;
  }
  
}

 
// CERTIFIED WORKING
void ledTown(){
Serial.print("LED CHANGE,");
//turn off current LED First 
strip.setPixelColor(ledSelection, 0, 0, 0);  // turn off the selected LED
strip.show();   // update the strip with the new LED color

// choose colour according to array

colourSelection = combinedArray[m];
  
ledSelection = random(LED_START,LED_END);
Serial.print(ledSelection);


if (colourSelection == RED)
{
  strip.setPixelColor(ledSelection, 255, 0, 0); 
  Serial.println(",RED");

} else if (colourSelection == GREEN) {
  strip.setPixelColor(ledSelection, 0, 255, 0);
  Serial.println(",GREEN");
}

//Serial.println(m);
strip.show();
}

void firstFlagSet(){
Sensor1Flag = 1;
//Serial.println("Switch 1 Triggered");
Flag1Time = micros();
//Serial.println(Flag1Time);
}

void lastFlagSet(){
  Sensor2Flag = 1;
 // Serial.println("Switch 2 Triggered");
  Flag2Time = micros();
 // Serial.println(Flag2Time);
}

void syncInterrupt(){
  Serial.println("START");
}

void pulseSendStart(){
  digitalWrite(PULSE_START, HIGH);
  delay(5);
  digitalWrite(PULSE_START, LOW);
}

void pulseSendEnd(){
  digitalWrite(PULSE_END, HIGH);
  delay(5);
  digitalWrite(PULSE_END, LOW);
}

void setup() {
  Serial.begin(9600);
  Serial.println("ABI SKATEBOARD EXPERIMENT");
  randomSeed(analogRead(0));
  directionFlag = 3;
  arrayStart();

  pinMode(SYNC_PIN, INPUT_PULLUP); // set interrupt pin as input with internal pull-up resistor
  attachInterrupt(4, syncInterrupt, FALLING); // attach interrupt 4 to the pin and set it to trigger on a falling edge
  attachInterrupt(digitalPinToInterrupt(SENSOR1), firstFlagSet,FALLING);
  attachInterrupt(digitalPinToInterrupt(SENSOR2), lastFlagSet,FALLING);

  strip.begin();
  strip.show();
  startLights();

  printArray(combinedArray,20);

  pinMode(A0, INPUT); // for random noise input
  pinMode(PULSE_PIN_RIGHT,OUTPUT);
  pinMode(PULSE_PIN_LEFT,OUTPUT);
  pinMode(PULSE_START, OUTPUT);
  pinMode(PULSE_PIN_UP, OUTPUT);

  pulseSendStart();

}

void pulseSend(int pulseSelection){
  digitalWrite(pulseSelection, HIGH); // Set the pin to a high signal
  delay(5); // Wait for 1 ms
  digitalWrite(pulseSelection, LOW); // Set the pin back to a low signal
}

void pulseSendRight(){
  digitalWrite(PULSE_PIN_RIGHT, HIGH); // Set the pin to a high signal
  delay(5); // Wait for 1 ms
  digitalWrite(PULSE_PIN_RIGHT, LOW); // Set the pin back to a low signal
}

void pulseSendLeft(){
  digitalWrite(PULSE_PIN_LEFT, HIGH); // Set the pin to a high signal
  delay(5); // Wait for 1 ms
  digitalWrite(PULSE_PIN_LEFT, LOW); // Set the pin back to a low signal
}



void loop() {
  


  if (Flag1Time > 0 && Flag2Time > 0)
  {
    directionCheck();
  }
  
  if (directionFlag == RIGHT)
  {
  //Serial.println("RIGHT");
    //if (Sensor1Flag == 1){ // this will be channed for direction 
    pulseSend(PULSE_PIN_UP);
    ledTown();
    //Serial.println(",Right");
    delay(500); // this delay function is to debounce the event
    m++;
    Flag1Time = 0;
    Flag2Time = 0;
 
    directionFlag = 3;
  //Sensor1Flag = 0;
  //Sensor2Flag = 0;


    // flags are to be reset once the skateboard has cleared the reverse direction
    //}
  }

    // if first flag is set first, do nothing and reset flags
  
  if (directionFlag == LEFT)
  {
    Serial.println("LEFT");
    if (colourSelection == RED)
    {
      pulseSendRight();
     // Serial.println("RED");
    }
    else if (colourSelection == GREEN)
    {
      pulseSendLeft();
     // Serial.println("GREEN");
    }
    
    
    delay(500); // this delay function is to debonuce the event
    Flag1Time = 0;
    Flag2Time = 0;
    //Sensor1Flag = 0;
    //Sensor2Flag = 0;
    directionFlag = 3;
    
  }


  if (m > 20)
  {
    //pulseSendEnd();
    noInterrupts();
    endLights();

    while (m>20)
    {
      /* code */
    }
    
  }
  



  }

