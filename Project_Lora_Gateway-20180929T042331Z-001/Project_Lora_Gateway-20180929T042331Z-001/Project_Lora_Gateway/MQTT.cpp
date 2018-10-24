#ifndef DEBUG
//  #define DEBUG
#endif

#include "MQTT.h"

#define MQTTPUBLISH     3 << 4

EthernetClient ethClient1;
PubSubClient client(ethClient1);
extern uint8_t bufferSerial[25];

byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 5, 201);
IPAddress server(172, 168, 5, 2);
extern unsigned int lengthPayload;

void callback(char* topic, byte* payload, unsigned int length) {
  lengthPayload = length;
//  Serial.println("xoa bufferSerial");
  for (int i=0;i<25;i++) {
    bufferSerial[i] = 0;
  }
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
    bufferSerial[i] = payload[i];
  }
#ifdef DEBUG
  Serial.print("bufferSerial: ");
  for (int i=0;i<length;i++) {
    Serial.print((char)bufferSerial[i]);
  }
  
  Serial.println();
  Serial.println(payload[length -1]);
  Serial.print("lengthPayload: ");
  Serial.println(lengthPayload);
#endif

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connect ("d30f5e8427a74ea9ac7de613a68ddce7", "jdf1", "123456")) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
//    if (client.connect ("d30f5e8427a74ea9ac7de613a68ddce7", "onnpbmkx", "-v2-KVvP42tY")) {
    if (client.connect ("d30f5e8427a74ea9ac7de613a68ddce7", "jdf1", "123456")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("light","hello world");
      // ... and resubscribe
      client.subscribe("light");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setupMQTT()
{
  pinMode(53, OUTPUT);
  digitalWrite(53, LOW);
  pinMode(11, OUTPUT);
  digitalWrite(11, HIGH);

  client.setServer("m15.cloudmqtt.com", 13590);
  client.setCallback(callback);
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
//      while (true) {
//        delay(1); // do nothing, no point running without Ethernet hardware
//      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }
  
//  Ethernet.begin(mac, ip);
  // Allow the hardware to sort itself out
  delay(1500);
}
bool subscribeMQTT(String topic){
  char bufTopic[20];
  topic.toCharArray(bufTopic, 20);
  if (!client.connected()) {
    Serial.println("reconnect");
    reconnect();
    client.subscribe(bufTopic);
  }
  else{
    client.loop();
  }
  if(lengthPayload != 0){
    return 1;
  }
  else
    return 0;
}

void publishMQTT(String messenger){
  char buf[100];
  messenger.toCharArray(buf, 100);
  client.publish("light",buf);
}
