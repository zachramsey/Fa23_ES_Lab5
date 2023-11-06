# Fall 2023 Embedded Systems Lab 5

---

Mid-lab review: November 8  
*Program can successfully demonstrate RS232-based communication and reading of ADC voltage values.*  
*Develop a test approach s.t. correctness of read ADC & generated DAC voltages can be easily demonstrated.*  
Deadline: November 15

---

## Objective
Gain some experience with C-based programming of AVR uCs, serial interface protocols (I2C, RS232, ...), ADC, and DAC.

## Description
Build a remotely controllable analog signal/data logging system using:
- The built-in A/D converter of the ATmega328P controller
- The MAX518, an external two-channel D/A converter with an I2C interface.
The system will have an RS232 interface (9600 8N1) that will be connected to your laptop/computer.  
The computer user will be able to trigger analog voltage measurements and set the output voltage for both channels of the DAC via commands sent through the RS232 interface.

---

## The system will implement the following commands:
| Command | Function                                   | Arguments |
|:------- |:------------------------------------------ |:--------- |
| G       | Get single voltage measurement from ADC    | no arguments <br> **The ATmega328P ADC must be used in 10-bit mode!** |
| M,n,dt  | Get multiple voltage measurements from ADC | n: # of measurements ~ *n ∈ [2, 20]* <br> dt: time between measurements ~ *n ∈ [1, 10] s*
| S,c,v   | Set DAC output voltage                     | c: DAC channel # ~ c ∈ {0,1} <br> v: output voltage ~ *float, format: "n.nn" V* <br> **Value sent to DAC must first be converted to** <br> **decimal s.t. quantization error is minimized.** |

---

Nominal message exchange between terminal user and analog interface system, which will send information back to the terminal screen.  
_Use the Arduino IDE Serial Monitor functionality as a terminal emulator._  

**BOLD** text is typed by the user on the keyboard. _ITALICIZED_ text represents responses by the analog interface system.

```
**G**
_v=0.244 V_

**M,5,10**
_t=0 s, v=4.985 V_
_t=10 s, v=0.156 V_
_t=20 s, v=1.253 V_
_t=30 s, v=2.510 V_
_t=40 s, v=1.250 V_

**S,1,3.45**
_DAC channel 1 set to 3.46 V (177d)_
```

---

**Notes:**
Available libraries for I2C and RS232 communication can be used.  
Used libraries must be mentioned in post-lab report.  
Some libraries need to be configured for the specific AVR microcontroller used.
