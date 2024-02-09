 /*
 *
 */
#include <Arduino.h>
// Include the AccelStepper Library
#include <AccelStepper.h>

// define step constant
#define FULLSTEP 4
#define STEP_PER_REVOLUTION 2048  // this value is from datasheet
#define SPEED 250
#define ACCELL 500
#define SEGMENT STEP_PER_REVOLUTION/12
#define MANUAL_STEP STEP_PER_REVOLUTION/64
#define LOOPING 1

#define SENSOR_HIGH 400
#define SENSOR_MID 350
#define SENSOR_LOW 275

int sensor1val;
int sensor1 = 5;
int currentStage;

// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
AccelStepper stepper0(FULLSTEP, 6, 8, 7, 9);
AccelStepper stepper1(FULLSTEP, 5, 3, 4, 2);
AccelStepper stepper2(FULLSTEP, 14, 16, 15, 17);
AccelStepper stepper3(FULLSTEP, 18, 20, 19, 21);
AccelStepper stepper4(FULLSTEP, 22, 24, 23, 25);
AccelStepper stepper5(FULLSTEP, 26, 30, 28, 32); 
AccelStepper stepper6(FULLSTEP, 27, 31, 29, 33);
AccelStepper stepper7(FULLSTEP, 34, 38, 36, 40);
AccelStepper stepper8(FULLSTEP, 35, 39, 37, 41);
AccelStepper stepper9(FULLSTEP, 48, 44, 46, 42);
AccelStepper stepper10(FULLSTEP, 49, 45, 47, 43);
AccelStepper stepper11(FULLSTEP, 52, 50, 53, 51);

char forwardKeys[12] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l'};
char reverseKeys[12] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L'};
AccelStepper motors[12] {
  stepper0, 
  stepper1, 
  stepper2, 
  stepper3, 
  stepper4, 
  stepper5, 
  stepper6, 
  stepper7, 
  stepper8, 
  stepper9, 
  stepper10, 
  stepper11 
};

int sensorPins[12] {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11 };

int randomOffsets[12];

int cylinderOffsets[12] {
  static_cast<int>(SEGMENT * 6),
  static_cast<int>(SEGMENT * 5), 
  static_cast<int>(SEGMENT * 4), 
  static_cast<int>(SEGMENT * 3),
  static_cast<int>(SEGMENT * 2),
  static_cast<int>(SEGMENT),
  0,
  STEP_PER_REVOLUTION - static_cast<int>(SEGMENT),
  STEP_PER_REVOLUTION - static_cast<int>(SEGMENT * 2),
  STEP_PER_REVOLUTION - static_cast<int>(SEGMENT * 3),
  STEP_PER_REVOLUTION - static_cast<int>(SEGMENT * 4),
  STEP_PER_REVOLUTION - static_cast<int>(SEGMENT * 5)
};

void initStepper(AccelStepper &stepper);
void resetCurrentPosition();
void initRandomOffset();
void resetRandomOffset();
void initCylinderOffset();
void resetCylinderOffset();
void doRotation(float rotations);
void initStage0();
void initStage1();
void initStage2();
void initStage3();
void initStage4();
void initStage5();
void initStage6();
void initStage7();
void initStage8();
void initStage9();
void alignMotors();
bool stageComplete();
int findMotorIndex(char inByte);
void alignAuto();
void alignBySensor(int stepperID);


void setup() {
  Serial.begin(9600);
  // pinMode (sensor1, INPUT);
  // sensor1val = digitalRead(sensor1);

  for (int i = 0; i < 12; ++i) {
    initStepper(motors[i]);
  }
  
  // alignManual();
  alignAuto();
  
  initStage0();
  for (int i = 0; i < 12; ++i) {
    Serial.println(motors[i].targetPosition());
  }
}


