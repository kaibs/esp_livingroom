#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "DHTesp.h"
#include <PubSubClient.h>
#include "RCSwitch.h"

//include passwords
#include "credentials.h"
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* mqtt_server = MQTT_SERVER_IP;
const char* user= MQTT_USER;
const char* passw= MQTT_PASSWORD;

//Wifi
WiFiClient espBedRoom;
PubSubClient client(espBedRoom);

//mqtt messages
#define MSG_BUFFER_SIZE	(50)
char msg_temp[MSG_BUFFER_SIZE];
char msg_humidity[MSG_BUFFER_SIZE];
String receivedString;

//FS1000A RF-Transmitter
#define rfpin 2
RCSwitch transmitter = RCSwitch();
int repetitions = 10;

//DHT11 sensor
#define dhtpin 4
DHTesp dht;

//timer for sensor-publish
unsigned long timer = 0;
const long interval = 20000; //20 Sek.


//-------------------------------------functions----------------------------------


void sendSignal(char tristate[]){
  for (int i=0;i<repetitions;i++){
    transmitter.sendTriState(tristate);
    delay(40);
  }
}


//callback for receiving MQTT
void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

 if (strcmp(topic,"home/bedroom/ambilight/switch")==0){
  
  for (int i=0;i<length;i++) {

    receivedString += (char)payload[i];
  
    if (receivedString == "ON"){
      sendSignal("F0FFFF0FFFFF");
      Serial.println("Ambilight ON");
    }

    if (receivedString == "OFF"){
      sendSignal("F0FFFF0F0000");
      Serial.println("Ambilight OFF");
    }
  }
  receivedString = "";
 }

 if (strcmp(topic,"home/bedroom/heartlight/switch")==0){
  
  for (int i=0;i<length;i++) {

    receivedString += (char)payload[i];
  
    if (receivedString == "ON"){
      sendSignal("FF0FF0FFFFFF");
      Serial.println("Heartlight ON");
    }

    if (receivedString == "OFF"){
      sendSignal("FF0FF0FF0000");
      Serial.println("Heartlight OFF");
    }
  }

  receivedString = "";
 }
} 


//reconnect for mqtt-broker
void reconnect(){
 // Loop until we're reconnected
 while (!client.connected()) {
 Serial.print("Attempting MQTT connection...");
 // Attempt to connect
 if (client.connect("espBedRoom", user, passw)) {
  Serial.println("connected");

  //subscribe
  client.subscribe("home/bedroom/ambilight/switch");
  client.subscribe("home/bedroom/heartlight/switch");
  
 } else {
  Serial.print("failed, rc=");
  Serial.print(client.state());
  Serial.println(" try again in 5 seconds");
  // Wait 5 seconds before retrying
  delay(5000);
  }
 }
}


//-------------------------------MAIN------------------------------------------------
void setup() {
  
  Serial.begin(9600);

  //DHT11
  dht.setup(dhtpin, DHTesp::DHT11);

  //Wifi
  WiFi.hostname("espBedRoom");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  //mqtt
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  //rf
  transmitter.enableTransmit(rfpin);
  transmitter.setProtocol(2);
  transmitter.setPulseLength(320);

}

void loop() {

  //check mqtt-connection
  if (!client.connected()){
    reconnect();
  }
  client.loop();

  //timer
  unsigned long currentTime = millis();

  if (currentTime - timer >= interval){

    Serial.println("send data");

    float humidity = dht.getHumidity();
    float temperature = dht.getTemperature();

    char helpval[8];
    dtostrf(temperature, 6, 2, helpval);
    snprintf (msg_temp, MSG_BUFFER_SIZE, helpval);
    client.publish("home/bedroom/temperature", msg_temp);
    delay(100);

    dtostrf(humidity, 6, 2, helpval);
    snprintf (msg_humidity, MSG_BUFFER_SIZE, helpval);
    client.publish("home/bedroom/humidity", msg_humidity);

    timer = currentTime;

  }
}