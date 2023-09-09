
// change port pin of uart pin @ uart_pin.h
// BAUD_RATE 115200 @ BasicSerial3.h
// xiaolaba


#define debug true  // enable ADC value dump to UART, see drum sound and the effect
#define dumpOSCCAL true // enable dump OSCCAL (0x0-0x7f), and find proper baud rate 115200

// compiler : Arduino IDE 1.8.5, AVR-GCC 4.9.2

//2023-09-09
// compiler : Arduino IDE 1.8.19

// Attiny25/45/85, 45 tested
// fuses L = 0xE2, H = 0xDF, E = 0xFF

// ATtiny13, tested
// fuses L = 0x3A, H = 0xFB

// 2019-FEB-26, xiaolaba, copy and include support ATtiny13 (TIMSK0), pin assignment same as ATtiny45 but TIMSK is different register name
// REF : https://xiaolaba.wordpress.com/2018/03/07/attiny13-high-voltage-serial-programmer-for-chip-re-enable/
// TTINY13 0x1E9007 // factory reset, L: 0x6A, H: 0xFF, 8 pin

#include "uart_pin.h"   // UART_Tx & UART_Rx pin defined over there
#include "BasicSerial3.h"   


#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif



#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>

#include <util/delay.h>

// Workaround for PROGMEM issues under g++: http://gcc.gnu.org/bugzilla/show_bug.cgi?id=34734 
#ifdef PROGMEM 
#undef PROGMEM 
#define PROGMEM __attribute__((section(".progmem.data"))) 
#endif


// Impact pulse table, quasi logarithmic dropoff for modulation
const uint8_t impact_tbl[8] PROGMEM = {
// dummy
//const uint8_t sine_tbl[8] PROGMEM = {
  9, 8, 8, 8, 8, 8, 8, 8
};

const uint8_t sine_tbl[256] PROGMEM = {
// dummy  
  9, 8, 8, 8, 8, 8, 8, 8
};  




unsigned char hex_h, hex_l; // for hex2asc()

void hex2asc(unsigned char byte_value){
      hex_h = (byte_value >> 4)   + 0x30;  // high nibble
      hex_l = (byte_value & 0x0f) + 0x30;  // low nibble
      if (hex_h > '9') {hex_h += 7;}    // byte of ASCII remap to "A ~ F"
      if (hex_l > '9') {hex_l += 7;}    // byte of ASCII remap to "A ~ F"
}

void printhex(unsigned char byte_value){
      hex2asc(byte_value);
      TxByte('0'); TxByte('x'); TxByte(hex_h); TxByte(hex_l); TxByte(' ');
}


void say_ok (byte count)
{
    sbi (DDRB, UART_Tx); //UART Tx port pin as output, used for UART Tx after wake up
    PORTB |= (1<<UART_Tx); // calling code must set Tx line to idle state (high) or 1st byte may be lost
    
//    serOut("OK-"); // 8 bytes more ROM code
//    TxByte('O'); //TxByte('K'); //TxByte('-');
}


// String send to Serial port, for ATtiny13 software UART
void serOut(const char* str)
//void serOut(char* str)
{
   while (*str) TxByte (*str++);
}


void say_hello ()
{
    serOut("\r\nAttiny13 hello xiaolaba\r\n"); //string too long, stack full run out
//    serOut("\r\nATtiny45, AttinyBassDrum, 2019-FEB-28 xiaolaba\r\nReady!"); //string too long, stack full run out
//    serOut("IR\r\n"); //4 bytes, same code size as following, less than 4 bytes, TxByte is less code size
/*
    TxByte('O');
    TxByte('K');
    TxByte('\r');
    TxByte('\n');
*/
}



//--------------------------------------------------------------------------------
// Analogue port read
//--------------------------------------------------------------------------------
void analogRead()
{
  ADCSRA |= _BV(ADEN);            // Analog-Digital enable bit
  ADCSRA |= _BV(ADSC);            // Discarte first conversion
  while (ADCSRA & _BV(ADSC)) {};  // Wait until conversion is done
  ADCSRA |= _BV(ADSC);            // Start single conversion
  while (ADCSRA & _BV(ADSC)) {};  // Wait until conversion is done
  ADCSRA &= ~_BV(ADEN);           // Shut down ADC
}




//--------------------------------------------------------------------------------
// Main loop
//--------------------------------------------------------------------------------
int main(void)
{
  // PWM output on PORTB0 = pin 5.
  //DDRB = _BV(0);

  // PWM output on PORTB0 = pin 5.
  // SERIAL output on PORTB1 = pin 6
  DDRB = _BV(0) | _BV(1);

  sbi (DDRB, UART_Tx); //UART Tx port pin as output, used for UART Tx after wake up 
  PORTB |= (1<<UART_Tx); // calling code must set Tx line to idle state (high) or 1st byte may be lost

  _delay_ms(1000);
#ifdef dumpOSCCAL
    for (OSCCAL = 0; OSCCAL < 0x7f; OSCCAL++) {
      printhex(OSCCAL);
      serOut("OSCCAL fine tune\r\n");
    }
#endif

  //OSCCAL = 0x5c;  // ATtiny13  internal RC 9.6MHZ
  OSCCAL = 0x60;  // ATtiny13  internal RC 9.6MHZ

  _delay_ms(1000);  
  say_hello();



/*    
  // PWM init, 8Mhz / 256 gives 31.25kHz
  TCCR0A =
    _BV(COM0A1) |             // Clear OC0A/OC0B on Compare Match.
    _BV(WGM00) |_BV(WGM01);   // Fast PWM, top 0xff, update OCR at bottom.
  TCCR0B = _BV(CS00);         // No prescaling, full 8MHz operation.

*/

/* add support ATtiny13 
 * xiaolaba 2019-FEB-26
 */

 /*
#if defined (__AVR_ATtiny25__) | (__AVR_ATtiny45__) | (__AVR_ATtiny85__)   
  TIMSK  = _BV(OCIE0A);       // Timer/Counter0 Output Compare Match A Interrupt Enable
//  __asm nop __endasm;         // test MCU model and conditional compiling, see disassemble, one nop
#endif
  
#if defined (__AVR_ATtiny13__)  
  TIMSK0 = _BV(OCIE0A);       // Timer/Counter0 Output Compare Match A Interrupt Enable
//  __asm nop __endasm;         // test MCU model and conditional compiling see disassemble, one nop
//  __asm nop __endasm;         // test MCU model and conditional compiling see disassemble, one nop
#endif


  // Analogue init.
  ADCSRA |=
    _BV(ADEN) |               //  ADC Enable
    _BV(ADPS1) | _BV(ADPS0);  // Div 8 prescaler

  // Enable interrupts.
  //sei();
*/

  // Main loop.

  //for (uint8_t i=0; i<10; i++)
  for (;;)
  {
      say_hello();
      delay(1000);
  }

//dummy
  for (;;)
  {
    //------------------------------------------------
    // Pot 1 determines type of drum and param 1.
    // ADC2 (PINB 4, chip pin 3).
    ADMUX = _BV(ADLAR) | _BV(MUX1);
    analogRead();

#ifdef debug
    printhex(ADCH);
    serOut(" speed ADCH\r\n"); 
#endif

//    TxByte('A');
//    TxByte('\r');
//    TxByte('\n');
//    say_hello();    


  }
}
