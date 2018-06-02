

#include <SerialCommand.h>
#include <SoftwareSerial.h>

/****Define board address bit***/
#define addressBit0 4
#define addressBit1 5
#define addressBit2 6
#define addressBit3 7
/***************************/

#define RS485_DE_RE  8

/***********Define trigger type *******/
#define triggerType_1x   0
#define triggerType_10   1
#define triggerType_x1   2
#define triggerType_01   3
#define triggerType_11   4
#define triggerType_00   5
#define triggerType_xx   6
/***************************************/

/************Define led channel*********/
#define ledA  10
#define ledB 9
/***************************************/
static uint8_t  address = 0;
static uint8_t  getAddress = 0;
static uint8_t  signalState = 0xFF;

static uint16_t IntensityVal_A = 50;
static uint16_t IntensityVal_B = 50;
static uint16_t pwmVal_A = 100;
static uint16_t pwmVal_B = 100;
static uint16_t durationVal_A = 5000;
static uint16_t durationVal_B = 5000;
static uint16_t delayVal_A = 5000;
static uint16_t delayVal_B = 5000;
static uint8_t setTriggerType_A = triggerType_xx;
static uint8_t  setTriggerType_B = triggerType_xx;
unsigned long timeStartA = 0;
unsigned long timeStartB = 0;
unsigned long durationTimeStartA = 0;
unsigned long durationTimeStartB = 0;

/********define status flag bit**************/
bool setCHA = false;
bool setCHB = false;
bool hander1Flag = false;
bool hander2Flag = false;
bool checkDelayCHA = false;
bool checkDelayCHB = false;
bool setAOK = false;
bool setBOK = false;
bool runAstop = true;
bool runBstop = true;
/********************************************/

