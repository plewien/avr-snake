/*************************************************************************
Title:    Snake Header File
Author : Patrick Lewien (694555)
Software: AVR-GCC 
Hardware: ATMEGA16 @ 8Mhz 

DESCRIPTION:
	Macros for Snake game.

*************************************************************************/

#ifndef _SNAKE_H_
#define _SNAKE_H_

#include <stdlib.h>

// Struct declarations
typedef struct {
	byte x;
	byte y;
} point_t;

typedef enum direction_t {UP,DOWN,LEFT,RIGHT} direction_t;

typedef struct node node_t;
struct node {
    point_t pos;
	byte length;
	direction_t dir;
    node_t* ptr;
};

typedef struct {
	byte length;
	byte max_length;
	node_t* head;
	node_t* tail;
} snake_t;

// Game function declarations
void 		play_snake_game(void);
void 		reset_snake_game(snake_t* snake);
void		srand_adc(void);
byte 		is_wall(point_t pt);
void 		clear_walls(void);
void 		check_wall_collision(snake_t* snake);
bool		equal_pts(point_t pt1, point_t pt2);

// Snake function declarations
point_t 	move_snake(snake_t* snake, direction_t dir);
snake_t* 	create_snake(point_t starting_pos, direction_t dir);
point_t 	add_to_head(snake_t* snake, direction_t dir);
void 		push_head(snake_t* snake, point_t s_pos, direction_t dir);
point_t 	get_head_position(snake_t* snake);
point_t 	remove_from_tail(snake_t* snake);
point_t 	pop_tail(snake_t* snake);
point_t 	pop_tail_tip(node_t* tail);
void 		increase_length(snake_t* snake);
void 		clear_snake(snake_t* snake);
point_t 	move_pos(point_t pt, direction_t dir, byte dist);

// Food function declarations
point_t		generate_food(void);
point_t 	generate_random_location(void);
point_t		check_food_collision(snake_t* snake, point_t food);

// Drawing function declarations
byte 		write_pixel(point_t pt, byte value);
void 		draw(point_t s_pos);
void		draw_food(point_t pt);
void 		clear(point_t s_pos);
byte 		get2bits(byte data, byte pixel);
byte 		interleave(byte x);
byte 		interleave2(byte x);
void 		redraw_all_walls(void);


//Snake Interface
#define SPEED 				200 //ms
#define SNAKE_WIDTH			4
#define MAX_SNAKE_COLUMN 	MAX_COLUMN/SNAKE_WIDTH
#define MAX_SNAKE_ROW		MAX_ROW/SNAKE_WIDTH
#define MAX_SNAKE_PAGE		MAX_PAGE/SNAKE_WIDTH
#define MAX_SEED			MAX_SNAKE_COLUMN*MAX_SNAKE_ROW

#define START_X				MAX_SNAKE_COLUMN/2
#define START_Y				MAX_SNAKE_ROW/2
#define START_LENGTH		5
#define LENGTH_DELTA		5

/*** End of Snake Header File ****/
#endif