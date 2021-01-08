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
 * AW9523b i2c address is set as high high so 0b1011011 plus the R/W bit on the end
 * 
 */

 #include <Wire.h>

//PIN Names
const int TouchPin = 6;
const int RSTN = 9;

//AW9523 i2c names
const byte AW9523b = 0x5b; //7bit i2c address for AW9523b with AD0 & AD1 pulled high
const byte resetAddress = 0x7F; //Address on the AW9523b to soft reset
const byte resetByte = 0x00; //Byte to send to the soft reset address to perform reset
const byte ID = 0x10; //Address on the AW9523b to read the deviceID
const byte P0Bank = 0x12; //Address for Port0 LED mode switch
const byte P1Bank = 0x13; //Address for Port1 LED mode switch
const byte BankLEDMode = 0b00000000; //Set ports to all 0 for LED mode
const byte X = 0x24; //Port0 Pin0 address
const byte Y = 0x25; //Port0 Pin1 address
const byte Z = 0x26; //Port0 Pin2 address
const byte XY0 = 0x27; //Port0 Pin3 address
const byte XY1 = 0x28; //Port0 Pin4 address
const byte YZ0 = 0x29; //Port0 Pin5 address
const byte YZ1 = 0x2A; //Port0 Pin6 address
const byte ZX0 = 0x2B; //Port0 Pin7 address
const byte ZX1 = 0x20; //Port1 Pin0 address
const byte SXY0 = 0x21; //Port0 Pin1 address
const byte SXY1 = 0x22; //Port0 Pin2 address
const byte SYZ0 = 0x23; //Port0 Pin3 address
const byte SYZ1 = 0x2C; //Port0 Pin4 address
const byte SZX0 = 0x2D; //Port0 Pin5 address
const byte SZX1 = 0x2E; //Port0 Pin6 address
const byte CENT = 0x2F; //Port0 Pin7 address
/*****LED DIMMING VALUE BETWEEN 0x00 = 0/off AND 0xFF = 255 (255~~37mA)*************/


//Global variables
volatile byte prog = 0; //PROGRAM STATE SETTING
const int maxProg = 7; //Value to loop the prog state back to 0 after x states
bool interrupted = 0;

//Animation arrays
const byte AWRegister[16] = {X,Y,Z,XY0,XY1,YZ0,YZ1,ZX0,ZX1,SXY0,SXY1,SYZ0,SYZ1,SZX0,SZX1,CENT};
const byte anim0[2][16] = {{255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};
// anim1, anim2, animX could all have different primary array size eg:anim1[8][16]

void setup() {
  cli(); //disable interrupts
  pinMode(TouchPin,INPUT); //Set Touch pin to input mode
  PORTB.PIN5CTRL|=0x01; //Set Touch pin to interrupt on rising edge and falling edge input as TTP223 is set to toggle on touch
  sei(); //enable interrupts
  
  pinMode(RSTN, OUTPUT); //Set the AW9523b reset to HIGH to prevent reset state before startup
  digitalWrite(RSTN, HIGH);
  
  Wire.begin();
  

}

void loop() {
  if (prog>maxProg){prog=0;} //setting the wraparound for the prog variable
  
  switch(prog){
    case 0:
      //animation0
      animation(anim0, 2, 100);
      break;
    case 1:
      //animation1
      break;
    case 2:
      //animation2
      break;
    case 3:
      //animation3
      break;
    case 4:
      //animation4
      break;
    case 5:
      //animation5
      break;
    case 6:
      //animation6
      break;
    case 7:
      //animation7
      break;
  }
}


//pass the address of the array, the size of the annimation and the delay between each frame of the animation!!
void animation(byte anim[][16], int sizeanim, int stepDelay){
  /***************USE THIS VERSION IF THE AW9523b TAKES REG-VAL-REG-VAL X16****************/
  while(!interrupted){ //stop doing this animation and return to the "loop" function when the interrupt triggers
    //do the code here
    for (int l=0; l<(sizeanim); l++){
      Wire.beginTransmission(AW9523b);
      for (int i=0;i<16;i++){
        Wire.write(AWRegister[i]);
        Wire.write(anim[l][i]);
      }
      while(!(Wire.endTransmission()==0)){//just wait until the endTransmission completes
      }
      delay(stepDelay);
    }
    /***********Use this one if the AW9523b takes REG-VAL-END REG-VAL-END X16**************
    for (int l=0; l<(sizeanim); l++){
      Wire.beginTransmission(AW9523b);
      for (int i=0;i<16;i++){
        Wire.write(AWRegister[i]);
        Wire.write(anim[l][i]);
        while(!(Wire.endTransmission()==0)){//just wait until the endTransmission completes
        }
      }*/
    delay(stepDelay);
  }
}

ISR(PORTB_PORT_vect){
  cli(); //disable interrupts
  byte flags=PORTB.INTFLAGS; //Read the Port B interrupt flags
  PORTB.INTFLAGS=0x20;       //Clear the interrupt flag for Touch pin so as to leave any other interrupt that triggered on Port B flagged for i2c
  if (flags&0x20){           //if it was triggered by pin5
    interrupted = 1;         //set program interrupted flag to end animation and re-enter the loop
    prog++;                  //then increment the program counter by 1
  }
  sei(); //enable interrupts
}
