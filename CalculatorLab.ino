#include "cowpi.h"

char keys[4][4] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

int rPins[4] = {13, 12, 10, 9};
int cPins[4] = {A0, A1, A2, A3};

long displayNumber = 0;
long operand1 = 0;
long operand2;

void setup() {

  //Set Pin Modes!
  for(int i = 0; i < 4; i++){
    pinMode(rPins[i], INPUT);
    digitalWrite(rPins[i], HIGH);
  }
  for(int k = 0; k < 4; k++){
    pinMode(cPins[k], OUTPUT);
    digitalWrite(cPins[k], HIGH);
  }
  // -Logan

}

void loop() {
  char digit = getPressed();
  displayNumber = getNumber(digit);
  Serial.println(digit);
  Serial.println(displayNumber);
  
  
}

//Checks each row and column for the key pressed
char getPressed(){
  char digit = 0;
  for(int k = 0; k < 4; k++){
    digitalWrite(cPins[4], LOW);
    
    for(int i = 0; i < 4; i++){
      if(digitalRead(rPins[i]) == LOW){
        delay(100);
        while(digitalRead(rPins[i] = LOW));
        digit = keys[i][k];
      }
    }
    
    digitalWrite(cPins[k], HIGH);
  }
  
  return digit;
}
// -Logan

int getNumber(char digit){
  if(digit == '1') displayNumber = (displayNumber*10) + 1;
  if(digit == '2') displayNumber = (displayNumber*10) + 2;
  if(digit == '3') displayNumber = (displayNumber*10) + 3;
  if(digit == '4') displayNumber = (displayNumber*10) + 4;
  if(digit == '5') displayNumber = (displayNumber*10) + 5;
  if(digit == '6') displayNumber = (displayNumber*10) + 6;
  if(digit == '7') displayNumber = (displayNumber*10) + 7;
  if(digit == '8') displayNumber = (displayNumber*10) + 8;
  if(digit == '9') displayNumber = (displayNumber*10) + 9;
  if(digit == '0') displayNumber = (displayNumber*10) + 0;
  
}
