#include "Arduino.h"

//#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>


void setupMQTT();
bool subscribeMQTT(String topic);
void publishMQTT(String messenger);
