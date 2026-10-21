#include "WiFi.h"
#include "PubSubClient.h"
#include <FastLED.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>
#include <DHT.h>

char clientID[50];

const int dhtPin = 4;
DHT dht(dhtPin , DHT22);

const int ledPin = 5;
CRGB leds[1];

WiFiClient espClient;
PubSubClient client(espClient);

const int pirPin  = 15;
char valPIR;

const int trigPin = 27;
const int echoPin = 26;

String phase = "OFF";
int timeX;
int distance;
void setup(){
  Serial.begin(9600);
  dht.begin();
  pinMode(pirPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin , INPUT);
  FastLED.addLeds<NEOPIXEL, ledPin>(leds,  1);

  WiFi.mode(WIFI_STA);
  WiFi.begin("Wokwi-GUEST", "");

  client.setServer("broker.emqx.io", 1883);
  client.setCallback(callback);
}

void mqttReconnect(){
  while(!client.connected()){
    Serial.println("Attempting to connect MQTT");
    if(client.connect(clientID)){
      Serial.println("Connected");
      client.subscribe("topicName/iot");
    }
    else{
      Serial.println("Connection failed try again in 5 seconds");
      delay(5000);
    }
  }
  
}

void callback(char* topic, byte* message, unsigned int length)
{
  String stMessage;
  for (int i = 0; i < length; i++)
  {
    stMessage += (char)message[i];
  }
  if(stMessage == "1"){
    leds[0] = CRGB::GreenYellow;
    FastLED.show();
    Serial.println("Submarine is ON");
    phase = "ON";

  }
  if(stMessage == "2"){
    if (phase == "ON"){
      valPIR = digitalRead(pirPin);
      if(valPIR == HIGH){
        client.publish("topicName/iot" , "Object detected");
      }
      else{
        client.publish("topicName/iot" , "Object not detected");
      }
    }
    else{
      client.publish("topicName/iot" , "Submarine is OFF");
    }

    
  }
  if(stMessage == "3"){
    digitalWrite(trigPin, LOW);
    digitalWrite(trigPin , HIGH);

    timeX = pulseIn(echoPin , HIGH);
    distance = 0.0166*timeX;

  }
  if(stMessage == "4"){
    float temperature = dht.readTemperature();
    Serial.println(temperature);
  }
  if(stMessage == "5"){
    leds[0] = CRGB::CRGB::Red;
    FastLED.show();
    Serial.println("Submarine is ON");
  }
}
void loop(){

  if(!client.connected()){
    mqttReconnect();
  }
  client.loop();

  delay(1000);
  }