SerialCommand sCmd;     // The demo mySerialCommand object
SoftwareSerial mySerial(11, 12); // RX, TX
void setup() {
  Serial.begin(9600);
  pinMode(ledA, OUTPUT);
  pinMode(ledB, OUTPUT);
  digitalWrite(ledA, HIGH);
  digitalWrite(ledB, HIGH);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(RS485_DE_RE, OUTPUT);
  digitalWrite(RS485_DE_RE , LOW); delay(5);  // Enable receive
  // set the data rate for the SoftwaremySerial port
  mySerial.begin(4800);
  getAddress = autoGetAddress();
  mySerial.println(getAddress, DEC);
  sCmd.addCommand("#",     processCommand);
}
void loop() {
  sCmd.readSerial();     // We don't do much, just process mySerial commands
  if ( hander1Flag | hander2Flag) {
    if (checkDelayCHA) {
      if ( micros() - timeStartA >= delayVal_A ) {
        checkDelayCHA = false;
        if ( signalState == triggerType_10) {
          if (setTriggerType_A == triggerType_10 || setTriggerType_A == triggerType_1x ) {
            analogWrite(ledA, IntensityVal_A);
            durationTimeStartA = micros();
          }
        } else if (signalState == triggerType_11 ) {
          if ( setTriggerType_A == triggerType_1x || setTriggerType_A == triggerType_11) {
            analogWrite(ledA, IntensityVal_A);
            durationTimeStartA = micros();
          }
        } else if (signalState == triggerType_01) {
          if (setTriggerType_A == triggerType_01 || setTriggerType_A == triggerType_x1 ) {
            analogWrite(ledA, IntensityVal_A);
            durationTimeStartA = micros();
          }
        }
        setAOK = true; runAstop = false;
      }
    }
    if (checkDelayCHB) {
      if ( micros() - timeStartB >= delayVal_B ) {
        checkDelayCHB = false;
        setBOK = true;
        if (signalState == triggerType_10) {
          if (setTriggerType_B == triggerType_10 || setTriggerType_B == triggerType_1x ) {
            analogWrite(ledB, IntensityVal_B);
            durationTimeStartB = micros();
          }
        } else if (signalState == triggerType_11) {
          if ( setTriggerType_B == triggerType_1x || setTriggerType_B == triggerType_11) {
            analogWrite(ledB, IntensityVal_B);
            durationTimeStartB = micros();
          }
        } else if (signalState == triggerType_01) {
          if (setTriggerType_B == triggerType_01 || setTriggerType_B == triggerType_x1 ) {
            analogWrite(ledB, IntensityVal_B);
            durationTimeStartB = micros();
          }
        }
        setBOK = true; runBstop = false;
      }
    }
    if (setAOK && setBOK) {
      setAOK = false;
      setBOK = false;
      hander1Flag = 0;
      hander2Flag = 0;
    }
  }
  if (!runAstop) {
    if ( micros() - durationTimeStartA >= durationVal_A) {
      runAstop = true;
      if (setTriggerType_A != triggerType_00 )
        analogWrite(ledA, 255);
      if (runAstop && runBstop) {
        attachInterrupt(digitalPinToInterrupt(2), &handler1, RISING);
        attachInterrupt(digitalPinToInterrupt(3), &handler2, RISING);
      }
    }
  }
  if (!runBstop) {
    if ( micros() - durationTimeStartB >= durationVal_B) {
      runBstop = true;
      if (setTriggerType_B != triggerType_00 )
        analogWrite(ledB, 255);
      if (runAstop && runBstop) {
        attachInterrupt(digitalPinToInterrupt(2), &handler1, RISING);
        attachInterrupt(digitalPinToInterrupt(3), &handler2, RISING);
      }
    }
  }
}
void processCommand() {
  char *arg;
  char *argTemp;
  int16_t getVal = 0;
  static uint16_t IntensityVal_Temp_A, IntensityVal_Temp_B;
  arg = sCmd.next();
  if (arg != NULL) {
    address = atoi(arg);    //get the address
    if (address != getAddress)
      if (address != 99)
        return;
  }
  argTemp = sCmd.next();  //get the next value
  arg = sCmd.next();
  if (arg != NULL) {
    getVal = atoi(arg);    //get the next value
    if (argTemp != NULL) {
      if (!strcmp(argTemp, "ai")) {
        setCHA = true; IntensityVal_A = 255 - (uint8_t)((getVal * 255) / 100.0);
        IntensityVal_Temp_A = getVal;
      } else if (!strcmp(argTemp, "bi")) {
        setCHB = true; IntensityVal_B = 255 - (uint8_t)((getVal * 255) / 100.0);
        IntensityVal_Temp_B = getVal;
      } else if (!strcmp(argTemp, "ar")) {
        setCHA = true; durationVal_A = getVal;
      } else if (!strcmp(argTemp, "br")) {
        setCHB = true; durationVal_B = getVal;
      } else if (!strcmp(argTemp, "ad")) {
        setCHA = true; delayVal_A = getVal;
      } else if (!strcmp(argTemp, "bd")) {
        setCHB = true; delayVal_B = getVal;
      } else {
        mySerial.println(F("set error!"));
        return;
      }
    }
  }
  arg = sCmd.next();
  if (arg != NULL) {
    if (!strcmp(arg, "at")) {
      if (setCHB) {
        mySerial.println(F("set error!"));
        return;
      }
      arg = sCmd.next();
      if (!strcmp(arg, "1x")) {
        setTriggerType_A = triggerType_1x;
      } else if (!strcmp(arg, "10")) {
        setTriggerType_A = triggerType_10;
      } else if (!strcmp(arg, "x1")) {
        setTriggerType_A = triggerType_x1;
      } else if (!strcmp(arg, "01")) {
        setTriggerType_A = triggerType_01;
      } else if (!strcmp(arg, "11")) {
        setTriggerType_A = triggerType_11;
      } else if (!strcmp(arg, "00")) {
        setTriggerType_A = triggerType_00;
      } else if (!strcmp(arg, "xx")) {
        setTriggerType_A = triggerType_xx;
      } else {
        mySerial.println(F("set error")); return;
      }
      mySerial.println(F("Set the channel_A "));
      mySerial.print(F("Address: ")); mySerial.println(address);
      mySerial.print(F("IntensityVal_A : ")); mySerial.println(IntensityVal_Temp_A);
      mySerial.print(F("durationVal_A : ")); mySerial.println(durationVal_A);
      mySerial.print(F("delayVal_A : ")); mySerial.println(delayVal_A); mySerial.println();
    } else  if (!strcmp(arg, "bt")) {
      if (setCHA) {
        mySerial.println(F("set error!"));
        return;
      }
      arg = sCmd.next();
      if (!strcmp(arg, "1x")) {
        setTriggerType_B = triggerType_1x;
      } else if (!strcmp(arg, "10")) {
        setTriggerType_B = triggerType_10;
      } else if (!strcmp(arg, "x1")) {
        setTriggerType_B = triggerType_x1;
      } else if (!strcmp(arg, "01")) {
        setTriggerType_B = triggerType_01;
      } else if (!strcmp(arg, "11")) {
        setTriggerType_B = triggerType_11;
      } else if (!strcmp(arg, "00")) {
        setTriggerType_B = triggerType_00;
      } else if (!strcmp(arg, "xx")) {
        setTriggerType_B = triggerType_xx;
      } else {
        mySerial.println(F("set error")); return;
      }
      mySerial.println(F("Set the channel_B "));
      mySerial.print(F("Address: ")); mySerial.println(address);
      mySerial.print(F("IntensityVal_B : ")); mySerial.println(IntensityVal_Temp_B);
      mySerial.print(F("durationVal_B: ")); mySerial.println(durationVal_B);
      mySerial.print(F("delayVal_B : ")); mySerial.println(delayVal_B); mySerial.println();
    }
  } else {
    mySerial.println("No  argument");
  }
  setCHA = false;
  setCHB = false;
  attachInterrupt(digitalPinToInterrupt(2), &handler1, RISING);
  attachInterrupt(digitalPinToInterrupt(3), &handler2, RISING);
  if (setTriggerType_A == triggerType_xx ) {

    digitalWrite(ledA, HIGH);
  }
  if (setTriggerType_B == triggerType_xx ) {

    digitalWrite(ledB, HIGH);
  }
  if (setTriggerType_A == triggerType_00 ) {
    //analogWrite(ledA, 0);
    digitalWrite(ledA, LOW);
  }
  if (setTriggerType_B == triggerType_00 ) {
    digitalWrite(ledB, LOW);
  }
  mySerial.println("Set  successful.");
}

