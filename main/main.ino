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
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <string.h>




WiFiClient espClient;
PubSubClient client(espClient);

// Tell functions to compiler
void printStatus(bool status, String status_message);
boolean turnOffOn(String message,int Relay);
boolean checkMqttServer();
void checkDisconnectedTime(unsigned int currentTime, unsigned int previousMillis, unsigned int restart_interval);


void setup() {


  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println("Booting");
  
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


  // Codes for Arduino Over the Air programing (OTA)
  
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();


  // Setup mqtt server config
  client.setServer(cred.mqtt_host_ip, cred.mqtt_port);
  // Setup callback function for mqtt
  client.setCallback(mqtt_callback);

  // get previous millis, so we can restart if the board was disconneted for long time
  unsigned long previousMillis = millis();

 // connect to mqtt server
  while (!client.connected()) {

    // if the connection to mqtt server was not stablished for 10 minutes, restart board
    // Note here we set 10 minutes as a long time descibed above!
    checkDisconnectedTime(millis() , previousMillis, 600000);

    
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
  client.publish("WEMOS_D1_R1", "room2 ESP8266_WEMOS_D1_R1");

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
  
  // get previous millis, so we can restart if the board was disconneted for long time
  unsigned long previousMillis = millis();
  
  
  while (!client.connected()) {
    // if the connection to mqtt server was not stablished for 10 minutes, restart board
    // Note here we set 10 minutes as a long time descibed above!
    checkDisconnectedTime(millis() , previousMillis, 600000);

    // this line is to make built in led behaviour as blinking
    digitalWrite(LED,HIGH);
    Serial.println("Reconnecting to MQTT...");

    if (client.connect("WemosD1R1")) {
 
      Serial.println("connected");
      return true;
    } else {
 
      Serial.print("failed with state ");
      Serial.println(client.state());  //If you get state 5: mismatch in configuration
      delay(2000);
      
      // this line is to make built in led behaviour as blinking
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
void checkDisconnectedTime(unsigned int currentTime, unsigned int previousMillis, unsigned int restart_interval){
    // restart if the disconnected time went for more than 10 minutes
    if( currentTime - previousMillis > restart_interval ){
      ESP.restart();
    } else return;
}

void loop() {
  ArduinoOTA.handle(); // Handle OTA connection
  client.loop();

  // check mqtt connection here
  if(!client.connected())
    checkMqttServer();
}
