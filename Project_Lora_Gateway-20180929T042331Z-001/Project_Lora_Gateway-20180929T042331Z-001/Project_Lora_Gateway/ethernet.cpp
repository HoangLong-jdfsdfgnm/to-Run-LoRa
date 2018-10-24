#include "ethernet.h"

byte mac1[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
static int portServer = 80;
IPAddress server1(74,125,232,128);
IPAddress ip1(192, 168, 0, 177);
EthernetClient ether_client;

uint8_t setupEthernet(){
  // start the Ethernet connection:
  if (Ethernet.begin(mac1) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac1, ip1);
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");
  
  // if you get a connection, report back via serial:
  if (ether_client.connect(server1, 80)) {
    Serial.println("connected");
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
}
/**************************/
void sendSever(String messenger){
  if(ether_client.connect(server1, portServer)){
    ether_client.println(messenger);
  }
}
/***********************/
void readSever(){
  if (ether_client.available()) {
    char c = ether_client.read();
    Serial.print(c);
  }
}
