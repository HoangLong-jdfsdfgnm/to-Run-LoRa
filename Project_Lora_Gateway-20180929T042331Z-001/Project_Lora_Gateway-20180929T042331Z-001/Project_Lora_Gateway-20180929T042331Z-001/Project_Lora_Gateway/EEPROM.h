#include "Arduino.h"
#include <EEPROM.h>
//------------------ DEFINE
#define ADDR_ADDRESS                0
#define ADDR_TEMPERATURE            ADDR_ADDRESS + 40
#define ADDR_HUMIDITY               ADDR_TEMPERATURE + 40
#define ADDR_LIGHT                  ADDR_HUMIDITY + 40
#define ADDR_GROUND_HUMIDITY        ADDR_LIGHT + 40
#define ADDR_ROLE_1                 ADDR_GROUND_HUMIDITY +40
#define ADDR_ROLE_2                 ADDR_ROLE_1 + 40
#define ADDR_ROLE_3                 ADDR_ROLE_2 + 40

//-------------------------

uint8_t read_EEPROM(int addr);
void write_EEPROM(int addr, uint8_t val);
void delete_Node();
void delete_all();
