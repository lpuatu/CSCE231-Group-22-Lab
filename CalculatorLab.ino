#include "cowpi.h"

void keyPressed(uint8_t key);
void displayCurrentNumber();
void displayError();
long operand1 = 0;
long operand2 = 0;
long lastOperand;
int inputNumber = 0;
int operand1Null = 1;
int operand2Null = 1;
int operandPosition = 1;
uint8_t lastOperation = 0x0E;
uint8_t get_key_pressed();


void handle_buttonpress();
void handle_keypress();

volatile unsigned long last_interaction = 0;
volatile uint8_t last_key_pressed = 244;
volatile unsigned long last_time_keypad_pressed = 0xFFFFFFFF;

uint8_t timed_out = 0;

struct gpio_registers *gpio;
struct spi_registers *spi;
unsigned long last_keypad_press = 0;
unsigned long last_left_button_press = 0;
unsigned long last_right_button_press = 0;

const uint8_t seven_segments[16] = {
  0b01111110, 0b00110000, 0b01101101, 0b01111001, 0b00110011,
  0b01011011, 0b01011111, 0b01110000, 0b01111111, 0b01111011,
  0b01110111, 0b00011111, 0b00001101, 0b00111101, 0b01001111, 0b01000111
};


const uint8_t keys[4][4] = {
  {0x01,0x02,0x03,0x0A},
  {0x04,0x05,0x06,0x0B},
  {0x07,0x08,0x09,0x0C},
  {0x0F,0x00,0x0E,0x0D}
};

void setup() {
  Serial.begin(9600);
  gpio = gpio = (gpio_registers *)(IObase+0x03);
  spi = (spi_registers *)(IObase+0x2C);

  attachInterrupt(digitalPinToInterrupt (2), handle_buttonpress , CHANGE );
  attachInterrupt(digitalPinToInterrupt (3), handle_keypress , CHANGE );
  last_interaction = millis(); //TODO: CHANGE

  setup_simple_io();
  setup_keypad();
  setup_display_module();

}

void loop() {
  uint8_t right_button_current_position =  (gpio[D8_D13].input & (1<<1))>>1;
  uint8_t left_button_current_position = gpio[D8_D13].input & (1);
  unsigned long now = millis(); //TODO: CHANGE

  // if(now - last_time_keypad_pressed > 100 && (last_key_pressed >= 0 && last_key_pressed < 16)){
  //   Serial.println("Keypad pressed");
  //   inputDisplay(last_key_pressed);
  //   last_key_pressed = 244;
  //   last_time_keypad_pressed = 0xFFFFFFFF;
  // }

     if ((~(gpio[A0_A5].input)& 0b1111)>0  && (millis() - last_keypad_press > 50)) {
        //keyPressed(get_key_pressed());
        inputDisplay(get_key_pressed());
     }

     if(millis() - last_keypad_press > 500){
      gpio[D8_D13].output &= ~(1<<4);
     }else{
       gpio[D8_D13].output |= 1<<4;
     }

    if (!left_button_current_position && (now - last_left_button_press > 50)) {
      Serial.println("Button left");
    leftButtonPressed();
    last_left_button_press = now;
   }
   if (!right_button_current_position && (now - last_right_button_press > 50)) {
     Serial.println("Button right");
    rightButtonPressed();
    last_right_button_press = now;
   }

  //handle timeout
  if(now - last_interaction > 0xFFFFFF){ // TODO: dynamically get timeout value
    for (char i = 1; i <= 8; i++) {
      display_data(i, 0);
      timed_out = 1;
    }
  }else if(timed_out){
      timed_out = 0;
      displayCurrentNumber();
  }
}

void inputDisplay(uint8_t key){
  if(timed_out){
    return;
  }
  keyPressed(key);
}


void setup_simple_io() {
  gpio[A0_A5].direction &= 0b11001111;
  gpio[A0_A5].output &= 0b11001111;
  gpio[D8_D13].direction &= 0b11111100;
  gpio[D8_D13].output |= 0b00000011;
  gpio[D8_D13].direction |= 0b00010000;
}

