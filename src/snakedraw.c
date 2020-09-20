/*************************************************************************
Title: Snake_Draw
Author: Patrick Lewien (694555)
Software: AVR-GCC 
Hardware: ATMEGA16 @ 8Mhz 

DESCRIPTION:


*************************************************************************/

#include "console.h"
#include "snake.h"

extern obj_t walls[MAX_SNAKE_COLUMN][MAX_SNAKE_ROW];


void update_display_buffer(point_t pt, obj_t object) {
	byte col = pt.x % MAX_SNAKE_COLUMN;
	byte row = pt.y % MAX_SNAKE_ROW;
	walls[col][row] = object;
	return;
}

/*
 * Function:  write_pixel
 * -----------------------
 * Draws/clears a 4x4 pixel at a given position on the screen. A general 
 * function which is extended by methods draw() and clear().
 *
 *  pt: The position on the snake grid.
 *  value: ON or OFF, depending on whether the pixel is to be turned on or off.
 *
 *  returns: True.
 *
 */
byte write_display(point_t pt) {
	byte col = pt.x % MAX_SNAKE_COLUMN;
	byte row = pt.y % MAX_SNAKE_ROW;
	byte i, j, mask, pixel_data[SNAKE_WIDTH];
	obj_t wall_data[2];

	if (row%2 == 1) {
		wall_data[0] = walls[col][row-1];
		wall_data[1] = walls[col][row];
	} else {
		wall_data[0] = walls[col][row];
		wall_data[1] = walls[col][row+1];
	}

	for (i=0; i<2; i++) {
		mask = ((i%2==0) ? 0x0F: 0xF0);
		for (j=0; j<SNAKE_WIDTH; j++) {
			switch (wall_data[i]) {
				case EMPTY:
					SET(pixel_data[j], mask, OFF);
					break;
				case WALL:
					SET(pixel_data[j], mask, ON);
					break;
				default:
					break;
			}
		}
	}


	//Select pixel locations and draw
	byte left_column = (SNAKE_WIDTH*pt.x)%MAX_COLUMN;
	byte page = ((SNAKE_WIDTH*pt.y)/PIXEL_PER_PAGE)%MAX_PAGE;
	select_page(page);
	for (i=0; i < SNAKE_WIDTH; i++) {
		select_column(left_column+i);
		LCD_data_tx(pixel_data[i]);
	}
	return(TRUE);
}

/*
 * Function:  draw
 * ----------------
 * Draws a 4x4 pixel at a given position on the screen.
 *
 *  s_pos: The position on the snake grid.
 *
 */
void draw(point_t pt) {
	update_display_buffer(pt, WALL);
	write_display(pt);
}


/*
 * Function:  clear
 * -----------------
 * Clears a 2x2 pixel from a given position on the screen.
 *
 *  s_pos: The position on the snake grid..
 *
 */
void clear(point_t pt) {
	update_display_buffer(pt, EMPTY);
	write_display(pt);
}


/*
 * Function:  draw_food
 * ----------------
 * Draws a piece of food on the screen.
 *  pt: The location of the food.
 *
 */
void draw_food(point_t pt) {
	// TODO: Implement in order to differentiate the food from the snake
	// Trickier than it looks because if the snake passes above/below the food,
	// then it will be erased (since it currently stored as a wall).
	return;
}


/*
 * Function:  get2bits
 * --------------------
 * Returns two bits from a byte of data, with location specified by pixel. This
 * is used to select two pixels from the wall data to be updated on the screen.
 *	
 *	Example:
 *		get2bits(0x0C, 2) = get2bits(0x0C, 3) = 0b11.
 * 
 *
 *	data: The byte of data to be selected from. 
 * 	pixel: The location in the byte of data, which is rounded down to even with 'shift'.
 *  returns: The two bits selected from data.
 *
 */
byte get2bits(byte data, byte pixel) {
	byte shift = pixel - (pixel % 2);
	return (data >> shift) & 0x03;
}


/*
 * Function:  interleave
 * ----------------------
 * Bit duplication of an 4-bit value into a 8-bit byte. Used to 
 * replicate 4-bits of a wall_buffer to be used as a page when
 * writing to the LCD.
 * 
 * Demonstration:
 * 		+---+---------+---------+
 *		| 0 | _ _ _ _ | A B C D |
 *		+---+---------+---------+
 *		| 1 | _ _ A B | _ _ C D |
 *		+---+---------+---------+
 *		| 2 | _ A _ B | _ C _ D |
 *		+---+---------+---------+
 *		| 3 | A A B B | C C D D |
 *		+---+---------+---------+
 *
 * See https://stackoverflow.com/questions/55051490/bit-duplication-from-8-bit-to-32-bit
 * for more information on bit duplication.
 * 
 *  value: 4-bit value to be duplicated.
 *
 */
byte interleave(byte value) {
	byte x = value;
    x = (x | (x << 2)) & 0x33;
    x = (x | (x << 1)) & 0x55;
    x |= (x << 1);
    return x;
}


/*
 * Function:  interleave2
 * -----------------------
 * Bit duplication of an 2-bit value into a 8-bit byte. With two 
 * bits, it's easier to run on a case-by-case method rather than 
 * use bit operations. There are only FOUR cases, after all.
 *
 *  value: 4-bit value to be duplicated.
 *	returns: Duplicated byte, e.g. 0b10 -> 0b11110000.
 *
 */
byte interleave2(byte value) {
	switch (value) {
		case 0b00: return 0x00;
		case 0b01: return 0x0F;
		case 0b10: return 0xF0;
		case 0b11: return 0xFF;
		default: return 0x00;
	}
}

/*
 * Function:  redraw_all_walls
 * ----------------------------
 * Debugging function for checking that all walls have been drawn. Displays a 
 * 'mini-map' in the top-left corner of the screen.
 *
 */
void redraw_all_walls(void) {
	uint8_t page;
	uint8_t column;
	for (page = 0; page<MAX_SNAKE_PAGE; page++) {
		select_page(page);
		for (column = 0; column<MAX_SNAKE_COLUMN; column++) {
			select_column(column);
			LCD_data_tx(walls[column][page]);
		}
	}
	return;
}


void draw_minimap(void) {
	uint8_t elem, row, page, column;
	byte pixel_data = 0x00;
	for (column=0; column<MAX_SNAKE_COLUMN; column++) {
		for (row=0; row<MAX_SNAKE_ROW; row++) {
			elem = row%PIXEL_PER_PAGE;
			if (walls[column][row] != EMPTY) {
				SET(pixel_data, _BV(elem), ON);
			} else {
				SET(pixel_data, _BV(elem), OFF);
			}
			if (elem == 7) {
				page = row/PIXEL_PER_PAGE;
				select_page(page);
				select_column(column);
				LCD_data_tx(pixel_data);
				pixel_data = 0x00;
			}
		}
	}
	return;
}

