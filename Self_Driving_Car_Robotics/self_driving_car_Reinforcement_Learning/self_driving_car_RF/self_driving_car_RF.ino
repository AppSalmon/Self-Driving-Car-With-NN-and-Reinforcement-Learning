#include <AFMotor.h>
#include <NewPing.h>
#include <Servo.h>

#define TRIG_PIN A0
#define ECHO_PIN A1
#define MAX_DISTANCE 150
#define MAX_SPEED 140
#define MAX_SPEED_OFFSET 20

NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

AF_DCMotor motor1(1, MOTOR12_1KHZ);
AF_DCMotor motor2(2, MOTOR12_1KHZ);
AF_DCMotor motor3(3, MOTOR34_1KHZ);
AF_DCMotor motor4(4, MOTOR34_1KHZ);
Servo myservo;

boolean goesForward = false;
int distance = 100;
int speedSet = 0;

const int numStates = 3; // Near, Medium, Far
const int numActions = 3; // Forward, Left, Right
float qTable[numStates][numStates][numStates][numActions]; // Q-Table
float learningRate = 0.1;
float discountFactor = 0.9;
int epsilon = 10; // Exploration rate (10%)

void setup() {
  myservo.attach(10);
  myservo.write(115);
  delay(2000);
  for (int i = 0; i < 4; i++) {
    distance = readPing();
    delay(50);
  }
  Serial.begin(9600);
}

void loop() {
  int state[3] = {discretize(distance), discretize(lookRight()), discretize(lookLeft())};
  int action = selectAction(state);
  int reward = performAction(action);

  int nextState[3] = {discretize(readPing()), discretize(lookRight()), discretize(lookLeft())};
  updateQTable(state, action, reward, nextState);
}

int discretize(int distance) {
  if (distance < 30) return 0; // Near
  if (distance < 60) return 1; // Medium
  return 2; // Far
}

int selectAction(int state[3]) {
  if (random(100) < epsilon) {
    return random(numActions); // Explore
  } else {
    return getMaxQAction(state); // Exploit
  }
}

int getMaxQAction(int state[3]) {
  float maxQ = -1000;
  int bestAction = 0;
  for (int a = 0; a < numActions; a++) {
    if (qTable[state[0]][state[1]][state[2]][a] > maxQ) {
      maxQ = qTable[state[0]][state[1]][state[2]][a];
      bestAction = a;
    }
  }
  return bestAction;
}

int performAction(int action) {
  switch(action) {
    case 0:
      moveForward();
      break;
    case 1:
      turnLeft();
      break;
    case 2:
      turnRight();
      break;
  }
  delay(500);
  moveStop();
  return readPing(); // Use the new distance as reward for simplicity
}

void updateQTable(int state[3], int action, int reward, int nextState[3]) {
  float maxQ = qTable[nextState[0]][nextState[1]][nextState[2]][getMaxQAction(nextState)];
  qTable[state[0]][state[1]][state[2]][action] = (1 - learningRate) * qTable[state[0]][state[1]][state[2]][action] +
                                                  learningRate * (reward + discountFactor * maxQ);
}

int readPing() {
  delay(50);
  int cm = sonar.ping_cm();
  if (cm == 0) cm = 250;
  return cm;
}

int lookRight() {
  myservo.write(50);
  delay(500);
  int distance = readPing();
  delay(100);
  myservo.write(115);
  return distance;
}

int lookLeft() {
  myservo.write(170);
  delay(500);
  int distance = readPing();
  delay(100);
  myservo.write(115);
  return distance;
}

void moveStop() {
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  motor3.run(RELEASE);
  motor4.run(RELEASE);
}

void moveForward() {
  if (!goesForward) {
    goesForward = true;
    motor1.run(FORWARD);
    motor2.run(FORWARD);
    motor3.run(FORWARD);
    motor4.run(FORWARD);
    for (speedSet = 0; speedSet < MAX_SPEED; speedSet += 2) {
      motor1.setSpeed(speedSet);
      motor2.setSpeed(speedSet);
      motor3.setSpeed(speedSet);
      motor4.setSpeed(speedSet);
      delay(5);
    }
  }
}

void turnRight() {
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(BACKWARD);
  motor4.run(BACKWARD);
  delay(500);
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
}

void turnLeft() {
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
  delay(500);
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
}
