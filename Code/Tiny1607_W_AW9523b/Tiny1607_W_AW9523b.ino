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
const int LED = 2;

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
const int maxProg = 4; //Value to loop the prog state back to 0 after x states
volatile bool interrupted = LOW;

//Animation arrays
const byte AWRegister[16] = {X,Y,Z,XY0,XY1,YZ0,YZ1,ZX0,ZX1,SXY0,SXY1,SYZ0,SYZ1,SZX0,SZX1,CENT};


/*const byte animX[XX][16] = {  //*********Blank Animation*********
  {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}
}*/

const byte anim0[2][16] = { /*********BLINK***********/
  {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255}, //One frame of all on 100% 
  {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}  //then one frame of all OFF
};

const byte anim1[17][16] = { /************TIPS FADE DOWN*********/
  {255,255,255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,255},/*One frame of X,Y,Z & Cent 100%*/
  {239,239,239,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,239},/*Same but 6.3% dimmer*/
  {223,223,223,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,223},/*Same but 6.3% dimmer*/
  {207,207,207,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,207},
  {191,191,191,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,191},
  {175,175,175,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,175},
  {159,159,159,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,159},
  {143,143,143,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,143},
  {127,127,127,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,127},
  {111,111,111,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,111},
  { 95, 95, 95,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 95},
  { 79, 79, 79,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 79},
  { 63, 63, 63,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 63},
  { 47, 47, 47,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 47},
  { 31, 31, 31,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 31},
  { 15, 15, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 15},
  {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0} /*One frame of All OFF*/
};

const byte anim2[17][16] = { /************ALL FADE DOWN*********/
  {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255},/*One frame of ALL 100%*/
  {239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239},/*Same but 6.3% dimmer*/
  {223,223,223,223,223,223,223,223,223,223,223,223,223,223,223,223},/*Same but 6.3% dimmer*/
  {207,207,207,207,207,207,207,207,207,207,207,207,207,207,207,207},
  {191,191,191,191,191,191,191,191,191,191,191,191,191,191,191,191},
  {175,175,175,175,175,175,175,175,175,175,175,175,175,175,175,175},
  {159,159,159,159,159,159,159,159,159,159,159,159,159,159,159,159},
  {143,143,143,143,143,143,143,143,143,143,143,143,143,143,143,143},
  {127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127},
  {111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111},
  { 95, 95, 95, 95, 95, 95, 95, 95, 95, 95, 95, 95, 95, 95, 95, 95},
  { 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79},
  { 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63},
  { 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47},
  { 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31},
  { 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15},
  {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0} /*One frame of All OFF*/
};

