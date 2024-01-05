.include "m328pdef.inc"
.include "delay_Macro.inc"
.include "1602_LCD_Macros.inc"
.include "UART_Macros.inc"

.cseg
.org 0x0000

SBI DDRB, PB3         ; Set PB3 pin for Output to LED
CBI PORTB, PB3        ; LED OFF
SBI DDRB, PB4         ; Set PB4 pin for Output to LED
CBI PORTB, PB4        ; LED OFF

.def A  = r16          ; just rename or attach a label to the register
.def AH = r17

; ADC config
LDI   A,0b11000111     ; [ADEN ADSC ADATE ADIF ADIE ADIE ADPS2 ADPS1 ADPS0]
STS   ADCSRA,A
LDI   A,0b01100000     ; [REFS1 REFS0 ADLAR – MUX3 MUX2 MUX1 MUX0]
STS   ADMUX,A          ; Select ADC0 (PC0) pin
SBI   PORTC,PC0        ; Enable Pull-up Resistor

Serial_begin

LCD_init               ; Initialize the 16x2 LCD
LCD_backlight_OFF      ; Turn Off the LCD Backlight
delay 1000
LCD_backlight_ON       ; Turn On the LCD Backlight

loop:
    delay 1000
    LDS   A,ADCSRA       ; Start Analog to Digital Conversion
    ORI   A,(1<<ADSC)
    STS   ADCSRA,A

wait:
    LDS   A,ADCSRA       ; Wait for ADC conversion to complete
    sbrc  A,ADSC
    rjmp  wait

    LDS   A,ADCL         ; Must Read ADCL before ADCH
    LDS   AH,ADCH
    delay 150            ; Delay 100ms

    LCD_send_a_command 0x01
    delay 10

    LCD_send_a_command 0x80
    delay 10

    ; Convert the value to ASCII and display on LCD
    mov   r24, AH
    call  int_to_ascii
    call  lcd_write_char

    ; Control LED based on threshold
    cpi   AH, 200        ; Compare LDR reading with the desired threshold value (e.g., 200)
    brlo  LED_OFF        ; Jump if AH < 200
    SBI   PORTB, PB4     ; LED ON
    CBI   PORTB, PB3     ; LED OFF
    rjmp  LED_ON_End     ; Jump to skip LED_OFF condition

LED_OFF:
    CBI   PORTB, PB4     ; LED OFF
    SBI   PORTB, PB3     ; LED ON

LED_ON_End:
    rjmp  loop

; Convert integer to ASCII and display on LCD
int_to_ascii:
    ldi   r25, 10         ; Base 10
    call  udivmod
    addi  r24, '0'        ; Convert to ASCII
    ret

; Unsigned division and modulo
udivmod:
    ldi   r23, 0          ; Clear remainder
    ldi   r22, 0          ; Initialize quotient
udivmod_loop:
    cp    r23, r25        ; Compare remainder with divisor
    brlo  udivmod_skip_subtract
    sub   r23, r25        ; Subtract divisor from remainder
    inc   r22             ; Increment quotient
udivmod_skip_subtract:
    lsl   r23             ; Shift remainder left
    rol   r24             ; Rotate quotient left through carry
    lsl   r24             ; Shift quotient left
    dec   r26             ; Decrement counter
    brne  udivmod_loop    ; Continue loop if counter is not zero
    ret