void handler1() {
  delayMicroseconds(10);
  if (digitalRead(2)) {
    detachInterrupt(digitalPinToInterrupt(2));
    if (digitalRead(3)) {
      signalState = triggerType_11;
    } else {
      signalState = triggerType_10;
    }
    hander1Flag = true;
    checkDelayCHA = true;
    checkDelayCHB = true;
    timeStartA = micros();
    timeStartB = micros();
  }
}
void handler2() {
  delayMicroseconds(10);
  detachInterrupt(digitalPinToInterrupt(3));
  if (digitalRead(3)) {
    if (digitalRead(2)) {
      signalState = triggerType_11;
    } else {
      signalState = triggerType_01;
    }
    hander2Flag = true;
    checkDelayCHA = true;
    checkDelayCHB = true;
    timeStartA = micros();
    timeStartB = micros();
  }
}

uint8_t autoGetAddress() {
  uint8_t addressTemp  = 0;
  addressTemp |= digitalRead(addressBit3);  addressTemp = addressTemp << 1;
  addressTemp |= digitalRead(addressBit2);  addressTemp = addressTemp << 1;
  addressTemp |= digitalRead(addressBit1);  addressTemp = addressTemp << 1;
  addressTemp |= digitalRead(addressBit0);
  return addressTemp;
}


