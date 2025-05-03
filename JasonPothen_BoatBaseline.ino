
//USERequired Libraries --------------------------------------------------------------------
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

//Create instances -----------------------------------------------------------------------
Adafruit_MPU6050 mpu;

Servo lfan;
Servo rfan;

// [START] MQTT & WiFI setting-------------------------------------------------------------
// Make sure your MQTT and WiFi network settings are correct
const char* ssid = "Stevens-IoT";
const char* password = "nMN882cmg7";

//MQTT Broker Credential
const char* mqtt_server = "98db5050a791439c98eac188febfecbe.s2.eu.hivemq.cloud";
const char* mqtt_username = "stevens";
const char* mqtt_password = "Stevens@1870";
const int mqtt_port = 8883;

//ArUco marker Setting
//Since the robot is distinguished using two different combinations for the south and north sides, 
//the following MQTT Topic information should be properly modified in the code.
//South
//"aruco/lulu3" for aruco marker #4, 8  //"aruco/lulu4" for aruco marker #3, 5 //
//North
//"aruco/lulu5" for aruco marker #4, 8  //"aruco/lulu6" for aruco marker #3, 5 //
//String payload;
String tempTopic = "aruco/lulu3";
// [END  ] MQTT & WiFI setting ------------------------------------------------------------

// [START] Global variables --------------------------------------------------------------- 
// If you wnat to declare additional global variables, please declare them below-----------
String str_1, str_2;                         //Define string variables to store dynamic messages (fanspeed and duration info) for the OLED display.
  int fanspeed = 180;                        //Note! 0 or 180 is the maximum speed, 90 Stop
  int duration = 100;    



// [END] Global variables -----------------------------------------------------------------
float x_robot;
float y_robot;
float z_ang_robot;
String str_3;
String str_4;
String str_5;


double coord[] = {-1,-1};             // Current coordinates
float target_angle = 0;        
float target_bubble = 100;       // angle - Check the units!

int current_target = 0;
bool motors_enabled = true;           // To disable motors

const int lfanPin = 2;
const int rfanPin = 4;

const int neutralSpeed = 90;  
    
int leftFanSpeed  = 90; 
int rightFanSpeed = 90; 

int forward_vel_l = 160;
int forward_vel_r = 160;
int backward_vel_l = 30;
int backward_vel_r = 30;

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
  
  Serial.print("Robot x: "); 
  Serial.print(x);
  Serial.print("Roboy y: ");
  Serial.print(y);
  Serial.print("Robot Angle: ");
  Serial.println(robot_angle);
  delay(500);


  int x_target[] = {1900};
  int y_target[] = {500};

  int targetX = x_target[current_target];
  int targetY = y_target[current_target];

  float dx = targetX - x;
  float dy = targetY - y;

  distance = sqrt(pow(dx, 2) + pow(dy, 2));

  Serial.print("Distance to target: ");
  Serial.println(distance);


  if (distance < target_bubble) {
        stopFans(500);
        display.clearDisplay();
        display.setTextSize(1.5);
        display.setTextColor(WHITE);
        display.setCursor(10, 0);
        display.println("Reached Target! Stopping.");
        delay(2000);
        while(1);
        return;
    }

  float target_angle = atan2(dy, dx) * (180/M_PI);

  float angle_turn = target_angle - robot_angle;

  // if (angle_turn > 180) angle_turn -= 360;
  // if (angle_turn < -180) angle_turn += 360;

  
    if (fabs(angle_turn) > 15) {
        turn_stop(angle_turn);
    } else {
       forward_backward(8,1);
       stopFans(500);
       delay(500);
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


  gz = g.gyro.z;
  gy = g.gyro.y;
  gx = g.gyro.x;

  az = a.acceleration.z;
  ay = a.acceleration.y;
  ax = a.acceleration.x;
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
  if (deg > 0) {
    leftFanSpeed = 30;
    rightFanSpeed = 50; 
    lfan.write(leftFanSpeed);
    rfan.write(rightFanSpeed);
    delay(fabs(deg) * 5);
  }
  else {
    leftFanSpeed = 50;
    rightFanSpeed = 30;
    lfan.write(leftFanSpeed);
    rfan.write(rightFanSpeed);
    delay(fabs(deg) * 5);
  }
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
