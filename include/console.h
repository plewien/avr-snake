/*************************************************************************
Title:    Game Console Header File
Inital Author:   David Jahshan
Extended by : Patrick Lewien (694555)
Software: AVR-GCC 
Hardware: ATMEGA16 @ 8Mhz 

DESCRIPTION:
	Macros for Game Console.

*************************************************************************/

#ifndef _GAME_CONSOLE_H_
#define _GAME_CONSOLE_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdlib.h>
#include <assert.h>

typedef unsigned char bool;
#define byte unsigned char

typedef struct {
	uint8_t page;
	uint8_t column;
	uint8_t bit;
} address_t;

typedef enum {UP,DOWN,LEFT,RIGHT,NONE} direction_t;

/*ON OFF*/
#define ON 		0xFF
#define OFF 	0x00
#define HIGH 	0xFF
#define LOW 	0x00
#define IN 		0x00
#define OUT 	0xFF
#define ALL 	0xFF
#define TRUE 	1
#define FALSE 	0
#define FORWARD 0x00
#define BACK 	0xFF

/*Helpful Macros*/
#define SET(PORT,MASK,VALUE) 	PORT = ((MASK & VALUE) | (PORT & ~MASK))
#define GET(PORT,MASK) 			PORT & MASK
#define CEILING(X,Y) 			(((X) + (Y) - 1) / (Y))

// Function declarations
void initialise_game_console();
byte bound_check(byte val, byte min, byte max);
void display_game_over_screen();
void LCD_clear();
void srand_adc(void);
int check_free_ram (void);



/*ATMEGA16 Pins*/
#define ADC_1v5_PIN		_BV(PA3)	

#define FRAM_HOLD_PIN	_BV(PB0)
#define FRAM_WP_PIN		_BV(PB1)
#define FRAM_CS_PIN		_BV(PB2)
#define LCD_PWM_PIN		_BV(PB3)
#define SS_PIN			_BV(PB4)
#define MOSI_PIN		_BV(PB5)
#define MISO_PIN		_BV(PB6)
#define SCK_PIN			_BV(PB7)

#define A_PIN			_BV(PC0)
#define B_PIN			_BV(PC1)
#define BAT_LOW_LED_PIN _BV(PC6)

#define LCD_CS_PIN		_BV(PD0)
#define LCD_RESET_PIN 	_BV(PD1)
#define LCD_A0_PIN		_BV(PD2)
#define LEFT_PIN 		_BV(PD4)
#define RIGHT_PIN		_BV(PD5)
#define UP_PIN			_BV(PD6)
#define DOWN_PIN		_BV(PD7)

/*Bit Vector Multiples*/
#define ALL_ARROW_PIN 	(LEFT_PIN|RIGHT_PIN|UP_PIN|DOWN_PIN)
#define ALL_ACTION_PIN 	(A_PIN|B_PIN)


/*Devices Direction MACROs*/
#define FRAM_HOLD_DIR(DIR)			SET(DDRB,FRAM_HOLD_PIN,DIR)
#define FRAM_WP_DIR(DIR)			SET(DDRB,FRAM_WP_PIN,DIR)
#define FRAM_CHIP_SELECT_DIR(DIR)	SET(DDRB,FRAM_CS_PIN,DIR)
#define LCD_BACKLIGHT_DIR(DIR) 		SET(DDRB,LCD_PWM_PIN,DIR)
#define SS_DIR(DIR)					SET(DDRB,SS_PIN,DIR)
#define MOSI_DIR(DIR)				SET(DDRB,MOSI_PIN,DIR)
#define MISO_DIR(DIR)				SET(DDRB,MISO_PIN,DIR)
#define SCK_DIR(DIR)				SET(DDRB,SCK_PIN,DIR)

#define A_PIN_DIR(DIR)				SET(DDRC,A_PIN,DIR)
#define B_PIN_DIR(DIR)				SET(DDRC,B_PIN,DIR)
#define BAT_LOW_LED_DIR(DIR) 		SET(DDRC,BAT_LOW_LED_PIN,DIR)

#define LCD_CHIP_SELECT_DIR(DIR) 	SET(DDRD,LCD_CS_PIN,DIR)
#define LCD_RESET_DIR(DIR) 			SET(DDRD,LCD_RESET_PIN,DIR)
#define LCD_A0_DIR(DIR)				SET(DDRD,LCD_A0_PIN,DIR)
#define BUTTON_INTERRUPT_DIR(DIR)	SET(DDRD,INT1,DIR)
#define LEFT_PIN_DIR(DIR) 			SET(DDRD,LEFT_PIN,DIR)
#define RIGHT_PIN_DIR(DIR) 			SET(DDRD,RIGHT_PIN,DIR)
#define UP_PIN_DIR(DIR) 			SET(DDRD,UP_PIN,DIR)
#define DOWN_PIN_DIR(DIR) 			SET(DDRD,DOWN_PIN,DIR)

#define ALL_ACTION_PIN_DIR(DIR) 	SET(DDRC,ALL_ACTION_PIN,DIR)
#define ALL_ARROW_PIN_DIR(DIR)	 	SET(DDRD,ALL_ARROW_PIN,DIR)

