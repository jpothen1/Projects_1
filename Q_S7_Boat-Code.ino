#include <Adafruit_MPU6050.h>                    //IMU sensor
//#include <Adafruit_Sensor.h>                   //Adafruit_MPU6050.h
#include <ESP32Servo.h>
//#include <Wire.h>                              //oled.h & Adafruit_MPU6050.h
#include <math.h>
#include "MQTT.h"                                
//#include <WiFi.h>                              //Included in the MQTT.h
//#include <WiFiClientSecure.h>                  //Included in the MQTT.h
//#include <PubSubClient.h>                      //Included in the MQTT.h
#include "ENGR122_ArduinoBoat_PID.h"
//#include <PID_v1.h>                            //Included in the ENGR122_ArduinoBoat_PID.h
#include "oled.h"
//#include <Adafruit_GFX.h>                      //Included in the oled.h
//#include <Adafruit_SSD1306.h>                  //Included in the oled.h

Adafruit_MPU6050 mpu;

Servo lfan;
Servo rfan;

const char* ssid = "Stevens-IoT";
const char* password = "nMN882cmg7";

//MQTT Broker Credential
const char* mqtt_server = "98db5050a791439c98eac188febfecbe.s2.eu.hivemq.cloud";
const char* mqtt_username = "stevens";
const char* mqtt_password = "Stevens@1870";
const int mqtt_port = 8883;

//"aruco/lulu3" for aruco marker #4, 8  //"aruco/lulu4" for aruco marker #3, 5 //
//North
//"aruco/lulu5" for aruco marker #4, 8  //"aruco/lulu6" for aruco marker #3, 5 //
String tempTopic = "aruco/lulu3";

float x_robot;
float y_robot;
float z_ang_robot;
String str_3;
String str_4;
String str_5;


double coord[] = {-1,-1};           
float target_angle = 0;        
float target_bubble = 200;       

int current_target = 0;
bool motors_enabled = true;           

const int lfanPin = 2;
const int rfanPin = 4;

const int neutralSpeed = 90;  
    
int leftFanSpeed  = 90; 
int rightFanSpeed = 90; 

int forward_vel_l = 170;
int forward_vel_r = 170;
int backward_vel_l = 60;
int backward_vel_r = 60;

float gx, gy, gz, ax, ay, az;
float x;
float y;
float robot_angle;
float distance;

double Setpoint;
double Input;
double Output;

void setup() {
  Serial.begin(115200);
  
  Wire.begin(21,22);                 
  oled_setup();
  
  String str = "OLED INITIALIZED";
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(10, 0);
  display.println(str);
  delay(500);
  wifi_mqtt_init();
  
  lfan.attach(lfanPin, 1300, 1700);
  rfan.attach(rfanPin, 1300, 1700);
  delay(1500);
  display.clearDisplay();

  setupIMU();

  pid_init();
  Setpoint = 0; 
}

void loop() {
  display.clearDisplay();
  if (!client.connected()) {
    reconnect();
    client.loop();
    client.subscribe(tempTopic.c_str(),0);
  }
  
  readRawImu();
  
  x = x_robot;
  y = y_robot;
  robot_angle = z_ang_robot;

  String str_1 = "X: " + String(x); //strings needed to display the x and y coordinates and the current angle of the boat for troubleshooting
  String str_2 = "Y: " + String(y);
  String str_3 = "Robot Angle:" + String(robot_angle);                       

  int x_target[] = {650, 1800, 1800, 240}; //an array of all of the targets to allow for easy switching of the targets to optimise the boat
  int y_target[] = {630, 630, 260, 260}; 
  int num_target = sizeof(x_target) / sizeof(x_target[0]); //function to know how many targets in the array 

  int targetX = x_target[current_target]; //sets the current target coordinates
  int targetY = y_target[current_target];

  float dx = targetX - x; //calculates the distance to the target
  float dy = targetY - y;
  distance = sqrt(pow(dx, 2) + pow(dy, 2));

  if (distance < target_bubble) { //once the distance is within 200mm the fans will stop for a split second and the screen will display "Reached Target!" for a second
        stopFans(0);
        display.clearDisplay();
        display.setTextSize(1.5);
        display.setTextColor(WHITE);
        display.setCursor(10, 0);
        display.println("Reached Target!");
        display.display();
        delay(1000);
      
      current_target = (current_target + 1) % num_target; //a function to tell the boat once the target is reached to move on to the next one in the array
  }

  display.clearDisplay(); //code for the previous x, y, and angle strings to output that information to the display
  display.setTextSize(1.5);
  display.setTextColor(WHITE);
  display.setCursor(10, 0);
  display.println(str_1);
  display.setCursor(10, 15);
  display.println(str_2);
  display.setCursor(10, 30);
  display.println(str_3);
  display.display();   


  float target_angle = atan2(dy, dx) * (180/M_PI); //calculates the angle the robot needs to move to
  float angle_turn = target_angle - robot_angle;

  while (angle_turn > 180) angle_turn -= 360; //normalizes the angle between 180 and -180 degrees
  while (angle_turn < -180) angle_turn += 360;

  
  if (fabs(angle_turn) > 15) { //if the robot is 15 degrees or more off of the required target angle it will turn till it is within the 15 degree threshold
  turn_stop(angle_turn);
  } else {
  forward(); //once its correctly oriented itself it will move forward
  delay(300);  
  stopFans(0); 
}

      mqtt_clean();
  }

