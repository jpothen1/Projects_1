#include <DHT.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "SystemLogic.h"  //This header contains certificate info for MQTT connection
//Global variable declarations or pin number settings can be done before the setup() function.

#define WetSoil 1600  // Wet soil Reading - note readings to be determined by experiment
#define DrySoil 1900  // Dry soil Reading
#define PinNo 35      // Pin Number for Capacitive Sensor

// Pin definitions
const int DHT_PIN = 32;
const int PHOTOCELL_PIN = 34;
const int SOIL_PIN = 22;
const int PUMP_PIN = 25;
// WiFi credentials
const char* WIFI_NAME = "Stevens-IoT";  // on campus route
const char* WIFI_PASS = "nMN882cmg7";
//const char* WIFI_NAME = ""; // home router
//const char* WIFI_PASS = "";
// MQTT configuration
const char* MQTT_SERVER = "98db5050a791439c98eac188febfecbe.s2.eu.hivemq.cloud";
const char* MQTT_USER = "stevens";
const char* MQTT_PASS = "Stevens@1870";
const int MQTT_PORT = 8883;
// Team information
// You must follow the Topic naming protocol!
const String YEAR = "2024";
const String CLASS = "ENGR111";
const String SECTION = "A";         // Use your section name
const String GROUP_NUMBER = "S10";  // Use your group number
// Sensor configuration
// Important! The values of below two variables should be updated with the values
// measured by testing your Capacitive Soil Moisture sensor.
const int SOIL_MAXWET = 1320;
const int SOIL_MAXDRY = 2620;
//VariableS for setting the operating range of the pump
const int PUMP_ON_ANGLE = 180;
const int PUMP_OFF_ANGLE = 90;
// intital time count variables
const int publishInterval = 600000;  // test Sampling time change to 600000 (10 minutes) for operationconst char* MQTT_SERVER = "98db5050a791439c98eac188febfecbe.s2.eu.hivemq.cloud";
// Team information
// Sensor configuration
// Important! The values of below two variables should be updated with the values
// measured by testing your Capacitive Soil Moisture sensor.
//VariableS for setting the operating range of the pump
// intital time count variables
unsigned long day_time = 60000 * 60 * 24;  // = 1day
unsigned long previous_time = 0;

WiFiClientSecure net;

void setup() {
  wifi_start();
  mqtt_start();
  enable_sensors();
  previous_time = 0;
}


void loop() {
  mqtt_loop();
  // Read and publish Soil moisture sensor value:
  int soilhumidity = analogRead(SOIL_PIN);
  int soilPercent = map(soilhumidity, SOIL_MAXWET, SOIL_MAXDRY, 100, 0);  //calculate %

  //Publish soil humidity reading
  char pubString_soil_hum[8];                       // Character array
  dtostrf(soilhumidity, 1, 2, pubString_soil_hum);  //dtostrf(float_value, min_width, num_digits_after_decimal, where_to_store_string)
  String soilTopic = YEAR + "/" + CLASS + "/" + SECTION + "/" + GROUP_NUMBER + "/" + "soil_hum";
  client.publish(soilTopic.c_str(), pubString_soil_hum);  // publish
  // Measuring & Publishing Soil moisture sensor value: END -----------

  // Read temperature as Celsius (default) and convert to F
  int Temperature = dht.readTemperature();
  int TemperatureF = ((Temperature * 1.8) + 32);  // convert degrees C to F
  // Read humidity as % RH
  int Humidity = dht.readHumidity();

  //Publish TemperatureF
  char pubString_temp[8];                       // Character array for temperature
  dtostrf(TemperatureF, 1, 2, pubString_temp);  //dtostrf(float_value, min_width, num_digits_after_decimal, where_to_store_string)
  String tempTopic = YEAR + "/" + CLASS + "/" + SECTION + "/" + GROUP_NUMBER + "/" + "temp";
  client.publish(tempTopic.c_str(), pubString_temp);  // publish

  //Publish Air Humidity
  char pubString_hum[8];                   // Character array for air humidity
  dtostrf(Humidity, 1, 2, pubString_hum);  //dtostrf(float_value, min_width, num_digits_after_decimal, where_to_store_string)
  String humTopic = YEAR + "/" + CLASS + "/" + SECTION + "/" + GROUP_NUMBER + "/" + "hum";
  client.publish(humTopic.c_str(), pubString_hum);  // publish


  //Read Light Intensity
  int photocellReading = analogRead(PHOTOCELL_PIN);
  // add code to convert to LUX
  float volt = (((photocellReading) / 4095) * 3.3);
  float resist = (((3.3 * 10000) / (volt)) - 10000);
  float lux = 183 * exp(0.00121 * volt);
  // float lux = ((4*pow(10,6)*pow(resist,-0.9785)));

  //Publish Light Intensity
  char pubString_lux[8];                           // Character array for light
  dtostrf(photocellReading, 1, 2, pubString_lux);  //dtostrf(float_value, min_width, num_digits_after_decimal, where_to_store_string)
  String luxTopic = YEAR + "/" + CLASS + "/" + SECTION + "/" + GROUP_NUMBER + "/" + "lux";
  client.publish(luxTopic.c_str(), pubString_lux);  // publish

  //print all the data to serial monitor
Serial.print("Soil Humidity Reading: ");
Serial.print(soilhumidity);Serial.print(" ");Serial.print(soilPercent);Serial.println(" %");
Serial.print("Temp Degrees C:"); Serial.println(Temperature);
Serial.print("Temp Degrees F:"); Serial.println(TemperatureF);
Serial.print("Hum:"); Serial.println(Humidity);
Serial.print("light intensity: "); Serial.println(photocellReading);

  if (soilhumidity < SOIL_MAXWET) {
    Serial.println("Soil is too wet");
  } else if (soilhumidity >= SOIL_MAXWET && soilhumidity <= SOIL_MAXDRY) {
    Serial.println("Soil humidity is good");
  } else {
    Serial.println("Soil is too dry - time to water!");
  }

  // Plant Watering Logic-water if 24hrs has passed since last watering or if moisture content is low
  unsigned long elapsed_time;  // create an unsigned long data type variable named time
  elapsed_time = millis();     // returns the number of milliseconds passed
  if ((elapsed_time - previous_time) > day_time || soilhumidity >= SOIL_MAXDRY) {
    previous_time = elapsed_time;
    Serial.print("watering at time=");
    Serial.println(elapsed_time);
    // Start pump for 20 seconds = 20ML-------------------------------------------
    int pumpDuration = (soilPercent < 40) ? 55000 : 22000;  // 20 ml for <40%, 10 ml for >=40%

    mypump.attach(PUMP_PIN, 1000, 2000);  // attaches the minipump on PUMP_PIN
    mypump.write(PUMP_ON_ANGLE);
    delay(pumpDuration);  // Duration determines water amount: Either 20000 for 20ml or 10000 for 10ml
    mypump.write(PUMP_OFF_ANGLE);
    delay(500);  // Small pause to ensure the pump stops
    mypump.detach();
    // PUMP code: End ----------------------------------------------
  }

  Serial.println("..............");
  delay(publishInterval);  // delay for publishing (10 minutes sampling time)
}