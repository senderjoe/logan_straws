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
#define ALIGNSPEED 500
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
int alignedTime;

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
void initStage(int stageNo);
void alignMotors();
bool stageComplete();
int findMotorIndex(char inByte);
void alignAuto();
void alignBySensor(int stepperID);
int getAverage (int (&lastTen)[10], int sensorValue);


void setup() {
  Serial.begin(115200);
  // pinMode (sensor1, INPUT);
  // sensor1val = digitalRead(sensor1);

  for (int i = 0; i < 12; ++i) {
    initStepper(motors[i]);
  }
  
  // alignManual();
  alignAuto();
  
  initStage(0);
  for (int i = 0; i < 12; ++i) {
    Serial.println(motors[i].targetPosition());
  }
}


void loop() {
  if (stageComplete() && currentStage != -1) {
    initStage(currentStage + 1);
  }
  
  for (int i = 0; i < 12; ++i) {
    motors[i].run();
  }
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

void initStage(int stageNo) {
  currentStage = stageNo;
  Serial.print("Stage ");
  Serial.print(stageNo);
  Serial.println();

  switch(stageNo) {
    case 0:  // open / close
      resetCurrentPosition();
      doRotation(2.5);
      break;
    case 1:
      initRandomOffset();
      break;
    case 2:
      doRotation(2);
      break;
    case 3:
      resetRandomOffset();
      break;
    case 4:
      initCylinderOffset();
      break;
    case 5:
      doRotation(3); 
      break;
    case 6:
      resetCylinderOffset();
      break;
    case 7:
      initRandomOffset();
      break;
    case 8:
      doRotation(2.5);
      break;
    case 9:
      resetRandomOffset();
      break;
    case 10:
      if (!LOOPING) {
        currentStage = -1;
        break;
      }
      delay(1000);
      if (millis() - alignedTime > 60 * 60 * 1000) {  // 1 hour has passed
        doRotation(0.5);  // open in prep for aligning
      } else {
        initStage(0);
      }
      break;
    case 11:
      alignAuto();
      initStage(0);
      break;

    default:
      currentStage = -1;

  }
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
  AccelStepper &motor = motors[stepperID];
  int &sensorPin = sensorPins[stepperID];
  int sensorValue = 0;
  int sensorAverage = 0;
  int lastTen[10] = {};
  int alignMode = 0;  
  int lowStart = 0;
  int lowEnd = 0;
  int midpoint = 0;

  motor.setSpeed(ALIGNSPEED);
  motor.setMaxSpeed(ALIGNSPEED);
  motor.moveTo(STEP_PER_REVOLUTION * 2);

  // wait until low
  while (alignMode == 0) {
    sensorValue = analogRead(sensorPin);
    Serial.print(stepperID);
    Serial.print(alignMode);
    Serial.println(sensorValue);
    if (sensorValue < SENSOR_LOW) {
      alignMode = 1;  
    }
    motor.run();
    sensorAverage = getAverage(lastTen, sensorValue);
  }

  // find start of high
  while (alignMode == 1) {
    sensorValue = analogRead(sensorPin);
    sensorAverage = getAverage(lastTen, sensorValue);
    Serial.println(sensorValue);
    if (sensorAverage > SENSOR_HIGH) {
      lowStart = motor.currentPosition() - 10;
      alignMode = 2;  
    }
    motor.run();
  }

  // find end of high
  while (alignMode == 2) {
    sensorValue = analogRead(sensorPin);
    sensorAverage = getAverage(lastTen, sensorValue);
    Serial.println(sensorAverage);
    if (sensorAverage < SENSOR_MID) {
      lowEnd = motor.currentPosition() - 10;
      alignMode = -1;  // end
    }
    motor.run();
  
  }

  if (lowEnd - lowStart < 0) {  // wraps around 0
    midpoint = (lowStart + lowEnd + STEP_PER_REVOLUTION)/2 % STEP_PER_REVOLUTION;
  } else {
    midpoint = (lowStart + lowEnd) / 2;
  }

  motor.moveTo(midpoint);
  motor.runToPosition();
  motor.setCurrentPosition(0);
  motor.setSpeed(SPEED);
  motor.setMaxSpeed(SPEED);
  alignedTime = millis();

}

// get average of last 10 sensor values 
int getAverage (int (&lastTen)[10], int sensorValue) {
  int total = sensorValue;
  for (int i = 0; i < 9; i++) {
    total += lastTen[i];
    lastTen[i] = lastTen[i+1];
  }
  lastTen[9] = sensorValue;
  int average = total / 10;
  return average;

}
