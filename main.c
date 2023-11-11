#include <avr/io.h>
#include <util/delay.h>
#include <i2cmaster.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define FOSC      16000000    // clock speed
#define BAUD      9600        // baudrate
#define MYUBRR    FOSC/16/BAUD-1

/* ============== Function Declarations ============== */
void getVoltage();
void getMultiVoltage();
void setVoltage();
void parseCommand();
unsigned char USART_Receive( void );
void USART_Transmit( unsigned char[] );

/* ============== Variable Declarations ============== */
char inStr[10];   // array of characters
char p2[4];       // char array for 2nd argument
char p1[2];       // char array for first argument

/* ====================== Setup ====================== */
void setup() {
  i2c_init();                         // Init I2C communication
  unsigned int ubrr = MYUBRR;         // Init serial communication
  UBRR0H = (unsigned char) (ubrr>>8); // Set Baud rate high byte
  UBRR0L = (unsigned char) ubrr;      // Set Baud rate low byte
  UCSR0B = (1<<RXEN0)|(1<<TXEN0);     // Enable reciever and transmitter
  UCSR0C = (1<<USBS0)|(3<<UCSZ00);    // Set frame format: 8data, 2stop bit
}

/* ========== Serial Communication Routines ========== */
// Receive serial data
unsigned char USART_Receive( void ) {
  while ( !(UCSR0A & (1<<RXC0)) ) ;     // Wait for data to be received
  return UDR0;                          // Get and return received data from buffer
}
// Transmit serial data
void USART_Transmit( unsigned char data[] ){
  for (int i = 0; i < strlen((char*)data); i++) {
    while ( !( UCSR0A & (1<<UDRE0)) );        // Wait for empty transmit buffer
    UDR0 = data[i];                           // Put data into buffer, sends the data
  }
}

/* ============ Command Handling Routines ============ */

/* Parse input command parameters */
void parseCommand() {
  memset(p1, 0, sizeof(p1));  // Clear previous p1
  memset(p2, 0, sizeof(p2));  // Clear previous p2
  int arrIdx; // Walking index scoped for use in both loops
  // Store first argument; start after first delimiter, stop at next delimiter ','
  for (arrIdx = 2; inStr[arrIdx] != ','; arrIdx++) p1[arrIdx-2] = inStr[arrIdx];
  // Store second argument; start after second delimeter, stop at newline '\n'
  for (int nextIndex = ++arrIdx; inStr[arrIdx] != '\n'; arrIdx++) p2[arrIdx - nextIndex] = inStr[arrIdx];
}

/* Get single voltage from ADC */
void getVoltage(){
  ADMUX = 0b01000000;                     // Select ADC0
  ADCSRA = 0b11000111;                    // Enable ADC, start conversion, prescaler = 128
  while(ADCSRA & (1<<ADSC));              // Wait for conversion to finish
  float vFlt = ((float)ADC * 5.0)/1023.0; // Convert ADC value to voltage
  char vStr[8];
  char sndStr[8] = "V=";
  dtostrf(vFlt, 4, 2, vStr);              // Convert float to string
  strcat(sndStr, vStr);                   // Add label
  strcat(sndStr,"V\n");                   // Add unit and newline
  USART_Transmit((unsigned char*)sndStr); // Send string
}

/* Get multiple voltages from ADC */
void getMultiVoltage(){
  parseCommand();                                 // Parse input command
  int n = strtol(p1, NULL, 10);                   // Convert number of samples to int
  int dt = strtol(p2, NULL, 10);                  // Convert time between samples to int
  for (int i = 0; i < n; i++) {                   // Read and send n samples
    getVoltage();                         
    for (int j = 0; j < dt; j++) _delay_ms(1000); // Wait dt seconds until next sample
  }
}

/* Set the voltage of a specific DAC channel */
void setVoltage(){
  parseCommand();                                       // Parse input command
  unsigned char c = strtol(p1, NULL, 10);               // Convert channel number to int
  unsigned char n = round(strtod(p2, NULL)*255.0/5.0);  // Make voltage machine readable
  i2c_start(0x58);                                      // Start I2C communication to DAC
  i2c_write(c);                                         // Send channel
  i2c_write(n);                                         // Send voltage
  i2c_stop();                                           // Stop I2C communication
}

/* ==================== Main Loop ==================== */
void loop() {
  for (int i = 0; i < 10; i++) {
    inStr[i] = USART_Receive();      // save each character
    if(inStr[i] == '\n') break;     // stop reading if newline char detected
  }
  switch (inStr[0]) {                     // Parse first i/p char
    case 'G': getVoltage(); break;        // If 'G', get immediate ADC voltage
    case 'M': getMultiVoltage(); break;   // If 'M', get multiple ADC voltages
    case 'S': setVoltage(); break;        // If 'S', set DAC voltage
    default: break;                       // Otherwise, do nothing
  }
}
