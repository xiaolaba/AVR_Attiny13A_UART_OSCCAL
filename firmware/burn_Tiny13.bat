@echo off
:: see https://www.nongnu.org/avrdude/user-manual/avrdude_4.html
:: Hide REM lines from showing in a batch file, https://www.computerhope.com/issues/ch001313.htm
prompt xiao$$$g 

set ac=C:\WinAVR-20100110
path %ac%\bin;%ac%\utils\bin;%path%

set MCU=attiny13
set #MCU=attiny85
set FUSE_L=0x3A
set FUSE_H=0xFF
set F_CPU=9600000
set CC=avr-gcc
set LD=avr-ld
set OBJCOPY=avr-objcopy
set OBJDUMP=avr-objdump
set SIZE=avr-size
set AVRDUDE=avrdude
set CFLAGS=-std=c99 -Wall -g -Os -mmcu=%MCU% -DF_CPU=%F_CPU% -I. -DUART_BAUDRATE=115200
set PROGRAMMER=usbtiny
set CALIBRATION=read_calibration_2bytes_hex.txt
set SRCS=main.c uart.c


::set TARGET=Tiny13_UART_OSCCAL.ino_attiny13a_9600000L.hex
set TARGET=%1


@echo on

:: remove files
::rm -f *.s *.o *.elf *.hex *.asm

:: compile
::%CC% %CFLAGS% -o %TARGET%.o %SRCS% -Wa,-adhlns=uart.asm
@rem %CC% %CFLAGS% -o %TARGET%.o %SRCS%

:: link
::%LD% -o %TARGET%.elf %TARGET%.o
:: build firmware hex image, this line will not display when batch file executed
::%OBJCOPY% -j .text -j .data -O ihex %TARGET%.o %TARGET%.hex
:: check firmware size
::%SIZE% -C --mcu=%MCU% %TARGET%.elf
:: build assembler listing
::%OBJDUMP% -S %TARGET%.elf > %TARGET%.asm



:: TWO CALIBRATION BYTE in HEX
@REM %AVRDUDE% -p %MCU% -c %PROGRAMMER% -v -U flash:w:%TARGET%.hex:i -F -P usb -U calibration:r:%CALIBRATION%:h -U lfuse:w:%FUSE_L%:m -U hfuse:w:%FUSE_H%:m 

:: TWO CALIBRATION BYTE in DEC
%AVRDUDE% -p %MCU% -c %PROGRAMMER% -v -b 115200 -U flash:w:%TARGET%:i -P usb -U calibration:r:%CALIBRATION%:h -U lfuse:w:%FUSE_L%:m -U hfuse:w:%FUSE_H%:m

@REM avrdude -c usbtiny -p t13 -v -b 19200 -U flash:w:main.hex -U lfuse:w:0x6a:m -U hfuse:w:0xff:m
pause

