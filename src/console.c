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


#include "game_console.h"
#include "snake.h"



/*********************************
 **		GLOBAL VARIABLES		**
 *********************************/
volatile direction_t direction = RIGHT;
//volatile byte button_pressed_flag = FALSE;



/*********************************
 **	INTERRUPT SERVICE ROUTINES  **
 *********************************/
ISR(INT1_vect) { //Button NAND ISR
	if (UP_BUTTON && direction!=DOWN) {
		direction = UP;
		//button_pressed_flag = TRUE;
	}
	if (DOWN_BUTTON && direction!=UP) {
		direction = DOWN;
		//button_pressed_flag = TRUE;
	}
	if (LEFT_BUTTON && direction!=RIGHT) {
		direction = LEFT;
		//button_pressed_flag = TRUE;
	}
	if (RIGHT_BUTTON && direction!=LEFT) {
		direction = RIGHT;
		//button_pressed_flag = TRUE;
	}
	if (ACTION_A_BUTTON) { //Reset screen: debug only
		LCD_clear();
		clear_walls();
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

	play_snake_game();

	return 0;
}

/*
 * Function:  free_RAM
 * --------------------
 * Checks how much RAM is avalible. Doesn't actually free RAM.
 *
 */
int free_RAM(void) {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
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
	
	//Set up LCD PWM
	LCD_BACKLIGHT(OFF);
	LCD_BACKLIGHT_DIR(OUT);
	PWM_GENERATION_MODE(FAST_PWM);
	SET_BRIGHTNESS(DEFAULT_BRIGHTNESS);

	//Enable SPI, Set as Master
	SS_DIR(OUT); //Ensures ATMEGA16 is master
	SS_SET(HIGH); //Don't want to upset the master
	MOSI_DIR(OUT);
	MISO_DIR(IN);
	SCK_DIR(OUT);
    SET(SPCR,SPI_ENABLE,ON); //Setup SPI
	
	//Set up LCD display
	LCD_CHIP_SELECT_DIR(OUT);
	LCD_CHIP_DESELECT; //Don't select LCD before ready
	LCD_A0_DIR(OUT); //For command/data control
	LCD_RESET_DIR(OUT);
	LCD_RESET(LOW); //Reset before build
	LCD_RESET(HIGH);
	LCD_initialise();
	LCD_clear();

	BAT_LOW_LED(OFF); //Loading sequence finished
	return;
}


/*
 * Function:  SPI_tx
 * ------------------
 * Load data into SPI. Remember to chip-select first!
 *	
 *	returns: The transmitted byte.
 *
 */
byte SPI_tx(byte tx_byte) {
	SPDR = tx_byte;	//Load data into buffer
    while(!(SPSR & (1<<SPIF))); //Wait until transmission complete
	return tx_byte;
}


/*
 * Function:  LCD_command_tx
 * --------------------------
 * Sends a command byte to the LCD. Chip select is performed inside this function,
 * so no need to do so externally.
 *
 */
void LCD_command_tx(byte tx_byte) {
	LCD_CHIP_SELECT;
	LCD_COMMAND;
	SPI_tx(tx_byte);
	LCD_CHIP_DESELECT;
	return;
}


/*
 * Function:  LCD_data_tx
 * -----------------------
 * Sends a data byte to LCD. Chip select is performed inside this function,
 * so no need to do so externally.
 *
 */
void LCD_data_tx(byte tx_byte) {
	LCD_CHIP_SELECT;
	LCD_DATA;
	SPI_tx(tx_byte);
	LCD_CHIP_DESELECT;
	return;
}


/*
 * Function:  LCD_initialise
 * --------------------------
 * Initialise LCD, to be run at power-up.
 *
 */
void LCD_initialise(void) {
	LCD_command_tx(0x40);//Display start line 0
	LCD_command_tx(0xA1);//SEG reverse
	LCD_command_tx(0xC0);//Normal COM0~COM63
	LCD_command_tx(0xA4);//Disable -> Set All Pixel to ON
	LCD_command_tx(0xA6);//Display inverse off
	_delay_ms(120);
	LCD_command_tx(0xA2);//Set LCD Bias Ratio A2/A3
	LCD_command_tx(0x2F);//Set Power Control 28...2F
	LCD_command_tx(0x27);//Set VLCD Resistor Ratio 20...27
	LCD_command_tx(0x81);//Set Electronic Volume
	LCD_command_tx(0x10);//Set Electronic Volume 00...3F
	LCD_command_tx(0xFA);//Set Adv. Program Control
	LCD_command_tx(0x90);//Set Adv. Program Control x00100yz yz column wrap x Temp Comp
	LCD_command_tx(0xAF);//Display on
	return;
}


/*
 * Function:  select_page
 * -----------------------
 * Choose a page to write to on the LCD screen.
 *
 *	page: The page to be selected, from 0 to MAX_PAGE.
 */
void select_page(byte page) {
	byte page_cmd_address;
	page_cmd_address = (CMD_PAGE | (page%MAX_PAGE)); //defensive programming
	LCD_command_tx(page_cmd_address);
	return;
}


/*
 * Function:  select_column
 * -------------------------
 * Choose a column to write to on the LCD screen.
 *
 *	column: The column to be selected, from 0 to MAX_COLUMN. If a column is 
 * 			selected outside this range, then its modulus is taken instead.
 */
void select_column(byte column) {
	byte page_cmd_address_MSB;
	byte page_cmd_address_LSB;
	column %= MAX_COLUMN; //defensive programming
	page_cmd_address_LSB = (CMD_COL_LSB | (column&0x0F));
	page_cmd_address_MSB = (CMD_COL_MSB | ((column>>4)&0x0F));
	LCD_command_tx(page_cmd_address_LSB);
	LCD_command_tx(page_cmd_address_MSB);
	return;
}


/*
 * Function:  draw_pixel
 * ----------------------
 * Draws a byte of pixels on the screen, in a given page and column.
 *	
 *	page: 		The page to be selected, from 0 to MAX_PAGE.
 *	column: 	The column to be selected, from 0 to MAX_COLUMN. 
 *	pixel_data: Data for eight pixels, with 1=ON and 0=OFF.
 */
void draw_pixel(byte page, byte column, byte pixel_data) {
	select_page(page);
	select_column(column);
	LCD_data_tx(pixel_data);
	return;
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
	uint8_t page;
	uint8_t column;
	for (page = 0; page<MAX_PAGE; page++) {
		select_page(page);
		for (column = 0; column<MAX_COLUMN; column++) {
			select_column(column);
			LCD_data_tx(OFF);
		}
	}
	return;
}