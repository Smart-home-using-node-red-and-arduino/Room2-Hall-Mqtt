#define relay1 D6
#define relay2 D15
#define relay3 D2
#define relay4 D4
#define relay5 D8
#define relay6 D7
#define dht1_pin D14
#define DhtType DHT22

#include "credentials.c"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <string.h>
#include <DHT.h> // to get sensors data
#include <Ticker.h> // ticker library to control dht22 sensors




WiFiClient espClient;
PubSubClient client(espClient);
Ticker check_Mqtt_connection; // Check if the connection is lost try to reconnect ( maybe mqtt server was shutdown unexpectedly or restarted! )

// Tell functions to compiler
void printStatus(bool status, String status_message);
boolean turnOffOn(String message,int Relay);
void checkMqttServer();



void setup() {
  // since we just have relay's here 
  // we make all them as output
  pinMode(relay1,OUTPUT);
  pinMode(relay2,OUTPUT);
  pinMode(relay3,OUTPUT);
  pinMode(relay4,OUTPUT);
  pinMode(relay5,OUTPUT);
  pinMode(relay6,OUTPUT);
  
  Serial.begin(115200);

  check_Mqtt_connection.attach(180, checkMqttServer); // check mqtt server connection every 3 minutes (3*60 seconds)

  // get credentials from c function
  // function is declared in credentials.c 
  struct credentials cred = getCredentials();

  // print credentials for debug purposes
  /* Serial.println("credentials");
  Serial.println(cred.ssid);
  Serial.println(cred.password);
  Serial.println(cred.mqtt_host_ip);
  Serial.println(cred.mqtt_port);  */
 
  WiFi.begin(cred.ssid, cred.password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.print("Connected to WiFi :");
  Serial.println(WiFi.SSID());
 
  client.setServer(cred.mqtt_host_ip, cred.mqtt_port);
  client.setCallback(mqtt_callback);
 
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

    if (client.connect("nodeMcu")) {
 
      Serial.println("connected");  
 
    } else {
 
      Serial.print("failed with state ");
      Serial.println(client.state());  //If you get state 5: mismatch in configuration
      delay(2000);
 
    }
  }
 
  client.publish("WEMOS_D1_R1", "room2 wemos d1 r1 connected!");
  client.subscribe("room2/lamp");
  client.subscribe("hall/lamp1");
  client.subscribe("hall/lamp2");
  client.subscribe("hall/lamp3");
  client.subscribe("garden/lamp1");
  client.subscribe("garden/lamp2");
  
 
}

void checkMqttServer(){
  while (!client.connected()) {
    Serial.println("Reconnecting to MQTT...");

    if (client.connect("nodeMcu")) {
 
      Serial.println("connected");
    } else {
 
      Serial.print("failed with state ");
      Serial.println(client.state());  //If you get state 5: mismatch in configuration
      delay(10000);
    }
  }
}
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
 
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
   
    Serial.print("Message:");

    // retrieve the message from payload
    String message;
    for (int i = 0; i < length; i++) {
      message = message + (char)payload[i];  //Conver *byte to String
    }
//   Serial.print(message); // print the message, For debug purposes


  // get the topic sent from mqtt server 
  // And change the status of relays

    if ( strstr(topic, "room2/lamp") ){
      
      bool status = turnOffOn(message,relay1);
      printStatus(status, "room2 task succeeded!");
    
    }else if( strstr(topic, "hall/lamp1") ){
    
      bool status = turnOffOn(message, relay2);
      printStatus(status, "hall/lamp1 task succeded!");
    
    } else if( strstr(topic, "hall/lamp2") ){
    
      bool status = turnOffOn(message,relay3);
      printStatus(status, "hall/lamp2 task succeeded!");
    
    } else if ( strstr(topic, "hall/lamp3") ){
    
      bool status = turnOffOn(message, relay4);
      printStatus(status, "hall/lamp3 task succeded!");
    
    } else if ( strstr(topic, "garden/lamp1") ){
    
      bool status = turnOffOn(message,relay5);
      printStatus(status, "garden/lamp1 task succeeded!");
    
    } else if ( strstr(topic, "garden/lamp2") ){
    
      bool status = turnOffOn(message,relay6);
      printStatus(status, "garden/lamp2 task succeded!");
    
    } else {
      Serial.println("Unsuppoted topic");
      Serial.print("Sent Topic: ");
      Serial.println(topic);
    }
 
  Serial.println();
  Serial.println("-----------------------------------------");
}

boolean turnOffOn(String message,int Relay){
  if(message == "#on") {
     digitalWrite(Relay,LOW);
     Serial.println("room1 lamp turned on");
     return true;
  }
  else if(message == "#off") {
     digitalWrite(Relay,HIGH);
     return true;
     Serial.println("room1 lamp turned off");
  }else{
     Serial.print("Unsupported query: ");
     Serial.println(message);
     return false;
  }
}
void printStatus(bool status, String status_message){
  if (status) {
    Serial.println(status_message);
  }
  else {
    Serial.println("Unsupported query");
  }
}
void loop() {
  client.loop();
}
