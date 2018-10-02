#include "ethernet.h"

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
static int portServer = 80;
IPAddress server(74,125,232,128);
IPAddress ip(192, 168, 0, 177);
EthernetClient client;

uint8_t setupEthernet(){
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");
  
  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("connected");
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
}
/**************************/
void sendSever(String messenger){
  if(client.connect(server, portServer)){
    client.println(messenger);
  }
}
/***********************/
void readSever(){
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }
}