const byte anim3[3][16] = {
  {  255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  {  0,  255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  {  0,  0,  255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}
};

//const byte anim0[2][16] = { /*********BLINK***********/
//  {  255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, //One frame of all on 100% 
//  {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}  //then one frame of all OFF
//};
//
//const byte anim1[2][16] = { /*********BLINK***********/
//  {  0,  255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, //One frame of all on 100% 
//  {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}  //then one frame of all OFF
//};
//
//const byte anim2[2][16] = { /*********BLINK***********/
//  {  0,  0,  255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, //One frame of all on 100% 
//  {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}  //then one frame of all OFF
//};

// anim1, anim2, animX could all have different primary array size eg:anim1[8][16]

void setup() {
  cli(); //disable interrupts
  pinMode(TouchPin,INPUT); //Set Touch pin to input mode
  PORTB.PIN5CTRL|=0x01; //Set Touch pin to interrupt on rising edge and falling edge input as TTP223 is set to toggle on touch
  sei(); //enable interrupts
  
  pinMode(RSTN, OUTPUT); //Set the AW9523b reset to HIGH to prevent reset state before startup
  digitalWrite(RSTN, HIGH);
  
  Wire.begin();

  //feedback led for debug
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

}

void loop() {
  if (prog>maxProg){prog=0;} //setting the wraparound for the prog variable
  int lastprog;
  if (lastprog != prog){
    lastprog = prog;
    sendblank();
  }
/*
  //feedback led for debug
  int feed=prog+1;
  for (feed; feed>0; feed--){
    digitalWrite(LED, HIGH);
    delay(250);
    digitalWrite(LED, LOW);
    delay(1000);
  }*/
  
  switch(prog){
    case 0:
      //animation0
      animation(anim0, 2, 250  , 1, 0); //(The Animation Array name, Number of frames in the Animation, ms Delay between frames, number of repeats [-1 is infinate], forward or reverse direction of animation [0==Forward 1=Reverse])
      break;
    case 1:
      //animation1
      //animation(anim1, 17, 500, 1, 0); //Fade Animation forwards
      //animation(anim1, 17, 500, 1, 1); //Fade Animation reverse
      animation(anim0, 2, 1000, 1, 0);
      break;
    case 2:
      //animation2
      //animation(anim2, 17, 500, 1, 0); //Fade Animation forwards
      //animation(anim2, 17, 500, 1, 1); //Fade Animation reverse
      animation(anim0, 2, 500, 1, 0);
      break;
    case 3:
      //animation3
      animation(anim0, 2, 2000, 1, 0);
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

void sendblank(){
  Wire.beginTransmission(AW9523b);
  Wire.write(resetAddress);
  Wire.write(resetByte);
  while(!(Wire.endTransmission()==0)){}
}

//pass the address of the array, the size of the annimation and the delay between each frame of the animation, nimber of times to repeat the animation, forward or reverse play!!
void animation(byte anim[][16], int sizeanim, int stepDelay, int repeats, bool forwardOrReverse){
  /***************USE THIS VERSION IF THE AW9523b TAKES REG-VAL-REG-VAL X16****************/
  //while(interrupted==LOW){ //stop doing this animation and return to the "loop" function when the interrupt triggers
    //do the code here
    //int r=repeats;
    while(repeats != 0){
      if (forwardOrReverse==0){
        for (int l=0; l<=(sizeanim-1); l++){
          Wire.beginTransmission(AW9523b);
          for (int i=0;i<16;i++){
            Wire.write(AWRegister[i]);
            Wire.write(anim[l][i]);
          }
          while(!(Wire.endTransmission()==0)){}//just wait until the endTransmission completes
          delay(stepDelay);
        }
      }
      else{
        for(int l=(sizeanim-1); l>=0; l--){
          Wire.beginTransmission(AW9523b);
          for (int i=0;i<16;i++){
            Wire.write(AWRegister[i]);
            Wire.write(anim[l][i]);
          }
          while(!(Wire.endTransmission()==0)){}//just wait until the endTransmission completes
          delay(stepDelay);
        }
      }
      if(repeats>-1){
        repeats--;
      }
    //}
  
  }
  interrupted=LOW; //resetting the interrupt flag on annimation exit
    /***********Use this one if the AW9523b takes REG-VAL-END REG-VAL-END X16**************
    for (int l=0; l<(sizeanim); l++){
      Wire.beginTransmission(AW9523b);
      for (int i=0;i<16;i++){
        Wire.write(AWRegister[i]);
        Wire.write(anim[l][i]);
        while(!(Wire.endTransmission()==0)){//just wait until the endTransmission completes
        }
      }
    delay(stepDelay);
  }*/
  
}

ISR(PORTB_PORT_vect){
  cli(); //disable interrupts
  byte flags=PORTB.INTFLAGS; //Read the Port B interrupt flags
  PORTB.INTFLAGS=0x20;       //Clear the interrupt flag for Touch pin so as to leave any other interrupt that triggered on Port B flagged for i2c
  if (flags&0x20){           //if it was triggered by pin5
    interrupted = HIGH;      //set program interrupted flag to end animation and re-enter the loop
    prog++;                  //then increment the program counter by 1
  }
  sei(); //enable interrupts
}
