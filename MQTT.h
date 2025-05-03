#ifndef MQTT_h
#define MQTT_h

static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "oled.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

//extern Adafruit_SSD1306 display;

WiFiClientSecure espClient;
PubSubClient client(espClient);

// Replace with your WiFi network credentials
extern const char* ssid;
extern const char* password;

// MQTT Broker Credential
extern const char* mqtt_server;
extern const char* mqtt_username;
extern const char* mqtt_password;
extern const int mqtt_port;

//"aruco/lulu" for aruco marker #4, 8  //"aruco/lulu2" for aruco marker #3, 5 //
String payload;
extern String tempTopic;
extern float x_robot;
extern float y_robot;
extern float z_ang_robot;

String get_wifi_status(int status){
    switch(status){
        case WL_IDLE_STATUS:
        return "WL_IDLE_STATUS";
        case WL_SCAN_COMPLETED:
        return "WL_SCAN_COMPLETED";
        case WL_NO_SSID_AVAIL:
        return "WL_NO_SSID_AVAIL";
        case WL_CONNECT_FAILED:
        return "WL_CONNECT_FAILED";
        case WL_CONNECTION_LOST:
        return "WL_CONNECTION_LOST";
        case WL_CONNECTED:
        return "WL_CONNECTED";
        case WL_DISCONNECTED:
        return "WL_DISCONNECTED";
    }
}

void callback(char* topic, byte* message, unsigned int length) {
  // Serial.print("Message arrived on topic: ");
  // Serial.print(topic);
  // Serial.print(". Message: ");
  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
  int i1 = messageTemp.indexOf(',');
  int i2 = messageTemp.indexOf(',', i1+1);
  String firstValue  = messageTemp.substring(0, i1);
  String secondValue = messageTemp.substring(i1 + 1, i2);
  String thirdValue  = messageTemp.substring(i2 + 1);

  x_robot     =  firstValue.toFloat();
  y_robot     = secondValue.toFloat();
  z_ang_robot =  thirdValue.toFloat();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    display.clearDisplay();
    display.setCursor(10, 20);
    display.println("Attempting MQTT..");
    display.display();
    
    String clientId = "ESP";   // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      display.setCursor(10, 30);
      Serial.println("Connected");
      display.println("Connected");
      display.display();
      display.setCursor(10, 40);
      display.println(client.connected());
      display.display();
      delay(5000);
    }
  }
}
extern bool grab_now = false;
extern char pubString1[8];

void wifi_mqtt_init(){
  //WIFI connection status checking
  WiFi.mode(WIFI_STA);
  int status = WL_IDLE_STATUS;
  
  WiFi.begin(ssid, password);
  //Serial.println(get_wifi_status(status));
  //Serial.print("Connecting to WiFi ..");
  display.setCursor(10, 0);
  display.println("Connect WiFi..");
  display.display();
  while (WiFi.status() != WL_CONNECTED){
    display.clearDisplay();
    display.setCursor(10,30);
    display.print('.');
    //Serial.print('.');
    status = WiFi.status();
    display.setCursor(10, 10);
    display.println(get_wifi_status(status));
    display.display();
    //Serial.println(get_wifi_status(status));
    delay(1000);
  }
  display.setCursor(10,40);
  display.print('Connected WiFi');
  Serial.println("Connected WiFi");
  display.display();
  delay(1000);
  
  Serial.println("\nConnected to the WiFi network");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());
  
  espClient.setCACert(root_ca);      // enable this line and the the "certificate" code for secure connection
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.loop();
  Serial.println("MQTT Connected");
  client.subscribe(tempTopic.c_str(),0);
}

void mqtt_clean(){
  for(int i = 0; i<100; i++){
    client.loop();
  }
}

void mqtt_rebound(){
  if (!client.connected()) {
      reconnect();
      client.subscribe(tempTopic.c_str(),0);
  }
}

#endif
