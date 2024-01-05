.include "m328pdef.inc"
.include "delay_Macro.inc"
.include "1602_LCD_Macros.inc"
.include "Wire_Macro.inc"

; Constants
BH1750address   = 0x23

.cseg
.org 0x0000

SBI DDRB, PB3         ; Set PB3 pin for Output to LED
CBI PORTB,	PB3        ; LED OFF
SBI DDRB, PB4         ; Set PB4 pin for Output to LED
CBI PORTB, PB4        ; LED OFF

.def A = r16
.def AH = r17

; LCD initialization
LCD_init
delay 1000
LCD_backlight_OFF    ; Turn Off the LCD Backlight
delay 1000
LCD_backlight_ON     ; Turn On the LCD Backlight
delay 2000

loop:
  ; Initialize BH1750 sensor
  CALL BH1750_Init

  ; Read data from BH1750 sensor
  CALL BH1750_Read
  LDS A, buff+1
  LDS AH, buff

  ; Convert the received data
  LSL A               ; Left shift to multiply by 2
  ADC A, AH           ; Add the high byte to the result
  LDI AH, 12          ; Divide by 1.2 (AH = 10, AL = 2)
  CALL div

  ; Display intensity value on LCD
  CALL LCD_clear
  CALL LCD_printString, "Intensity in LUX"
  CALL LCD_setCursor, 7, 2
  CALL LCD_printNumber, A

  ; Control LEDs based on intensity threshold
  CPI A, 200          ; compare intensity value with the threshold (e.g., 200)
  BRLO LED_OFF        ; jump if A < 200

  ; LEDs ON
  CBI PORTB, PB3      ; LED OFF
  SBI PORTB, PB4      ; LED ON
  RJMP LED_ON_End     ; Jump to skip LED_OFF condition

LED_OFF:
  SBI PORTB, PB3      ; LED ON
  CBI PORTB, PB4      ; LED OFF

LED_ON_End:
  RJMP loop

; Function to read data from BH1750 sensor
BH1750_Read:
  LDI A, BH1750address ; Load BH1750 address
  CALL Wire_beginTransmission
  CALL Wire_requestFrom, 2

  LDS A, buff           ; Read the first byte
  LDS AH, buff+1        ; Read the second byte

  CALL Wire_endTransmission
  RET

; Function to initialize BH1750 sensor
BH1750_Init:
  LDI A, BH1750address  ; Load BH1750 address
  CALL Wire_beginTransmission
  CALL Wire_write, 0x10  ; Write command for sensor initialization
  CALL Wire_endTransmission
  RET
