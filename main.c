#include <avr/io.h>
//#include <util/delay.h>
#include <avr/interrupt.h>
#include <twi_master.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define FOSC      16000000    // clock speed
#define BAUD      9600        // baudrate
#define DAC_ADDR  0x58        // DAC address
#define MYUBRR    FOSC/16/BAUD-1

//function declarations:
void getVoltage();
void getMultiVoltage();
void setVoltage();
void parseCommand();
void USART_Init(unsigned int);
unsigned char USART_Receive( void );
void USART_Transmit( unsigned char );

//Variable declarations
int addr = 0x58; // DAC address
char stringEndMArker = '\n';   //indicates the end of the serial in msg
char charIn;                  //individual character
char charInArray[10];         //array of characters
char command;                 //stores the command from string in

//parse variables
char p1[2];                   //char array for first argument
char p2[4];                   //char array for 2nd argument

void setup() {
  //init i2c
  tw_init(TW_FREQ_400K, true); // set I2C Frequency, enable internal pull-up

  //init serial communication
  //set baud rate
  USART_Init(MYUBRR);
}

//serial recieve interupt
ISR(USART_RX_vect){
  //check characters and or strings and perform necissary actions
  for (int i = 0; i < 10; i++) {
    charIn = USART_Receive; //read string from serial, note, timeout is 1sec
    charInArray[i] = charIn; //save each character
    if(charIn == stringEndMArker){
      i = 10;               //if we hit the end marker, stop filling the array
    }
  }
  //Pick out commands and variables.
  command = charInArray[0];       //the command is stored in position zero on the array
  if(command == 'G'){ getVoltage(); }
  else if(command == 'M'){ getMultiVoltage(); }
  else if(command == 'S'){ setVoltage(); }
}

void loop() {
  delay(100);
}

//Serial init
void USART_Init(unsigned int ubrr){
  /*Set Baud rate*/
  UBRR0H = (unsigned char)(ubrr>>8);
  UBRR0L = (unsigned char)ubrr;
  //Enable reciever and transmitter
  UCSR0B = (1<<RXEN0)|(1<<TXEN0);
  /* Set frame format: 8data, 2stop bit */
  UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}
//Serial recieve routine
unsigned char USART_Receive( void )
{
/* Wait for data to be received */
while ( !(UCSR0A & (1<<RXC0)) )
;
/* Get and return received data from buffer */
return UDR0;
}
//serial transmit routine
void USART_Transmit( unsigned char data ){
  /* Wait for empty transmit buffer */
  while ( !( UCSR0A & (1<<UDRE0)) );

/* Put data into buffer, sends the data */
UDR0 = data;
}

void parseCommand() {
  //Clear work arrays so the string conversions dont get confused (we set them to ascii 0)
  memset(p1, '\0', sizeof(p1));
  memset(p2, '\0', sizeof(p2));
  
  //arrayIndex is shared between the two for loops to conserve position in the charInArray. This is to ensure we can read each argument 
  int arrayIndex = 0;
  for (arrayIndex = 2; charInArray[arrayIndex] != ','; arrayIndex++) {  //saves the first argument
    p1[arrayIndex] = charInArray[arrayIndex];
  }
  //we use next index to offset the p2 array so that we dont go out of bounds
  int nextIndex = arrayIndex;
  for (arrayIndex++; charInArray[arrayIndex] != '\n'; arrayIndex++) {   //this saved the 2nd argument
    p2[arrayIndex - nextIndex] = charInArray[arrayIndex];
  }
}

// Get single voltage from ADC
void getVoltage(){          //print the voltage at the ADC
  // print("V=")
  // print((analogRead(0) * (5.0/1023.0)))
}

// Get multiple voltages from ADC

void getMultiVoltage(){
  /*
  int n;
  int dt;
  parseCommand();
  n = atoi(p1);
  dt = atoi(p2);

  for (int i = 0; i < n; i++) {
    
    Serial.print("V=");
    Serial.print((analogRead(0) * (5.0/1023.0)));
    delay(dt*1000);
  }
  */
}

// Set the voltage of a specific DAC channel
void setVoltage(){
  int c = strtol(p1, NULL, 10);
  float v = strtod(p2, NULL);
  uint8_t n = round(v * 256 / 5); // Convert voltage to 8-bit number
  tw_master_transmit(DAC_ADDR, n, sizeof(n), false);
}
