#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Servo.h>
#include <cstdint>
//
//  motor-1(A) 0-----0 motor-2(B)
//             | Rc  |
//             | car |
//  motor-3(A) 0-----0 motor-4(B) enable 2 , 4 servo  d6
//
//
static bool ULTIMATE_CONTROL_MODE = false;
static bool LIGHT_MODE = false;

static const uint8_t MOTOR_A_IN1 = D0; // IN1 on L298N
static const uint8_t MOTOR_A_IN2 = D1; // IN2 on L298N
static const uint8_t MOTOR_B_IN1 = D6; // IN3 on L298N
static const uint8_t MOTOR_B_IN2 = D7; // IN4 on L298N

static const uint8_t MOTOR_A_ENABLE = D2; // ENA on L298N
static const uint8_t MOTOR_B_ENABLE = D4; // ENB on L298N

static uint8_t MOTOR_SPEED_PERCENTAGE = 100; // ENB on L298N

static const uint8_t SERVO_PIN = D8; // ENB on L298N
static const uint8_t SERVO_ANGLE_LEFT = 0;
static const uint8_t SERVO_ANGLE_NEUTRAL = 90;
static const uint8_t SERVO_ANGLE_RIGHT = 180;

static const uint8_t LIGHT_PIN = D3;

static const char AP_SSID[] = "ESP8266_AP";
static const char AP_PASSWORD[] = "12345678";
static const char FORWARD_COMMAND = 'F';
static const char BACKWARD_COMMAND = 'B';
static const char LEFT_COMMAND = 'L';
static const char RIGHT_COMMAND = 'R';
static const char FORWARD_LEFT_COMMAND = 'G';
static const char BACKWARD_LEFT_COMMAND = 'H';
static const char FORWARD_RIGHT_COMMAND = 'I';
static const char BACKWARD_RIGHT_COMMAND = 'J';
static const char STOP_COMMAND = 'S';
static const char ENABLE_HORN_COMMAND = 'V';
static const char DISABLE_HORN_COMMAND = ' ';
static const char LIGHT_ON_COMMAND = 'W';
static const char LIGHT_OFF_COMMAND = 'w';
static const char MAX_SPEED_COMMAND = 'q';

static const uint8_t MAX_SPEED = 255;
static const uint8_t MIN_SPEED = 0;

static const float WHEEL_RATIO_WHEN_TURNING = 1.0 / 4.0;
void MoveForward();
void MoveBackward();
void MoveLeft();
void MoveRight();
void MoveForwardLeft();
void MoveForwardRight();
void MoveBackwardLeft();
void MoveBackwardRight();
void StopVechile();
void EnableHorn();
void DisableHorn();
void TurnOnLight();
void TurnOffLight();
void InvalidCommandPressed();

void SetMotorForward(uint8_t, uint8_t);
void SetMotorBackward(uint8_t, uint8_t);
void SetMotorStop(uint8_t, uint8_t);

void SteeringPositionLeft();
void SteeringPositionRight();
void SteeringPositionNeutral();

void SetSpeedPercentage(uint8_t, uint8_t);
bool isDigit(char);
AsyncWebServer server(80);
Servo servo;

