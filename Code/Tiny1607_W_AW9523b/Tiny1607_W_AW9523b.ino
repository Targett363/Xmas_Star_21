/*
 * Designed for ATtiny1607 or 807 using https://github.com/SpenceKonde/megaTinyCore
 * Using AW9523b for driving the 16 channels of LEDs via i2c from the tiny.
 * Touch input from TTP223-BA6.
 * 
 * Pins:
 * Touch   - Arduino Pin 6,  Port name PB5, Physical 11
 * i2c SCL - Arduino Pin 11, Port name PB0, Physical 16
 * i2c SDA - Arduino Pin 10, Port name PB1, Physical 15
 * AW RSTN - Arduino Pin 9,  Port name PB2, Physical 14
 * 
 */

 #include <Wire.h>

//PIN Names
const int TouchPin = 6;
const int SCLPin = 11;
const int SDAPin = 10;
const int RSTN = 9;

//Global variables
volatile byte prog = 0; //PROGRAM STATE SETTING


void setup() {
  pinMode(TouchPin,INPUT); //Set Touch pin to input mode
  PORTB.PIN5CTRL|=0x02; //Set Touch pin to interrupt on rising edge input

}

void loop() {
  // put your main code here, to run repeatedly:

}

ISR(PORTB_PORT_vect){
  byte flags=PORTB.INTFLAGS; //Read the Port B interrupt flags
  PORTB.INTFLAGS=0x20; //Clear the interrupt flag for Touch pin so as to leave any other interrupt that triggered on Port B flagged for i2c
  if (flags&0x20){ //if it was triggered by pin5
    prog++;                  //then increment the program counter by 1
  }
}
