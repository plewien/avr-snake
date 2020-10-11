/*************************************************************************
Title: Snake_Draw
Author: Patrick Lewien (694555)
Software: AVR-GCC 
Hardware: ATMEGA16 @ 8Mhz 

DESCRIPTION:


*************************************************************************/

#include "console.h"
#include "snake.h"

extern byte walls[MAX_SNAKE_COLUMN][MAX_SNAKE_PAGE];

void update_display_buffer(point_t pt, obj_t object) {
	byte col = pt.x % MAX_SNAKE_COLUMN;
	byte page = (pt.y / SNAKE_ROWS_PER_PAGE) % MAX_SNAKE_PAGE;
	byte bit_start = SNAKE_ROW_BIT_SIZE*(pt.y % SNAKE_ROWS_PER_PAGE);
	byte mask = _BV(bit_start) | _BV(bit_start+1);
	byte msg = (object&0b11) << bit_start;
	SET(walls[col][page], mask, msg);		
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
	byte snake_page = (pt.y / SNAKE_ROWS_PER_PAGE) % MAX_SNAKE_PAGE;
	byte bit_start = SNAKE_ROW_BIT_SIZE*(pt.y % SNAKE_ROWS_PER_PAGE);
	
	// select applicable wall data
	byte offset, wall_data = walls[col][snake_page];
	if (bit_start < 4) {
		offset = 0;
	} else {
		offset = 2*SNAKE_ROW_BIT_SIZE;
	}
	

	// transcribe to pixel data
	byte i, j, shift;
	byte pixel_data = 0x00;
	byte wall_mask, pixel_mask;
	for (i=0; i<2; i++) {
		shift = (offset + i*SNAKE_ROW_BIT_SIZE);
		wall_mask = 0b11 << shift;
		pixel_mask = 0x0F << i*SNAKE_WIDTH;
		if ((GET(wall_data, wall_mask)) == 0) {
			SET(pixel_data, pixel_mask, OFF);
		} else {
			SET(pixel_data, pixel_mask, ON);
		}
		/*for (j=0; j<SNAKE_WIDTH; j++) {
			switch (GET(wall_data, wall_mask)) {
				case EMPTY:
					SET(pixel_data[j], pixel_mask, OFF);
					break;
				case WALL:
					SET(pixel_data[j], pixel_mask, ON);
					break;
				default:
					break;
			}
		}*/
	}


	//Select pixel locations and draw
	byte left_column = (SNAKE_WIDTH*pt.x)%MAX_COLUMN;
	byte page = ((SNAKE_WIDTH*pt.y)/PIXEL_PER_PAGE)%MAX_PAGE;
	select_page(page);
	for (i=0; i < SNAKE_WIDTH; i++) {
		select_column(left_column+i);
		LCD_data_tx(pixel_data);
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
 * Function:  redraw_all_walls
 * ----------------------------
 * Debugging function for checking that all walls have been drawn. Displays a 
 * 'mini-map' in the top-left corner of the screen.
 *
 */
 
void draw_all_walls(void) {
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
	uint8_t i, elem, page, column;
	byte mask, data, pixel_data = 0x00;
	for (column=0; column<MAX_SNAKE_COLUMN; column++) {
		for (page=0; page<MAX_SNAKE_PAGE; page++) {
			data = walls[column][page];
			for (i=0; i<4; i++) {
				mask = 0b11 << 2*i;
				elem = i + 4*(page%2);
				if ((GET(data,mask)) != 0x00) {
					SET(pixel_data, _BV(elem), ON);
				} else {
					SET(pixel_data, _BV(elem), OFF);
				}
			}
			if (page%2 == 1 || (page == MAX_SNAKE_PAGE-1)) {
				select_column(column);
				select_page(page/2);
				LCD_data_tx(pixel_data);
				pixel_data = 0x00;
			}
		}
	}
	return;
}
