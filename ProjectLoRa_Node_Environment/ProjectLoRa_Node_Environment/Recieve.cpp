#include "Recieve.h"
uint8_t bufferLoRa[25];
uint8_t sizeBuffer = 0;
byte poisition_header  = 0;
byte poisition_end     = 0;
byte poisition_command = 0;
byte poisition_source  = 0;
byte poisition_destination = 0;

void readBufferLoRa(uint8_t bufferLoRa[]){
  sizeBuffer = 0;
  while (LoRa.available()) {
    bufferLoRa[sizeBuffer] = char(LoRa.read());
    Serial.print(char(bufferLoRa[sizeBuffer]));
    sizeBuffer++;
  }
  Serial.println();
}
/*********************************************/
uint8_t haveDataLora(){
  Serial.println(LoRa.parsePacket());
  return LoRa.parsePacket();
}
/*********************************************/
uint8_t sizeBufferLora(){
  return sizeBuffer;
}

/*********************************************/
byte checkFrame(uint8_t buffer_Frame[]){
  int command = -1;
  byte check = 1;
  int Size = sizeBufferLora();
   /* Kiểm tra có ký tự đầu ko */
  for(uint8_t i = 0; i < Size; i++){
    if(buffer_Frame[i] == SYMBOL_HEADER){
//      Serial.println(buffer_Frame[i]);
      poisition_header = i;
      check ++;
    }
  }
  
  if(check != 2){
    return 2;
  }
  Serial.print("poisition_header: ");
  Serial.println(poisition_header);
  /* Kiểm tra có ký tự cuối ko */
  for(uint8_t i = poisition_header; i < Size; i++){
    if(buffer_Frame[i] == SYMBOL_END){
       poisition_end = i;
       check ++;
    }
  }
  if(check != 3){
    return 3;
  }
  Serial.print("poisition_end: ");
  Serial.println(poisition_end);
  /* Number Node*/
  uint8_t numberNode = buffer_Frame[poisition_header + 4];
//  Serial.println(numberNode);
  if(numberNode !=  49)
    return 4;
  /* kiem tra ban tin */
//  for (byte i = 0; i < Size; i++){
//    if()
//  }
  /* Kiểm tra lệnh */
  switch (buffer_Frame[poisition_header + 6]){
    case 'J':
      command = JOIN;
      break;
    case 'A':
      command = ACCEPT;
      break;
    case 'C':
      command = CONFIGURE;
      break;
    case 'I':
      command = IMPORMATION;
      break;
//    case 'S':
//      command = STATE;
      break;
    case 'T':
      command = TEMPERATURE;
      break;
    case 'L':
      command = LIGHT;
      break;
    case 'H':
      command = HUMIDITY;
      break;
    case 'D':
      break;
    default:
      command = 0;
//      Serial.println("Lenh ko dung!");
      return 5;
  }
  //
  return 0;
}

/****************************/
void recieveCommandLoRa(uint8_t bufferLoRa[]){
  return bufferLoRa[6];
}
/****************************/
void recieveTypeCommand(uint8_t bufferLoRa[]){
  return bufferLoRa[8];
}

//////////////////////////////////////////////////////////
//void determilPoisition(){
//  positionHeader();
//  positionComma1();
//  positionSource();
//  positionComma2();
//  positionDesition();
//  positionComma3();
//  positionCommand();
//  positionDevide();
//  positionComma4();
//  positionExclamation();
//}