//Device Internal Pull-ups
#define A_BUTTON_PULL_UP(STATE)				SET(PORTC,A_PIN,STATE)
#define B_BUTTON_PULL_UP(STATE)				SET(PORTC,B_PIN,STATE)
#define LEFT_BUTTON_PULL_UP(STATE)			SET(PORTD,LEFT_PIN,STATE)
#define RIGHT_BUTTON_PULL_UP(STATE)			SET(PORTD,RIGHT_PIN,STATE)
#define UP_BUTTON_PULL_UP(STATE)			SET(PORTD,UP_PIN,STATE)
#define DOWN_BUTTON_PULL_UP(STATE)			SET(PORTD,DOWN_PIN,STATE)

#define ALL_ACTION_BUTTON_PULL_UP(STATE) 	SET(PORTC,ALL_ACTION_PIN,STATE)
#define ALL_ARROW_BUTTON_PULL_UP(STATE) 	SET(PORTD,ALL_ARROW_PIN,STATE)
#define BUTTON_INTERRUPT_PULL_UP(STATE)		SET(PORTD,INT1,STATE)

//Devices Outputs
#define BAT_LOW_LED(STATE) 					SET(PORTC,BAT_LOW_LED_PIN,~STATE)
#define LCD_BACKLIGHT(STATE)				SET(PORTB,LCD_PWM_PIN,STATE)

//Devices Inputs
#define LEFT_BUTTON 						~GET(PIND,LEFT_PIN)
#define RIGHT_BUTTON 						~GET(PIND,RIGHT_PIN)
#define UP_BUTTON 							~GET(PIND,UP_PIN)
#define DOWN_BUTTON 						~GET(PIND,DOWN_PIN)
#define ACTION_A_BUTTON 					~GET(PINC,A_PIN)
#define ACTION_B_BUTTON 					~GET(PINC,B_PIN)
#define INTERRUPT							GET(PIND,INT1)

//Backlight Interface
#define SET_BRIGHTNESS(VAL) 				OCR0 = VAL
#define INCREASE_BRIGHTNESS 				OCR0 = (OCR0 + 64) % 256
#define DEFAULT_BRIGHTNESS					0
#define PWM_GENERATION_MODE(MODE) 			TCCR0 |= MODE
#define FAST_PWM 							(_BV(WGM00)|_BV(WGM01)|_BV(COM01)|_BV(CS01)|_BV(CS00))
#define CTC 								(_BV(WGM01)|_BV(CS00))

//SPI Interface
#define SPI_ENABLE 							(_BV(SPI2X)|_BV(SPE)|_BV(MSTR))
#define SS_SET(STATE)						SET(PORTB,SS_PIN,STATE)
#define SCK_SET(STATE)						SET(PORTB,SCK_PIN,STATE)
#define MOSI_SET(STATE)						SET(PORTB,MOSI_PINSTATE)	
#define SETUP_SPI 							SET(SPCR,SPI_ENABLE,ON)

//LCD Interface
#define LCD_CHIP_SELECT 	SET(PORTD,LCD_CS_PIN,LOW)
#define LCD_CHIP_DESELECT 	SET(PORTD,LCD_CS_PIN,HIGH)
#define LCD_DATA			SET(PORTD,LCD_A0_PIN,HIGH)
#define LCD_COMMAND			SET(PORTD,LCD_A0_PIN,LOW)
#define CMD_PAGE			0xB0
#define CMD_COL_LSB 		0x00
#define CMD_COL_MSB 		0x10

//LCD Dimensions
#define BIT_PER_BYTE		8
#define PIXEL_PER_PAGE		8
#define MAX_PAGE			8
#define MAX_COLUMN			102
#define MAX_ROW				MAX_PAGE*PIXEL_PER_PAGE

//Interrupt Interface
#define ENABLE_INT1							GICR|=_BV(INT1)
#define ENABLE_TIMER_INTERRUPT				TIMSK=_BV(TOIE1)
#define INTERRUPT_TIMER_MODE(MODE)			TCCR1B=MODE
#define TIMER_PRESCALE_1024					(_BV(CS10)|_BV(CS12))
#define INTERRUPT_SENSE_CONTROL(MODE) 		MCUCR=MODE
#define INT1_ANY_CHANGE_IN_LOGIC			(_BV(ISC10))
#define INT1_RISING_EDGE 					(_BV(ISC11)|_BV(ISC10))

//ADC Interface
#define INTERNAL_REF			_BV(REFS1)|_BV(REFS0)
#define ADC_L_ADJUST			_BV(ADLAR)
#define ADC3_MUX				_BV(MUX1)|_BV(MUX0)
#define SETUP_ADMUX(MUX)		ADMUX=INTERNAL_REF|ADC_L_ADJUST|MUX
#define SETUP_ADCSRA_128		ADCSRA=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)
#define START_ADC_CONVERSION	ADCSRA |=_BV(ADSC)
#define WAIT_FOR_CONVERSION		ADCSRA & _BV(ADSC)
#define LOW_POWER				ADCH<0x6E //This is 1.1V if Vref=INTERNAL_REF=2.56V

//FRAM Interface
#define FRAM_CHIP_SELECT 	SET(PORTB,FRAM_CS_PIN,LOW)
#define FRAM_CHIP_DESELECT 	SET(PORTB,FRAM_CS_PIN,HIGH)
#define FRAM_HOLD_SET(VAL)	SET(PORTB,FRAM_HOLD_PIN,~VAL)
#define FRAM_WP_SET(VAL)	SET(PORTB,FRAM_WP_PIN,~VAL)
#define FRAM_WRITE_ENABLE	0x06
#define FRAM_WRITE			0x02
#define	FRAM_READ			0x03

/*** End of Game Console Header File ****/
#endif
