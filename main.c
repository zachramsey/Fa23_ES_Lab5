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

/* ============== Function Declarations ============== */
void getVoltage();
void getMultiVoltage();
void setVoltage();
void parseCommand();
void USART_Init(unsigned int);
unsigned char USART_Receive( void );
void USART_Transmit( unsigned char );

/* ============== Variable Declarations ============== */
int addr = 0x58;        // DAC address
char charIn;            // individual character
char charInArray[10];   // array of characters
char command;           // stores the command from string in
char p1[2];             // char array for first argument
char p2[4];             // char array for 2nd argument


/* ================ Setup & Main Loop ================ */

/* Initialization */
void setup() {
  tw_init(TW_FREQ_400K, true);  // Init I2C; set freq, enable internal pull-up
  
  // Init serial communication
  UBRR0H = (unsigned char)(MYUBRR>>8);  // Set Baud rate
  UBRR0L = (unsigned char)MYUBRR;
  UCSR0B = (1<<RXEN0)|(1<<TXEN0);       // Enable reciever and transmitter
  UCSR0C = (1<<USBS0)|(3<<UCSZ00);      // Set frame format: 8data, 2stop bit
}

/* Main Loop */
void loop() {
  delay(100);
}

/* ============ Interupt Service Routines ============ */

//serial recieve interupt
ISR(USART_RX_vect){
  // Read serial input into charInArray
  for (int i = 0; i < 10; i++) {
    charIn = USART_Receive;       // read string from serial; 1s timeout
    charInArray[i] = charIn;      // save each character
    if(charIn == '\n') break;     // stop reading if newline char detected
  }
  // Handle different commands based on first character of input
  command = charInArray[0];
  if(command == 'G'){ getVoltage(); }
  else if(command == 'M'){ getMultiVoltage(); }
  else if(command == 'S'){ setVoltage(); }
}

/* ========== Serial Communication Routines ========== */

//Serial recieve routine
unsigned char USART_Receive( void ) {
while ( !(UCSR0A & (1<<RXC0)) ) ;     // Wait for data to be received
return UDR0;                          // Get and return received data from buffer
}

//serial transmit routine
void USART_Transmit( unsigned char data ){
  while ( !( UCSR0A & (1<<UDRE0)) );  // Wait for empty transmit buffer
  UDR0 = data;                        // Put data into buffer, sends the data
}

/* ============ Command Handling Routines ============ */

/* Parse input command parameters */
void parseCommand() {
  // Clear previous command arguments for new command
  memset(p1, 0, sizeof(p1));
  memset(p2, 0, sizeof(p2));

  // Store first argument; start after first delimiter, stop at next delimiter ','
  int arrayIndex = 0;
  for (arrayIndex = 2; charInArray[arrayIndex] != ','; arrayIndex++) {  //saves the first argument
    p1[arrayIndex] = charInArray[arrayIndex];
  }

  // Store second argument; start after second delimeter, stop at newline '\n'
  arrayIndex++;                 // increment to skip the delimiter
  int nextIndex = arrayIndex;   // store start index of second argument
  for (arrayIndex; charInArray[arrayIndex] != '\n'; arrayIndex++) {
    p2[arrayIndex - nextIndex] = charInArray[arrayIndex];
  }
}

/* Get single voltage from ADC */
void getVoltage(){          //print the voltage at the ADC
  // print("V=")
  // print((analogRead(0) * (5.0/1023.0)))
}

/* Get multiple voltages from ADC */
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

/* Set the voltage of a specific DAC channel */
void setVoltage(){
  int c = strtol(p1, NULL, 10);
  float v = strtod(p2, NULL);
  uint8_t n = round(v * 256 / 5); // Convert voltage to 8-bit number
  tw_master_transmit(DAC_ADDR, n, sizeof(n), false);
}
