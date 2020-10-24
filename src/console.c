/*************************************************************************
Title: Game Console
Initial Author: David Jahshan
Extended by: Patrick Lewien (694555)
Software: AVR-GCC 
Hardware: ATMEGA16 @ 8Mhz 

DESCRIPTION:
	Game Console is a PCB made in ELEN90064 Embedded System Design at 
	The University of Melbourne. The code in this file includes everything
	needed to power on the device and adjust the screen. Game files can
	be loaded in as separate programs.
	
	Arrow keys: Move direction
	A-button: Select option
	B-button: Back/return


*************************************************************************/


#include "console.h"
#include "dogm-graphic.h"


/*********************************
 **		GLOBAL VARIABLES		**
 *********************************/
volatile direction_t selected_direction = RIGHT;
volatile byte direction_pressed_flag = FALSE;
volatile byte action_a_flag = FALSE;
//



/*********************************
 **	INTERRUPT SERVICE ROUTINES  **
 *********************************/
ISR(INT1_vect) { //Button NAND ISR
	if (UP_BUTTON) {
		selected_direction = UP;
		direction_pressed_flag = TRUE;
	}
	if (DOWN_BUTTON) {
		selected_direction = DOWN;
		direction_pressed_flag = TRUE;
	}
	if (LEFT_BUTTON) {
		selected_direction = LEFT;
		direction_pressed_flag = TRUE;
	}
	if (RIGHT_BUTTON) {
		selected_direction = RIGHT;
		direction_pressed_flag = TRUE;
	}
	if (ACTION_A_BUTTON) { //Reset screen: debug only
		//action_a_flag = TRUE;
	}
	if (ACTION_B_BUTTON) { //Up the brightness
		INCREASE_BRIGHTNESS;
	}
}

ISR(TIMER1_OVF_vect) { //Timer ISR for low battery LED
	START_ADC_CONVERSION;
	while(WAIT_FOR_CONVERSION);
	if (LOW_POWER)
		BAT_LOW_LED(ON);
	else
		BAT_LOW_LED(OFF);
}


/*********************************
 **		CONSOLE FUNCTIONS		**
 *********************************/

/*
 * Function:  main
 * ----------------
 * Main function to be run on power-up.
 *
 */
int main(void) {
	initialise_game_console();
	
	//TODO: Initalise game menu screen
	while(TRUE) {
		play_snake_game();
		display_game_over_screen();
	}
	
	return 0;
}


/*
 * Function:  initialise_game_console
 * -----------------------------------
 * Console initialisation function to be run at power-up.
 *
 */
void initialise_game_console(void) {	
	//Set up low power LED
	BAT_LOW_LED(OFF); //Make sure it is off before changing direction
	BAT_LOW_LED_DIR(OUT); //Set BATTERY LED I/Os as outputs
	BAT_LOW_LED(ON); //Hold on for loading sequence
	SETUP_ADMUX(ADC3_MUX);
	SETUP_ADCSRA_128; //Clock prescaler of 128
	
	//Set all button I/Os as input
	ALL_ARROW_PIN_DIR(IN); 
	ALL_ACTION_PIN_DIR(IN);

	//Switch on internal pull-ups for all input buttons
	ALL_ARROW_BUTTON_PULL_UP(ON);
	ALL_ACTION_BUTTON_PULL_UP(ON);
	
	//Set up interrupts
	ENABLE_INT1;
	INTERRUPT_SENSE_CONTROL(INT1_RISING_EDGE);
	ENABLE_TIMER_INTERRUPT;
	INTERRUPT_TIMER_MODE(TIMER_PRESCALE_1024);
	sei(); //Enable global interrupts

	//Set up SPI with LCD display
	lcd_init();
	lcd_set_font(FONT_FIXED_8, NORMAL);
	
	//Set up LCD PWM
	LCD_BACKLIGHT(OFF);
	LCD_BACKLIGHT_DIR(OUT);
	PWM_GENERATION_MODE(FAST_PWM);
	SET_BRIGHTNESS(DEFAULT_BRIGHTNESS);
	
	//Seed random generator
	srand_adc();

	//Loading sequence finished
	BAT_LOW_LED(OFF); 
	return;
}


/*
 * Function:  init_spi_lcd
 * ------------------------
 * Load data into SPI. Remember to chip-select first!
 *
 */
void init_spi_lcd() {
	DDRB = SS_PIN | MISO_PIN | MOSI_PIN | SCK_PIN;
	SET(PORTB, SS_PIN, HIGH);
	SPCR = (0<<SPIE) | (1<<SPE) | (0<<DORD) | (1<<MSTR) | (1<<CPOL) | (1<<CPHA) | (1<<SPR0);
	//SPSR = 1<<SPI2X;
	//SPDR = LCD_NO_OP; //Do not use 0 here, only LCD_NOP is allowed!
}


/*
 * Function:  bound_check
 * -----------------------
 * Determines if a value is within a given range, specified by min and
 * max. Used to wrap the screen if a value is off the edge.
 *	
 *	val: A given number, as a byte.
 *	min: The minimum number for that value.
 *	max: The maximum number for that value.
 *
 *	returns: The modulus of val that places it within range.
 */
byte bound_check(byte val, byte min, byte max) {
	if (val == 0xFF || val < min) //First check is for if min=0
		return max - 1;
	return val % max; 
}

/*
 * Function:  LCD_clear
 * ---------------------
 * Sets all bits on screen to be OFF. Must also clear the screen buffer in
 * tandem, or else weird stuff will happen when trying to rewrite the pixels
 * on the screen (since the buffer will still store the current pixels.
 *	
 */
void LCD_clear(void) {
	//clear display content
	lcd_clear_area_xy(LCD_RAM_PAGES,LCD_WIDTH,NORMAL,0,0); 
	return;
}


/*
 * Function:  srand_adc
 * ---------------------
 * The pseudo-random number generator is usually seeded with time(NULL). However,
 * in an AVR program, a reference time isn't kept. Instead, a random seed can be 
 * generated using the high-resolution bits of an analog-digital converter (ADC).
 *
 */
void srand_adc(void) {
	START_ADC_CONVERSION;
	while(WAIT_FOR_CONVERSION);
	srand(ADCL);  // seed with low-byte of ADC
	return;
}

void display_game_over_screen(void) {
	
	lcd_moveto_xy(2,20);
	lcd_putstr("GAME OVER");
	lcd_moveto_xy(4,13);
	lcd_putstr("play again?");
	action_a_flag = FALSE;

	while(action_a_flag == FALSE) {
		_delay_ms(100);
	}
	
	LCD_clear();
	return;
}
