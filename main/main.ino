#define relay1 D6
#define relay2 D15 // TODO: conflict with D3, NEED TO BE SOLVED!!!
#define relay3 D2
#define relay4 D4
#define relay5 D8
#define relay6 D7
#define relay7 D3
#define relay8 D5
#define LED 2   // built in led 


#include "credentials.c"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <string.h>




WiFiClient espClient;
PubSubClient client(espClient);

// Tell functions to compiler
void printStatus(bool status, String status_message);
boolean turnOffOn(String message,int Relay);
boolean checkMqttServer();



void setup() {
  // since we just have relay's here 
  // we make all them as output
  pinMode(relay1,OUTPUT);
  pinMode(relay2,OUTPUT);
  pinMode(relay3,OUTPUT);
  pinMode(relay4,OUTPUT);
  pinMode(relay5,OUTPUT);
  pinMode(relay6,OUTPUT);
  pinMode(relay7,OUTPUT);
  pinMode(relay8,OUTPUT);
  pinMode(LED,OUTPUT);  // setup onboard 
  // -------------------- 
  digitalWrite(relay1,HIGH);
  digitalWrite(relay2,HIGH);
  digitalWrite(relay3,HIGH);
  digitalWrite(relay4,HIGH);
  digitalWrite(relay5,HIGH);
  digitalWrite(relay6,HIGH);
  digitalWrite(relay7,HIGH);
  digitalWrite(relay8,HIGH);
  
  
  Serial.begin(115200);


  // get credentials from c function
  // function is declared in credentials.c 
  struct credentials cred = getCredentials();

  // print credentials for debug purposes
  /* Serial.println("credentials");
  Serial.println(cred.ssid);
  Serial.println(cred.password);
  Serial.println(cred.mqtt_host_ip);
  Serial.println(cred.mqtt_port);  */

  // setup wifi
  WiFi.begin(cred.ssid, cred.password);
  

  Serial.print("MAC Addr:\t");
  // print mac address
  Serial.println(WiFi.macAddress());
  
  // connecting to wifi access point
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi..");
    digitalWrite(LED,LOW);
    delay(300);
    digitalWrite(LED,HIGH);
    delay(300);
  }
  Serial.print("Connected to WiFi :");
  Serial.println(WiFi.SSID());
  digitalWrite(LED,LOW);


  // Setup mqtt server config
  client.setServer(cred.mqtt_host_ip, cred.mqtt_port);
  // Setup callback function for mqtt
  client.setCallback(mqtt_callback);

 // connect to mqtt server
  while (!client.connected()) {
    digitalWrite(LED,HIGH);
    Serial.println("Reconnecting to MQTT...");

    if (client.connect("WemosD1R1")) {
 
      Serial.println("connected");
    } else {
 
      Serial.print("failed with state ");
      Serial.println(client.state());  //If you get state 5: mismatch in configuration
      delay(2000);
      digitalWrite(LED,LOW);
      delay(2000);
    }
  }

  // publish a message when device connected to mqtt server
  client.publish("WEMOS_D1_R1", "room2 wemos d1 r1 connected!");

  // subscribe to topics
  client.subscribe("room2/lamp");
  client.subscribe("hall/lamp1");
  client.subscribe("hall/lamp2");
  client.subscribe("hall/lamp3");
  client.subscribe("garden/lamp1");
  client.subscribe("garden/lamp2");
  client.subscribe("garden/lamp3");
  client.subscribe("garden/lamp4");
  
 
}

// reconnect to mqtt server if the connection was interrupted
boolean checkMqttServer(){
  while (!client.connected()) {
    digitalWrite(LED,HIGH);
    Serial.println("Reconnecting to MQTT...");

    if (client.connect("WemosD1R1")) {
 
      Serial.println("connected");
      return true;
    } else {
 
      Serial.print("failed with state ");
      Serial.println(client.state());  //If you get state 5: mismatch in configuration
      delay(2000);
      digitalWrite(LED,LOW);
      delay(2000);
    }
  }
  // would never reach here but in case it reaches here
  return true;
}

// mqtt call back function
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
    
      bool status = turnOffOn(message,relay5);
      printStatus(status, "hall/lamp2 task succeeded!");
    
    } else if ( strstr(topic, "hall/lamp3") ){
    
      bool status = turnOffOn(message, relay6);
      printStatus(status, "hall/lamp3 task succeded!");
    
    } else if ( strstr(topic, "garden/lamp1") ){
    
      bool status = turnOffOn(message,relay3);
      printStatus(status, "garden/lamp1 task succeeded!");
    
    } else if ( strstr(topic, "garden/lamp2") ){
    
      bool status = turnOffOn(message,relay4);
      printStatus(status, "garden/lamp2 task succeded!");
    
    } else if ( strstr(topic, "garden/lamp3") ){

      bool status = turnOffOn(message,relay7);
      printStatus(status, "garden/lamp3 task succeded!");
    
    }else if ( strstr(topic, "garden/lamp4") ){
    
      bool status = turnOffOn(message,relay8);
      printStatus(status, "garden/lamp4 task succeded!");
    
    }else {
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

  // check mqtt connection here
  if(!client.connected())
    checkMqttServer();
}
