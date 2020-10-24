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


address_t pt2display(point_t pt) {
	address_t display;
	display.column = (SNAKE_WIDTH*pt.x)%MAX_COLUMN;
	display.page = ((SNAKE_WIDTH*pt.y)/PIXEL_PER_PAGE)%MAX_PAGE;
	display.bit = 0;
	return display;
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
	
	
	// select applicable wall data
	address_t loc = pt2bufferaddress(pt);
	byte offset, wall_data = walls[loc.column][loc.page];
	if (loc.bit < 4) {
		offset = 0;
	} else {
		offset = 2*SNAKE_ROW_BIT_SIZE;
	}
	
	// transcribe to pixel data
	byte i, j, shift;
	byte pixel_data[SNAKE_WIDTH] = {0x00};
	byte wall_mask, pixel_mask;
	for (i=0; i<2; i++) {
		shift = (offset + i*SNAKE_ROW_BIT_SIZE);
		wall_mask = 0b11 << shift;
		pixel_mask = 0x0F << i*SNAKE_WIDTH;
		for (j=0; j<SNAKE_WIDTH; j++) {
			switch (GET(wall_data, wall_mask)>>shift) {
				case EMPTY:
					SET(pixel_data[j], pixel_mask, OFF);
					break;
				case WALL:
					SET(pixel_data[j], pixel_mask, ON);
					break;
				case FOOD:
					SET(pixel_data[j], pixel_mask, ON);
				default:
					break;
			}
		}
	}


	//Select pixel locations and draw
	address_t display = pt2display(pt);
	lcd_moveto_xy(display.page, display.column);
	for (i=0; i < SNAKE_WIDTH; i++) {
		lcd_data(pixel_data[i]);
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
	update_buffer(pt, WALL);
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
	update_buffer(pt, EMPTY);
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





