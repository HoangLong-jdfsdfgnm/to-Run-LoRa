#include "Arduino.h"
uint16_t timeOut = 10000;

uint8_t readBufferSerial();
uint8_t sizeofBuf();
uint8_t determineCommandSerial();
uint8_t getLight(uint8_t equal, uint8_t questMark);
uint8_t getTemperature(uint8_t equal, uint8_t questMark);
uint8_t getHumidity(uint8_t equal, uint8_t questMark);
uint8_t getGroundHumidity(uint8_t equal, uint8_t questMark);
uint8_t getBattery(uint8_t equal, uint8_t questMark);
uint8_t getPowerLoRa(uint8_t equal, uint8_t questMark);
uint8_t setupTimer(uint16_t millis);
uint8_t wattingReponse(uint8_t commandSerial);
uint8_t determineParameter(uint8_t index, uint8_t type, uint8_t equal, uint8_t lineFeed);
void resetNode(uint8_t equal, uint8_t lineFeed);
void setFreqJoin(uint8_t equal, uint8_t lineFeed);
void setFreqAccept(uint8_t equal, uint8_t lineFeed);
void setPowerLoRa(uint8_t equal, uint8_t lineFeed);
void setDataRateLoRa(uint8_t equal, uint8_t lineFeed);
void setID(uint8_t equal, uint8_t lineFeed);
uint8_t getState(uint8_t equal, uint8_t questMark);
void controlRole(uint8_t equal, uint8_t lineFeed);
void resetNode(uint8_t equal, uint8_t lineFeed);
uint8_t readAddress(uint8_t equal, uint8_t lineFeed);
void LoRa_transmit(String messenger);
uint8_t readDataLoRa(uint8_t command);
uint8_t bufferLoRaAvailable();
