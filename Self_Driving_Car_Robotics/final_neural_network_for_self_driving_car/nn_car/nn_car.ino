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
unsigned long moveStartTime = 0;
const unsigned long MOVE_DURATION = 5000;


// Input Layer Weights
float input_weights[3][5] = {
    {-1.07503892e-06, 3.18756737e-01, 7.46107667e-01, 1.15956894e-01, -4.03260328e-02},
    {-4.03396345e-02, -2.17407221e-01, 2.20367466e-01, 1.25770278e-01, 1.32099316e-03},
    {-1.50878342e-01, 1.25184803e+00, 3.31272525e-01, -5.47762628e-01, -2.71938820e-02}
};

// Hidden Layer Weights
float hidden_weights[5][4] = {
    {1.57860284e-08, -3.80535461e-02, -7.33745701e-04, -1.17298103e-06},
    {-2.80223163e-01, -6.36172075e-02, -1.04683854e-01, 8.75902225e-02},
    {-1.05749264e-01, -2.96692777e-01, -2.25601601e-01, -5.12402477e-01},
    {-6.71791326e-01, 3.13756695e-01, 7.21927084e-01, 3.02706177e-01},
    {-3.78284135e-04, -6.28180985e-02, 1.90206060e-04, -9.28427862e-20}
};

// Input Layer Biases
float input_biases[5] = {-0.54835947, -0.62750809, 0.09388703, -0.17051108, -0.36160174};

// Hidden Layer Biases
float hidden_biases[4] = {-0.22015047, 0.23561133, -1.85591617, 0.30063622};


void setup() {
  myservo.attach(10);  
  myservo.write(115); 
  delay(2000);
  distance = readPing();
  delay(50);
  distance = readPing();
  delay(50);
  distance = readPing();
  delay(50);
  distance = readPing();
  delay(100);
  moveStartTime = millis();
}

void loop() {
 int distanceR = 0;
 int distanceL = 0;
 int distanceF = 0;
 delay(40);

 if (distance <= 30 || millis() - moveStartTime >= MOVE_DURATION) {
  moveStop();
  delay(100);
  moveBackward();
  delay(300);
  moveStop();
  delay(200);
  distanceR = lookRight();
  delay(200);
  distanceL = lookLeft();
  delay(200);
  distanceF = lookForward();
  delay(200);

  int direction = predictDirection(distanceL, distanceF, distanceR);

  if (direction == 0) {
    moveBackward();
  } else if (direction == 1) {
    moveForward();
    moveStop();

  } else if (direction == 2) {
    turnLeft();
    moveStop();
  } else {
    turnRight();
    moveStop();
  }

  moveStartTime = millis();
 } else {
  moveForward();
 }
 distance = readPing();
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

int lookForward() {
    myservo.write(115); 
    delay(500);
    int distance = readPing();
    delay(100);
    return distance;
}

int readPing() { 
  delay(50);
  int cm = sonar.ping_cm();
  if (cm == 0) {
    cm = 250;
  }
  return cm;
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

void moveBackward() {
  goesForward = false;
  motor1.run(BACKWARD);      
  motor2.run(BACKWARD);
  motor3.run(BACKWARD);
  motor4.run(BACKWARD);  
  for (speedSet = 0; speedSet < MAX_SPEED; speedSet += 2) {
    motor1.setSpeed(speedSet);
    motor2.setSpeed(speedSet);
    motor3.setSpeed(speedSet);
    motor4.setSpeed(speedSet);
    delay(5);
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




int predictDirection(int distanceL, int distanceF, int distanceR) {
  // Input layer to hidden layer
  float hidden_layer[5];
  for (int i = 0; i < 5; i++) {
    hidden_layer[i] = input_weights[0][i] * distanceL + input_weights[1][i] * distanceF + input_weights[2][i] * distanceR + input_biases[i];
    hidden_layer[i] = max(0.0, hidden_layer[i]);  // ReLU activation
  }

  // Hidden layer to output layer
  float output_layer[4];
  for (int i = 0; i < 4; i++) {
    output_layer[i] = 0;
    for (int j = 0; j < 5; j++) {
      output_layer[i] += hidden_layer[j] * hidden_weights[j][i];
    }
    output_layer[i] += hidden_biases[i];
  }

  // Find the index of the maximum value in the output layer
  int max_index = 0;
  for (int i = 1; i < 4; i++) {
    if (output_layer[i] > output_layer[max_index]) {
      max_index = i;
    }
  }

  return max_index;
}
