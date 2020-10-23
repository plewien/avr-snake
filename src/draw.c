/*************************************************************************
Title: Snake_Draw
Author: Patrick Lewien (694555)
Software: AVR-GCC 
Hardware: ATMEGA16 @ 8Mhz 

DESCRIPTION:


*************************************************************************/

#include "console.h"
#include "snake.h"
#include "dogm-graphic.h"

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
	lcd_moveto_xy(page, left_column);
	for (i=0; i < SNAKE_WIDTH; i++) {
		lcd_data(pixel_data);
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
				lcd_moveto_xy(page/2, column);
				lcd_data(pixel_data);
				pixel_data = 0x00;
			}
		}
	}
	return;
}

void write_score(uint8_t score) {
	lcd_moveto_xy(7,2);
	lcd_putstr("score:");
	lcd_moveto_xy(7,80);
	lcd_put_uint(score);
	return;
}