void forward_backward(int amount1, int amount2){
  for(int i = 0; i<amount1; i++) {
      readRawImu();
      forward();
    delay(15);
  }
  for(int j = 0; j<amount2; j++) {
      readRawImu();
      backward();
    delay(30);
  }
}

void turn_stop(double degree){
  turn(degree);
  readRawImu();
  while (fabs(gz) > 0.1) {
    adjustRotationalSpeed(gz);
    readRawImu();
  }
  stopFans(0);
}

void readRawImu() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  unsigned long currentTime = millis(); //gets current time and caluclates the difference for the function below
  static unsigned long lastTime = currentTime;
  float dt = (currentTime - lastTime) / 1000.0; 
  lastTime = currentTime;

  gz = g.gyro.z;
  gy = g.gyro.y;
  gx = g.gyro.x;

  az = a.acceleration.z;
  ay = a.acceleration.y;
  ax = a.acceleration.x;

  z_ang_robot += gz * dt * (180.0 / M_PI);  //calculating the robots angle along the z axis

  
  while (z_ang_robot > 180) z_ang_robot -= 360; //normalizing the angle
  while (z_ang_robot < -180) z_ang_robot += 360;
}

void forward() {
  Input = gz; 
  myPID_forward.Compute();
  leftFanSpeed = constrain(forward_vel_l -  Output, 90, 180);
  rightFanSpeed = constrain(forward_vel_r + Output, 90, 180);
  lfan.write(leftFanSpeed);
  rfan.write(rightFanSpeed);
}

void backward() {
  Input = gz; 
  myPID_forward.Compute();
  leftFanSpeed  = constrain(backward_vel_l - Output, 0, 90);
  rightFanSpeed = constrain(backward_vel_r + Output, 0, 90);
  lfan.write(leftFanSpeed);
  rfan.write(rightFanSpeed);
}

void turn(double deg) {
  int turnSpeed = 70;
  int turnDuration = fabs(deg) * 5;
  if (deg > 0) {
    lfan.write(neutralSpeed - turnSpeed);
    rfan.write(neutralSpeed + turnSpeed);
  } else {
    lfan.write(neutralSpeed + turnSpeed);
    rfan.write(neutralSpeed - turnSpeed);
  }
 delay(turnDuration);
}

void stopFans(int amount2) {
  float AverageSpeed = (leftFanSpeed + rightFanSpeed)/2;
  lfan.write(neutralSpeed - (AverageSpeed - neutralSpeed)*2);
  rfan.write(neutralSpeed - (AverageSpeed - neutralSpeed)*2); 
  delay(amount2); 
  lfan.write(neutralSpeed);
  rfan.write(neutralSpeed);
}

void adjustRotationalSpeed(float gz) {
  Input = gz; 
  myPID_rstop.Compute();
  leftFanSpeed  = constrain(neutralSpeed - Output, 0, 180); 
  rightFanSpeed = constrain(neutralSpeed + Output, 0, 180);

  lfan.write(leftFanSpeed);
  rfan.write(rightFanSpeed);
} 

void setupIMU() {
  if (!mpu.begin(0x68)) {
    while (1) {
      delay(10);
    }
  }
}