void setup_keypad() {
  gpio[D0_D7].direction |= 0b11110000;
  gpio[D0_D7].output &= 0b00001111;
  gpio[A0_A5].direction &= 0b11110000;
  gpio[A0_A5].output |= 0b00001111;
}

void setup_display_module() {
  // Set COPI, SCK, and CS to output
  //00101100
  gpio[D8_D13].direction |=0b00101100;
  spi->control |= 0b01010011;
  spi->control &= 0b11111101;
  // Enable SPI, Controller, set clock rate fck/16
  display_data(0xA, 8);     // intensity at 17/32
  display_data(0xB, 7);     // scan all eight digits
  display_data(0xC, 1);     // take display out of shutdown mode
  display_data(0xF, 0);     // take display out of test mode, just in case
  for (char i = 1; i <= 8; i++) {
    display_data(i, 0);     // clear all digit registers
  }
  display_data(1,seven_segments[0]);
}

uint8_t get_key_pressed() {
  uint8_t key_pressed;
  unsigned long now = millis();
  if (now - last_keypad_press > 500) {
    last_keypad_press = now;
    for(int i=0;i<4;i++){
      gpio[D0_D7].output |= 0b11110000;
      gpio[D0_D7].output &= (~(0b1<<4+i));
//      Serial.println("Row");
//      Serial.println(i,HEX);
      //Serial.println(gpio[D0_D7].output,HEX);
      //for some reason this solves issues
      gpio[D0_D7].output;
      uint8_t col = ~(gpio[A0_A5].input | 0b11110000);
     for(int j = 0;j<4;j++){
        if(col == 0b1<<j){
//          Serial.println("Correct Row");
//          Serial.println(col);
//          Serial.println("FOund");
          key_pressed = keys[i][j];
        }
      }
      gpio[D0_D7].output&=0b00001111;
    }
  }
  return key_pressed;
}

void leftButtonPressed(){
  Serial.println("Negated");
  if(operandPosition ==1){
    operand1 *= -1;
  }
  if(operandPosition == 2){
    operand2 *= -1;
  }
  displayCurrentNumber();
}

void rightButtonPressed(){
  Serial.println("Cleared");
  if(operandPosition == 1){
    operand1 = 0;
    operand1Null = 1;
  }
  if(operandPosition == 2){
    operand2 = 0;
    operand2Null = 1;
  }
  displayCurrentNumber();
}

void keyPressed(uint8_t key){
  int shouldDisplayError = 0;
  Serial.println(key, HEX);
  if(key <=0x09){
    if(operandPosition ==1){
      Serial.println(operand2, HEX);
      if(operand1Null == 1){
        operand1=0;
        operand1Null=0;
      }
      if(abs(operand1)/ 1000000 <1){
        operand1 *=10;
        if(operand1 >=0){
          operand1 += key;
        }else{
          operand1 -= key;
        }
        Serial.println(operand1, HEX);
      }
    }
    if(operandPosition ==2){
      Serial.println(operand2, HEX);
      if(operand2Null == 1){
        operand2=0;
        operand2Null=0;
      }
      if(abs(operand2) / 1000000 <1){
        operand2 *=10;
        if(operand2 >=0){
          operand2 += key;
        }else{
          operand2 -= key;
        }
        Serial.println(operand2, HEX);
      }
    }
  }
  // HANDLE MATH HERE
  if(key >=0x0A && key<=0x0E){
    operandPosition = 2;
    if(lastOperation == 0x0A){
      //handle number greater than max
      if(abs(operand1+operand2)>9999999){
        shouldDisplayError =1;
      }
      operand1 = operand1 + operand2;
      operand2 = 0;
      operand2Null = 1;
    }
    if(lastOperation == 0x0B){
      operand1 = operand1 - operand2;
      operand2 = 0;
      operand2Null = 1;
    }
    if(lastOperation == 0x0C){
      //handle number greater than max
      if(abs(operand1*operand2)>9999999){
        shouldDisplayError =1;
      }
      operand1 = operand1 * operand2;
      operand2 = 0;
      operand2Null = 1;
    }
    if(lastOperation == 0x0D){
      //handle division by zero
      if(operand2 !=0){
      operand1 = operand1 / operand2;
      operand2 = 0;
      operand2Null = 1;
      }else{
        shouldDisplayError =1;
      }
    }
    if(key == 0x0E){
      operandPosition = 1;

    }
    lastOperation = key;
  }
  //
  displayCurrentNumber();
  if(shouldDisplayError){
    displayError();
  }
}