void loop() {
  // change direction once the motor reaches target position
  //  if (stepper1.distanceToGo() == 0)
  //    stepper1.moveTo(0);
  if (stageComplete() && currentStage == 0) {
    delay(1000);
    initStage1();
  } else if (stageComplete() && currentStage == 1) {
    // delay(1000);
    initStage2();
  } else if ( stageComplete() && currentStage == 2) {
    // delay(1000);
    initStage3();
  } else if ( stageComplete() && currentStage == 3) {
    delay(1000);
    initStage4();
  } else if ( stageComplete() && currentStage == 4) {
    delay(1000);
    initStage5();
  } else if ( stageComplete() && currentStage == 5) {
    // delay(1000);
    initStage6();
  } else if ( stageComplete() && currentStage == 6) {
    delay(1000);
    initStage7();
  } else if ( stageComplete() && currentStage == 7) {
    // delay(1000);
    initStage8();
  } else if ( stageComplete() && currentStage == 8) {
    initStage9();
  } else if ( stageComplete() && currentStage == 9) {
    Serial.println("Stages Complete");
    currentStage = -1;
  } else if (LOOPING && stageComplete() && currentStage == -1) {
    initStage0();
  }
  
  for (int i = 0; i < 12; ++i) {
    motors[i].run();
  }
  

//  Serial.print(stepper1.currentPosition());
//  Serial.print("\t");
//  Serial.println(stepper2.currentPosition());

  // Serial.print(F("Current Position: "));
  // Serial.println(stepper.currentPosition());
//  int newSensor1Val = digitalRead(sensor1);
//  if (newSensor1Val != sensor1val) {
//    sensor1val = newSensor1Val;
//    Serial.println(newSensor1Val);
//  }
}

void initStepper(AccelStepper &stepper) {
  stepper.setMaxSpeed(SPEED);   // set the maximum speed
  stepper.setAcceleration(ACCELL); // set acceleration
  stepper.setSpeed(SPEED);         // set initial speed
  stepper.setCurrentPosition(0); // set position
}

void resetCurrentPosition() {
  for (int i = 0; i < 12; ++i) {
    motors[i].setCurrentPosition(0); 
  }
}

void initRandomOffset() {
  // Move to a random offset
  for (int i = 0; i < 12; ++i) {
    int randomOffset = random(0, STEP_PER_REVOLUTION) - (STEP_PER_REVOLUTION / 2);
    randomOffsets[i] = randomOffset;
    motors[i].moveTo(motors[i].currentPosition() + randomOffset); 
  }
}

void resetRandomOffset() {
  // Return from random offset
  for (int i = 0; i < 12; ++i) {
    motors[i].moveTo(motors[i].currentPosition() + STEP_PER_REVOLUTION - randomOffsets[i]); 
  }
}

void initCylinderOffset() {
  // Align in cylinder
  for (int i = 0; i < 12; ++i) {
    motors[i].moveTo(motors[i].currentPosition() + cylinderOffsets[i]); 
  }
}
void resetCylinderOffset() {
  // Reset from cylinder to flower
  for (int i = 0; i < 12; ++i) {
    motors[i].moveTo(motors[i].currentPosition() + STEP_PER_REVOLUTION - cylinderOffsets[i]); 
  }
}

void doRotation(float rotations){
  // Rotate
  for (int i = 0; i < 12; ++i) {
    motors[i].moveTo(motors[i].currentPosition() + STEP_PER_REVOLUTION * rotations); 
  }
}

void initStage0() {
  // Open / close
  resetCurrentPosition();
  doRotation(2.5);
  currentStage = 0;
  Serial.println("Stage 0");
}

void initStage1(){
  initRandomOffset();
  currentStage = 1;
  Serial.println("Stage 1");
}

void initStage2(){
  doRotation(2);
  currentStage = 2;
  Serial.println("Stage 2");
}

void initStage3(){
  resetRandomOffset();
  currentStage = 3;
  Serial.println("Stage 3");
}

void initStage4(){
  initCylinderOffset();
  currentStage = 4;
  Serial.println("Stage 4");
}