void setup() {
  Serial.begin(9600);
  servo.attach(SERVO_PIN);

  pinMode(MOTOR_A_ENABLE, OUTPUT);
  pinMode(MOTOR_B_ENABLE, OUTPUT);
  pinMode(MOTOR_A_IN1, OUTPUT);
  pinMode(MOTOR_A_IN2, OUTPUT);
  pinMode(MOTOR_B_IN1, OUTPUT);
  pinMode(MOTOR_B_IN2, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);

  analogWriteFreq(400);
  analogWriteRange(MAX_SPEED);

  SetSpeedPercentage(MOTOR_A_ENABLE, MOTOR_SPEED_PERCENTAGE);
  SetSpeedPercentage(MOTOR_B_ENABLE, MOTOR_SPEED_PERCENTAGE);

  // Set up the access point
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.println("Access point created");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  // Define request handlers
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("State")) {

      String stateValue = request->getParam("State")->value();
      // Serial.printf("\nSpeed : %d %% |", MOTOR_SPEED_PERCENTAGE);
      Serial.printf("%s ( ", stateValue.c_str());
      for (int i = 0; i < stateValue.length(); i++) {
        Serial.printf("%d", int(stateValue.charAt(i)));
      }
      Serial.printf(" ) : ");
      char command = stateValue.charAt(0);

      if (command == FORWARD_COMMAND)
        MoveForward();
      else if (command == BACKWARD_COMMAND)
        MoveBackward();
      else if (command == LEFT_COMMAND)
        MoveLeft();
      else if (command == RIGHT_COMMAND)
        MoveRight();
      else if (command == FORWARD_LEFT_COMMAND)
        MoveForwardLeft();
      else if (command == FORWARD_RIGHT_COMMAND)
        MoveForwardRight();
      else if (command == BACKWARD_LEFT_COMMAND)
        MoveBackwardLeft();
      else if (command == BACKWARD_RIGHT_COMMAND)
        MoveBackwardRight();
      else if (command == STOP_COMMAND)
        StopVechile();
      else if (command == ENABLE_HORN_COMMAND)
        EnableHorn();
      else if (command == DISABLE_HORN_COMMAND)
        DisableHorn();
      else if (command == LIGHT_ON_COMMAND)
        TurnOnLight();
      else if (command == LIGHT_OFF_COMMAND)
        TurnOffLight();
      else if (isDigit(command)) {
        // converting 0,1,2 to percentage
        MOTOR_SPEED_PERCENTAGE = (command - '0') * 10;
        SetSpeedPercentage(MOTOR_A_ENABLE, MOTOR_SPEED_PERCENTAGE);
        SetSpeedPercentage(MOTOR_B_ENABLE, MOTOR_SPEED_PERCENTAGE);
      } else if (command == MAX_SPEED_COMMAND) {
        MOTOR_SPEED_PERCENTAGE = 100;
        SetSpeedPercentage(MOTOR_A_ENABLE, MOTOR_SPEED_PERCENTAGE);
        SetSpeedPercentage(MOTOR_B_ENABLE, MOTOR_SPEED_PERCENTAGE);
      } else
        InvalidCommandPressed();
      request->send(200, "text/plain", "OK");
    } else {
      request->send(400, "text/plain", "Missing State parameter");
    }
  });
  server.begin();
  Serial.println("HTTP server started");
}
void loop() {}

void MoveForward() {
  Serial.println("Forward");
  SteeringPositionNeutral();
  SetMotorForward(MOTOR_A_IN1, MOTOR_A_IN2);
  SetMotorForward(MOTOR_B_IN1, MOTOR_B_IN2);
  SetSpeedPercentage(MOTOR_A_ENABLE, MOTOR_SPEED_PERCENTAGE);
  SetSpeedPercentage(MOTOR_B_ENABLE, MOTOR_SPEED_PERCENTAGE);
}

void MoveBackward() {
  Serial.println("Backward");
  SteeringPositionNeutral();
  SetMotorBackward(MOTOR_A_IN1, MOTOR_A_IN2);
  SetMotorBackward(MOTOR_B_IN1, MOTOR_B_IN2);
  SetSpeedPercentage(MOTOR_A_ENABLE, MOTOR_SPEED_PERCENTAGE);
  SetSpeedPercentage(MOTOR_B_ENABLE, MOTOR_SPEED_PERCENTAGE);
}

void MoveLeft() {
  Serial.println("Left");
  SteeringPositionLeft();
  if (!ULTIMATE_CONTROL_MODE)
    return;
  SetMotorBackward(MOTOR_A_IN1, MOTOR_A_IN2);
  SetMotorForward(MOTOR_B_IN1, MOTOR_B_IN2);
  SetSpeedPercentage(MOTOR_A_ENABLE, MOTOR_SPEED_PERCENTAGE);
  SetSpeedPercentage(MOTOR_B_ENABLE, MOTOR_SPEED_PERCENTAGE);
}

void MoveRight() {
  Serial.println("Right");
  SteeringPositionRight();
  if (!ULTIMATE_CONTROL_MODE)
    return;
  SetMotorBackward(MOTOR_B_IN1, MOTOR_B_IN2);
  SetMotorForward(MOTOR_A_IN1, MOTOR_A_IN2);
  SetSpeedPercentage(MOTOR_A_ENABLE, MOTOR_SPEED_PERCENTAGE);
  SetSpeedPercentage(MOTOR_B_ENABLE, MOTOR_SPEED_PERCENTAGE);
}

