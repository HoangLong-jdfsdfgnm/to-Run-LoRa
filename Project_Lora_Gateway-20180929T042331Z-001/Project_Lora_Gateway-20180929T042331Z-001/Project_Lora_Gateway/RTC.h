#include <Wire.h> 
#include "Arduino.h"



void readDS1307();
int bcd2dec(byte num);
int dec2bcd(byte num);
void digitalClockDisplay();
void printDigits(int digits);
void setTime(byte hr, byte min, byte sec, byte wd, byte d, byte mth, byte yr);
