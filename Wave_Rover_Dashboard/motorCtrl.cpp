#include "motorCtrl.h"

void pinInit() {
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);

  ledcSetup(ledChannel1, freq, resolution);
  ledcSetup(ledChannel2, freq, resolution);

  ledcAttachPin(PWMA, ledChannel1);
  ledcAttachPin(PWMB, ledChannel2);

  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);
}

void motorDrive(int speed, int channel, int motorPin1, int motorPin2) {
  if (speed > 5) {
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, LOW);
    ledcWrite(channel, abs(speed));
  } else if (speed < -5) {
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, HIGH);
    ledcWrite(channel, abs(speed));
  } else {
    speed = 0;
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, HIGH);
    ledcWrite(channel, 0);
  }
}


void differential_steer(int XValue, int YValue) {
  // Outside no action limit joystick
  // if (!((XValue > -5) && (XValue < 5) && (YValue > -5) && (YValue < 5)||(YValue==0)))
  if (!(((XValue > -5) && (XValue < 5)) || ((YValue > -5) && (YValue < 5)))) {
    Serial.print(XValue);
    Serial.print(",");
    Serial.print(YValue);

    // XValue = map(XValue, -90, 90, -255, 255); // 0-180
    // YValue = map(YValue, -90, 90, 255, -255); // 0-180 reversed

    DiffSteer.computeMotors(XValue, YValue);
    int leftMotor = DiffSteer.computedLeftMotor();
    int rightMotor = DiffSteer.computedRightMotor();

    // sendMessage("leftMotor", String(leftMotor));
    // sendMessage("rightMotor", String(rightMotor));

    leftMotor = map(leftMotor, -127, 127, -255, 255);    // 0-180
    rightMotor = map(rightMotor, -127, 127, -255, 255);  // 0-180 reversed
    Serial.print(" ** ");
    Serial.print(leftMotor);
    Serial.print(",");
    Serial.println(rightMotor);
    motorDrive(leftMotor, ledChannel1, AIN1, AIN2);
    motorDrive(rightMotor, ledChannel2, BIN1, BIN2);
  } else {
    motorDrive(idle, ledChannel1, AIN1, AIN2);
    motorDrive(idle, ledChannel2, BIN1, BIN2);
  }
}