#include "EEPROM.h"


uint8_t read_EEPROM(int addr){
  uint8_t value = 0;
  EEPROM.read(addr);
  delay(5);
  return value;
}

void write_EEPROM(int addr, uint8_t val){
  EEPROM.write(addr, val);
  delay(5);
}

void delete_Node(){
  for(uint16_t i = 0; i<512; i++){
    EEPROM.write(i, 0);
    delay(5);
  }
}
