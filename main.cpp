#include <Arduino.h>


//function declarations:
void getVoltage();

void getMultiVoltage();

void setDACVoltage();

void setup() {
  Serial.begin(9600); // Initialize serial communication at 9600 baud rate
}

//Serial Interupt
ISR(USART_RX_vect){ //serial recieve interupt
//check characters and or strings and perform necissary actions
Serial.readString(); //read string from serial, note, timeout is 1sec

}

void loop() {



  for (int i = 0; i <= 5; ++i) {
    int sensorValue = analogRead(i); // Read voltage from analog pin i
    float voltage = sensorValue * (5.0 / 1023.0); // Convert the analog value to voltage (assuming 5V reference voltage)
    Serial.print("Analog Pin A");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(sensorValue);
    Serial.print(" (");
    Serial.print(voltage, 2); // Print voltage with 2 decimal places
    Serial.println("V)");
    delay(1000); // Wait for a second before reading the next analog pin
  }
}

void getVoltage(){

}

void getMultiVoltage(){

}

void setDACVoltage(){

}