void displayCurrentNumber(){
  for (char i = 1; i <= 8; i++) {
        display_data(i, 0);
  }
  //OPERAND 1
  if(operandPosition == 1){
    //clear input
    if(operand1Null == 1){
      for (char i = 1; i <= 8; i++) {
        display_data(i, 0);
      }
      display_data(1,seven_segments[0]);
    }else{
      long intToDisplay = operand1;
      int indexInDisplay = 1;
      if(operand1<0){
        intToDisplay *=-1;
      }
      do{
        int digitToDisplay = intToDisplay %10;
        display_data(indexInDisplay,seven_segments[digitToDisplay]);
        indexInDisplay+=1;
        intToDisplay /= 10;
      }while(intToDisplay>0);
      if(operand1<0){
        display_data(indexInDisplay,0b00000001);
      }else{
         display_data(indexInDisplay,0);
      }
    }
  }
  //OPERAND 2
  if(operandPosition == 2){
    //clear input
    if(operand2Null == 1){
      for (char i = 1; i <= 8; i++) {
        display_data(i, 0);
      }
       display_data(1,seven_segments[0]);
    }else{
      long intToDisplay = operand2;
      int indexInDisplay = 1;
      if(operand2<0){
        intToDisplay *=-1;
      }
      do{
        int digitToDisplay = intToDisplay %10;
        display_data(indexInDisplay,seven_segments[digitToDisplay]);
        indexInDisplay+=1;
        intToDisplay /= 10;
      }while(intToDisplay>0);
      if(operand2<0){
        display_data(indexInDisplay,0b00000001);
      }else{
         display_data(indexInDisplay,0);
      }
    }
  }
}

void display_data(uint8_t address, uint8_t value) {
  // address is MAX7219's register address (1-8 for digits; otherwise see MAX7219 datasheet Table 2)
  // value is the bit pattern to place in the register
  gpio[D8_D13].output &= 0b11111011;
  spi->data = address;
  while((spi->status & 0b10000000)==0){
  }
  spi->data = value;
  while((spi->status & 0b10000000)==0){
  }
  gpio[D8_D13].output |= 0b00000100;
}

void displayError(){
  operand1 = 0;
  operand1Null = 1;
  operand2 = 0;
  operand2Null = 1;
  operandPosition = 1;
  for (char i = 1; i <= 8; i++) {
        display_data(i, 0);
  }
  display_data(1,0b101);
  display_data(2,0b11101);
  display_data(3,0b101);
  display_data(4,0b101);
  display_data(5,0b1001111);
}


void handle_keypress(){
  Serial.println("keypad");
  unsigned long now = millis(); // TODO: CHANGE
  last_interaction = now;
  last_keypad_press = now;
}

void handle_buttonpress(){
  Serial.println("Button");
  unsigned long now = millis(); // TODO: CHANGE
  last_interaction = now;

  uint8_t right_button_current_position =  (gpio[D8_D13].input & (1<<1))>>1;
  uint8_t left_button_current_position = gpio[D8_D13].input & (1);

  if(!left_button_current_position){
    last_left_button_press = now;
  }

  if(!right_button_current_position){
    last_right_button_press = now;
  }
}