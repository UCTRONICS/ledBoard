#include <SerialCommand.h>

unsigned long trig1Time = 0;
unsigned long trig2Time = 0;

unsigned long trig1HighTime = 0;
unsigned long trig2HighTime = 0;


bool trig1 = false;
bool trig2 = false;


SerialCommand sCmd;     // The demo SerialCommand object
void setup() {
  Serial.begin(9600);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  digitalWrite(6, LOW);
  digitalWrite(7,LOW);
sCmd.addCommand("signal",     setMode);
}
void loop() {
  
 ///SetPinFrequencySafe(ledB, pwmVal_B); 
  sCmd.readSerial();     // We don't do much, just process serial commands 
/************************模拟信号源************************************/
  
  if (trig1 || trig2) {
    if (trig1) {
      if ( micros() - trig1Time >= 100000) {
        trig1Time = micros();
        trig1HighTime = micros();
        digitalWrite(6, HIGH);digitalWrite(6, HIGH);   
      }
      if (micros() - trig1HighTime >= 10000) {
        digitalWrite(6, LOW);digitalWrite(6, LOW);
      }
    }
    if (trig2) {
      if ( micros() - trig2Time >= 100000) {
        trig2Time = micros();
        trig2HighTime = micros();
        digitalWrite(7, HIGH);digitalWrite(7, HIGH);
      }
      if (micros() - trig2HighTime >= 10000) {
        digitalWrite(7, LOW);digitalWrite(7, LOW);
      }
    }
  }
/********************************************************************/
  
}
void setMode() {
  char *arg;
  arg = sCmd.next();
  if (arg != NULL) {
    if (!strcmp(arg, "10")) {
      Serial.println("Set the signal to 10");
      trig1 = true;
      trig2 = false;  
      trig1Time  = micros();
    } else if (!strcmp(arg, "11")) {
      Serial.println("Set the signal to 11");
      trig1 = true;
      trig2 = true;
      trig1Time  = micros();
      trig2Time  = micros();

    } else if (!strcmp(arg, "01")) {
      trig2 = true;
      trig1 = false;
       Serial.println("Set the signal to 01");
      trig2Time  = micros();
    }
  }

}