void MoveForwardLeft() {
  Serial.println("Forward Left");
  MoveForward();
  SteeringPositionLeft();
  if (!ULTIMATE_CONTROL_MODE)
    return;
  SetSpeedPercentage(MOTOR_A_ENABLE,
                     MOTOR_SPEED_PERCENTAGE * WHEEL_RATIO_WHEN_TURNING);
  SetSpeedPercentage(MOTOR_B_ENABLE, MOTOR_SPEED_PERCENTAGE);
}

void MoveForwardRight() {
  Serial.println("Forward Right");
  MoveForward();
  SteeringPositionRight();
  if (!ULTIMATE_CONTROL_MODE)
    return;
  SetSpeedPercentage(MOTOR_A_ENABLE, MOTOR_SPEED_PERCENTAGE);
  SetSpeedPercentage(MOTOR_B_ENABLE,
                     MOTOR_SPEED_PERCENTAGE * WHEEL_RATIO_WHEN_TURNING);
}

void MoveBackwardLeft() {
  Serial.println("Backward Left");
  MoveBackward();
  SteeringPositionLeft();
  if (!ULTIMATE_CONTROL_MODE)
    return;
  SetSpeedPercentage(MOTOR_A_ENABLE,
                     MOTOR_SPEED_PERCENTAGE * WHEEL_RATIO_WHEN_TURNING);
  SetSpeedPercentage(MOTOR_B_ENABLE, MOTOR_SPEED_PERCENTAGE);
}

void MoveBackwardRight() {
  Serial.println("Backward Right");
  MoveBackward();
  SteeringPositionRight();
  if (!ULTIMATE_CONTROL_MODE)
    return;
  SetSpeedPercentage(MOTOR_A_ENABLE, MOTOR_SPEED_PERCENTAGE);
  SetSpeedPercentage(MOTOR_B_ENABLE,
                     MOTOR_SPEED_PERCENTAGE * WHEEL_RATIO_WHEN_TURNING);
}

void StopVechile() {
  Serial.println("Stop");
  SetMotorStop(MOTOR_A_IN1, MOTOR_A_IN2);
  SetMotorStop(MOTOR_B_IN1, MOTOR_B_IN2);
  SteeringPositionNeutral();
}

void EnableHorn() {
  Serial.println("Horn : on");
  ULTIMATE_CONTROL_MODE = !ULTIMATE_CONTROL_MODE;
}

void DisableHorn() { Serial.println("Horn : off"); }
void TurnOnLight() {
  Serial.println("Light : on");
  LIGHT_MODE = HIGH;
  digitalWrite(LIGHT_PIN, LIGHT_MODE);
}
void TurnOffLight() {
  Serial.println("Light : off");
  LIGHT_MODE = LOW;
  digitalWrite(LIGHT_PIN, LIGHT_MODE);
}

void InvalidCommandPressed() { Serial.println("Invalid Command Pressed !!"); }

void SetMotorForward(uint8_t INPUT1, uint8_t INPUT2) {
  digitalWrite(INPUT1, HIGH);
  digitalWrite(INPUT2, LOW);
}
void SetMotorBackward(uint8_t INPUT1, uint8_t INPUT2) {
  digitalWrite(INPUT1, LOW);
  digitalWrite(INPUT2, HIGH);
}
void SetMotorStop(uint8_t INPUT1, uint8_t INPUT2) {
  digitalWrite(INPUT1, HIGH);
  digitalWrite(INPUT2, HIGH);
}
void SetSpeedPercentage(uint8_t pin, uint8_t percentage) {
  uint8_t speed = (percentage * MAX_SPEED) / 100;
  Serial.printf("\nSetting Motor Speed Value (0-%d) : %d. ", MAX_SPEED, speed);
  analogWrite(pin, speed);
}
void SteeringPositionLeft() { servo.write(SERVO_ANGLE_LEFT); }
void SteeringPositionRight() { servo.write(SERVO_ANGLE_RIGHT); }
void SteeringPositionNeutral() { servo.write(SERVO_ANGLE_NEUTRAL); }

bool isDigit(char ch) { return ch >= '0' && ch <= '9'; }