void initStage5(){
  doRotation(3);
  currentStage = 5;
  Serial.println("Stage 5");
}

void initStage6(){
  resetCylinderOffset();
  currentStage = 6;
  Serial.println("Stage 6");
}

void initStage7(){
  initRandomOffset();
  currentStage = 7;
  Serial.println("Stage 7");
}

void initStage8(){
  doRotation(2.5);
  currentStage = 8;
  Serial.println("Stage 8");
}
void initStage9(){
  resetRandomOffset();
  currentStage = 9;
  Serial.println("Stage 9");
}

void alignMotors(){
  stepper1.moveTo(STEP_PER_REVOLUTION); // set target position: 64 steps <=> one revolution
  while (digitalRead(sensor1) == 0) {}
    stepper1.run();
  }


bool stageComplete() {

  for (int i = 0; i < 12; ++i) {
    if (motors[i].distanceToGo() != 0) return false;
  }
  return true;
}


int findMotorIndex(char inByte) {
  for (int i = 0; i < 12; ++i)
    {
        if (forwardKeys[i] == inByte) return i;
        if (reverseKeys[i] == inByte) return i;
    }
    return -1;
  }


void alignManual() {
  Serial.println("Alignment mode");
  bool done = 0;
  while (!done) {
    if (Serial.available() == 0) {
      continue;
    }
    
    char inByte = Serial.read();
    Serial.println(inByte);
    if (inByte == 'z') break;

    int motorIndex = findMotorIndex(inByte);
    if (motorIndex == -1) continue;

    AccelStepper motor = motors[motorIndex];

    if (isupper(inByte)) 
      motor.moveTo(motor.currentPosition() - MANUAL_STEP);
    else
      motor.moveTo(motor.currentPosition() + MANUAL_STEP);

    motor.runToPosition();
    continue;

  }

  for (int i = 0; i < 12; ++i) {
    motors[i].setCurrentPosition(0);
  }

  Serial.println("Alignment done");
}

void alignAuto() {
  for (int i = 0; i < 12; ++i) {
    alignBySensor(i);
  }
}

void alignBySensor(int stepperID) {
  int alignMode = 0;  // wait until low
  int lowStart = 0;
  int lowEnd = 0;
  int midpoint = 0;
  int sensorValue = 0;

  motors[stepperID].moveTo(STEP_PER_REVOLUTION * 2);
  while (alignMode == 0) {
    sensorValue = analogRead(sensorPins[stepperID]);
    Serial.print(stepperID);
    Serial.print(alignMode);
    Serial.println(sensorValue);
    if (sensorValue < SENSOR_LOW) {
      alignMode = 1;  // find start of high
    }
    motors[stepperID].run();
  }

  while (alignMode == 1) {
    sensorValue = analogRead(sensorPins[stepperID]);
    Serial.println(sensorValue);
    if (sensorValue == 0) {  // skip
      return;
    }
    if (sensorValue > SENSOR_HIGH) {
      lowStart = motors[stepperID].currentPosition();
      alignMode = 2;  // find end of high
    }
    motors[stepperID].run();
  
  }

  while (alignMode == 2) {
    sensorValue = analogRead(sensorPins[stepperID]);
    Serial.println(sensorValue);
    if (sensorValue < SENSOR_MID) {
      lowEnd = motors[stepperID].currentPosition();
      alignMode = -1;  // end
    }
    motors[stepperID].run();
  
  }

  if (lowEnd - lowStart < 0) {  // wraps around 0
    midpoint = (lowStart + lowEnd + STEP_PER_REVOLUTION)/2 % STEP_PER_REVOLUTION;
  } else {
    midpoint = (lowStart + lowEnd)/2;
  }

  motors[stepperID].moveTo(midpoint);
  motors[stepperID].runToPosition();
  motors[stepperID].setCurrentPosition(0);

  
  
}
