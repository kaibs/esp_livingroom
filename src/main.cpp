#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "DHTesp.h"
#include <PubSubClient.h>

//include passwords
#include "credentials.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* mqtt_server = MQTT_SERVER_IP;
const char* user= MQTT_USER;
const char* passw= MQTT_PASSWORD;

WiFiClient espLivingRoom;
PubSubClient client(espLivingRoom);

#define MSG_BUFFER_SIZE	(50)
char msg_temp[MSG_BUFFER_SIZE];
char msg_humidity[MSG_BUFFER_SIZE];


//DHT11 sensor
#define dhtpin 2

DHTesp dht;

//functions

void reconnect() {
 // Loop until we're reconnected
 while (!client.connected()) {
 Serial.print("Attempting MQTT connection...");
 // Attempt to connect
 if (client.connect("espLivingRoom", user, passw)) {
  Serial.println("connected");
  
 } else {
  Serial.print("failed, rc=");
  Serial.print(client.state());
  Serial.println(" try again in 5 seconds");
  // Wait 5 seconds before retrying
  delay(5000);
  }
 }
}

void setup() {
  
  Serial.begin(9600);

  dht.setup(dhtpin, DHTesp::DHT11);

  WiFi.hostname("espLivingRoom");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  client.setServer(mqtt_server, 1883);
  //client.setCallback(callback);

}

void loop() {

  if (!client.connected()){
    reconnect();
  }
  client.loop();

  delay(dht.getMinimumSamplingPeriod());

  
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  
  Serial.print("humidity: "); 
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");




  char helpval[8];
  dtostrf(temperature, 6, 2, helpval);
  snprintf (msg_temp, MSG_BUFFER_SIZE, helpval);
  client.publish("home/livingroom/temperature", msg_temp);
  delay(100);


  dtostrf(humidity, 6, 2, helpval);
  snprintf (msg_humidity, MSG_BUFFER_SIZE, helpval);
  client.publish("home/livingroom/humidity", msg_humidity);
  

  Serial.println("Message sent");
  delay(2000);

}