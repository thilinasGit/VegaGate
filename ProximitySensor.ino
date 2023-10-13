#include <EEPROM.h>
#include <MovingAverage.h>

// Define Trig and Echo pin:
#define trigPin 2       //// ultrasonic trigger pin
#define echoPin 4
#define gateRelay 10
MovingAverage<int> avr(5, 2);
// Define variables:
long duration,gateRelayTimeout;
int distance;
const int waitTime = 50000;
int calibDist;
const int calibBtn = 9;
const int led = 13;
const int camRelay = 6;   ////////// !!!!!!! Drop the voltage to 3.3V
byte longPress = 0; /// to capture button press n hold
int inRangeCount=0;
bool gateClosedFlag=false;
int thresholdDist=280;


void setup() {
  // Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(camRelay, OUTPUT);
  pinMode(gateRelay, OUTPUT);
  pinMode(calibBtn, INPUT_PULLUP);
  pinMode(led, OUTPUT);
   calibDist = fetchIntFromEEPROM(11);
  //  if (calibDist < 0)calibDist = 800;
  Serial.println("Ultrasonic range sensor");
  Serial.println("Firmware version 1.2");
  Serial.println("2023.08.04");
  Serial.println("Developer email mediathilina@gmail.com");
  
  //calibDist = 350;
  Serial.begin(115200);
  Serial.println("Output in cm");
  Serial.print("Calib dist - ");
  Serial.println(calibDist);
  
}

void loop() {
  // Clear the trigPin by setting it LOW:
  digitalWrite(trigPin, LOW);              
  digitalWrite(led, HIGH);
  longPress = 0;                /// do not shift
  delayMicroseconds(5);

  duration = getDuration();
  if (duration == 0)duration = 22500;

  /// residue echos may saturate the sensor if sampled fast, This delay is suitable detecting objects longer than 1.5m with max speed of 40kmph
  // Calculate dist:
  avr.push(duration * 0.0175);
  distance = avr.get();
  Serial.println(distance);

  inRangeCount=0;
  if (distance > 0)            //// 0 is invalid
  if (distance < thresholdDist){
        digitalWrite(camRelay, HIGH);
        digitalWrite(led, LOW);
        delay(50);
        digitalWrite(led, HIGH);     
        
    while (distance < thresholdDist) {           /// hold trigger
      inRangeCount++;                                    
      digitalWrite(led, HIGH);
      if(inRangeCount%500==0){
        digitalWrite(camRelay, HIGH);
        delay(300);
        digitalWrite(camRelay, LOW);  
      }
      duration = getDuration();
      if (duration == 0)duration = 22500;
      avr.push(duration * 0.0175);
      distance = avr.get();
      Serial.println(distance);
      digitalWrite(led, LOW);
      delay(10);
    }
    
    digitalWrite(camRelay, LOW);   ///// release the cam trigger on vehicle exit
    gateClosedFlag=false; 
    gateRelayTimeout=10000+millis(); //////////timeout set to 5 seconds in advance  
    } 
    btnStuff();

  if(millis()>=gateRelayTimeout&&!gateClosedFlag){
    digitalWrite(gateRelay, HIGH); 
    delay(150);
    digitalWrite(gateRelay, LOW); 
    gateClosedFlag=true;
  }
  /// status led
  
  digitalWrite(led, LOW);
  delay(10);
}

long getDuration() {
  // Trigger the sensor by setting the trigPin high for 10 microseconds:
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the echoPin. pulseIn() returns the duration (length of the pulse) in microseconds:
  return pulseIn(echoPin, HIGH, 22500);
}

void decideAction(int d) {
  if (d > 0)            //// 0 is invalid
    if (d < calibDist - ( calibDist * 0.1)) {
      digitalWrite(camRelay, HIGH);
      for (int c = 0; c < 2; c++) {
        digitalWrite(led, HIGH);
        delay(50);
        digitalWrite(led, LOW);
        delay(50);
      }
      digitalWrite(camRelay, LOW);
      digitalWrite(led, HIGH);
      delay(1000);

    }
}

void btnStuff() {
  while (!digitalRead(calibBtn) && longPress < 20) {
    longPress++;
    digitalWrite(led, HIGH);
    delay(50);
    digitalWrite(led, LOW);
    delay(50);

  }
  if (longPress >= 20) {
    saveIntToEEPROM(distance, 11);
    calibDist = distance;
    Serial.print("Calibration done ");
    Serial.print(distance);
    Serial.print(" cm");
    for (int c = 0; c < 3; c++) {
      digitalWrite(led, HIGH);
      delay(500);
      digitalWrite(led, LOW);
      delay(500);
    }
  }
}

void saveIntToEEPROM(int value, int address) {
  EEPROM.put(address, value);
}

int fetchIntFromEEPROM(int address) {
  int value;
  EEPROM.get(address, value);

  if (value == -1 || value == 255) {
    return -1; // No value has been stored before or EEPROM location is in default state
  } else {
    return value;
  }
}
