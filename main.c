#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <i2cmaster.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FOSC      16000000    // clock speed
#define BAUD      9600        // baudrate
#define DAC_ADDR  0b01011000  // DAC address
#define MYUBRR    FOSC/16/BAUD-1

/* ============== Function Declarations ============== */
void getVoltage();
void getMultiVoltage();
void setVoltage();
void parseCommand();
unsigned char USART_Receive( void );
void USART_Transmit( unsigned char[] );

/* ============== Variable Declarations ============== */
int addr = 0x58;        // DAC address
char charInArray[10];   // array of characters
char command;           // stores the command from string in
char p1[2];             // char array for first argument
char p2[4];             // char array for 2nd argument

/* ====================== Setup ====================== */

/* Initialization */
void setup() {
  i2c_init();   // Init I2C
  
  // Init serial communication
  unsigned int ubrr = MYUBRR;
  UBRR0H = (unsigned char) (ubrr>>8);  // Set Baud rate
  UBRR0L = (unsigned char) ubrr;
  UCSR0B = (1<<RXEN0)|(1<<TXEN0);       // Enable reciever and transmitter
  UCSR0C = (1<<USBS0)|(3<<UCSZ00);      // Set frame format: 8data, 2stop bit
}

/* ========== Serial Communication Routines ========== */

//Serial recieve routine
unsigned char USART_Receive( void ) {
  while ( !(UCSR0A & (1<<RXC0)) ) ;     // Wait for data to be received
  return UDR0;                          // Get and return received data from buffer
}

//serial transmit routine
void USART_Transmit( unsigned char data[] ){
  for (int i = 0; i < strlen(data); i++) {
    while ( !( UCSR0A & (1<<UDRE0)) );  // Wait for empty transmit buffer
    UDR0 = data[i];                        // Put data into buffer, sends the data
  }
}

/* ============ Command Handling Routines ============ */

/* Parse input command parameters */
void parseCommand() {
  // Clear previous command arguments for new command
  memset(p1, 0, sizeof(p1));
  memset(p2, 0, sizeof(p2));

  // Store first argument; start after first delimiter, stop at next delimiter ','
  int arrayIndex; //Note, we initialize arrayIndex outside of the scope because its value is shared between the two foor loops
  for (arrayIndex = 2; charInArray[arrayIndex] != ','; arrayIndex++) {
    p1[arrayIndex-2] = charInArray[arrayIndex];
  }

  // Store second argument; start after second delimeter, stop at newline '\n'
  int nextIndex = ++arrayIndex;   // store start index of second argument
  for (; charInArray[arrayIndex] != '\n'; arrayIndex++) {
    p2[arrayIndex - nextIndex] = charInArray[arrayIndex];
  }
}

/* Get single voltage from ADC */
void getVoltage(){
  ADMUX = 0b01000000;       // Select ADC0
  ADCSRA = 0b11000111;      // Enable ADC, start conversion, prescaler = 128
  while(ADCSRA & (1<<ADSC));  // Wait for conversion to finish
  float voltage = ((float)ADC * 5.0)/1023.0;  // Convert ADC value to voltage
  unsigned char string[64];
  dtostrf(voltage, 4, 2, string);
  USART_Transmit("V=");
  USART_Transmit(string);
  USART_Transmit("\n");
}

/* Get multiple voltages from ADC */
void getMultiVoltage(){
  parseCommand();
  int n = strtol(p1, NULL, 10);
  int dt = strtol(p2, NULL, 10);
  for (int i = 0; i < n; i++) {
    getVoltage();
    for (int j = 0; j < dt; j++) _delay_ms(1000);
  }
}

/* Set the voltage of a specific DAC channel */
void setVoltage(){
  USART_Transmit("SetingV\n");
  i2c_start(DAC_ADDR);  // Start I2C communication
  int c = strtol(p1, NULL, 10);
  i2c_write(c);         // Send channel number
  int n = round(strtod(p2, NULL) * 255.0 / 5.0); // Convert voltage to 8-bit number
  i2c_write(n);         // Send voltage
  i2c_stop();           // Stop I2C communication
}

/* Main Loop */
void loop() {
  for (int i = 0; i < 10; i++) {
    charInArray[i] = USART_Receive();      // save each character
    if(charInArray[i] == '\n') break;     // stop reading if newline char detected
  }
  // Handle different commands based on first character of input
  command = charInArray[0];
  if(command == 'G'){ getVoltage(); }
  else if(command == 'M'){ getMultiVoltage(); }
  else if(command == 'S'){ setVoltage(); }
}
