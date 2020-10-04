/*************************************************************************
Title: Snake_Game
Author: Patrick Lewien (694555)
Software: AVR-GCC 
Hardware: ATMEGA16 @ 8Mhz 

DESCRIPTION:
	Snake is a game in which the goal is to achieve the longest snake 
	possible without crashing into yourself. The snake gets longer every time
	it eats 'food'.


	Arrow keys: Move direction
	A-button: Clear screen
	B-button: Change brightness

*************************************************************************/

#include "console.h"
#include "snake.h"

volatile byte walls[MAX_SNAKE_COLUMN][MAX_SNAKE_PAGE] = {{ OFF }};
extern direction_t direction;


ISR(RESET
/*
 * Function:  play_snake_game
 * ---------------------------
 * Main function to move the snake through the game field. The snake is structured as a 
 * single-ended linked list, in order to use dynamic memory allocation to extend the 
 * snake's length as the game progresses. The data structure acts like a queue, with O(1)
 * complexity to add to the front of the list and remove from the end.
 *
 * Each node in the linked list contains a POSITION, DIRECTION and LENGTH. An alternative
 * would be to save each pixel as a node. As shown by the example, this method saves memory 
 * space, since the amount of memory used is agnostic of the length of each snake segment.
 *
 * Example: 
 *
 *		*********************************************
 *		*											*
 *		*	------------o							*
 *		*				|							*
 *		*				|							*
 *		*				|							*
 *		*				o-----------o -->			*
 *		*											*
 *		*											*
 *		*********************************************
 *	
 * The arrow represents the direction being travelled. The 'o' is the position of the snake's
 * head, and the '---' denotes the length of the snake.
 *
 */
void play_snake_game() {
	point_t tail, head = {.x = START_X, .y = START_Y};
	snake_t* snake = create_snake(head, direction);
	srand_adc();
	point_t food = generate_food();
	
	while (TRUE) {
		head = add_to_head(snake, direction); 
		food = check_food_collision(snake, food);
		check_wall_collision(snake);
		draw(head);  // Only draw head once the collision has been checked

		while (snake->length >= snake->max_length) {
			tail = remove_from_tail(snake);
			clear(tail);
		}
		//draw_all_walls();
		draw_minimap();
		_delay_ms(SPEED); // Pause before drawing next pixel
	}
	
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
}


/*
 * Function:  reset_snake_game
 * ----------------------------
 * The game should be reset whenever it is game-over. This involves clearing the
 * screen and freeing the memory associated with the walls and snake. A game-over
 * message should be displayed on the screen for the user too.
 *
 */
void reset_snake_game(snake_t* snake) {
	LCD_clear();
	clear_snake(snake);
	clear_walls();

	// TODO: Display game-over.
	return;
}


/*
 * Function:  is_wall
 * -------------------
 * Determines if the given point is a wall or free space.
 *
 *  returns: True, if the given point is a wall.
 *
 */
byte is_wall(point_t pt) {
	
	// TODO: Check if the wall is actually a food?
	// TODO: Return false if out of bounds
	byte col = pt.x % MAX_SNAKE_COLUMN;
	byte page = (pt.y / SNAKE_ROWS_PER_PAGE) % MAX_SNAKE_PAGE;
	byte bit_start = SNAKE_ROW_BIT_SIZE*(pt.y % SNAKE_ROWS_PER_PAGE);
	byte object = GET(0x11, walls[col][page] >> bit_start);
	return object == WALL;
}


/*
 * Function:  clear_walls
 * -----------------------
 * Removes all walls from memory.
 *
 */
void clear_walls(void) {
	byte i, j;
	for (i = 0; i < MAX_SNAKE_COLUMN; i++) {
		for (j = 0; j < MAX_SNAKE_PAGE; j++) {
			walls[i][j] = OFF;
		}
	}
	// TODO: Redraw the food
}



/*
 * Function:  equal_pts
 * ---------------------
 * Checks if the location of two points is the same. Can't just return the logical 
 * here for some reason?
 *
 *  returns: True, if they are the same.
 *
 */
bool equal_pts(point_t pt1, point_t pt2) {
	if (pt1.x == pt2.x && pt1.y == pt2.y) return TRUE;
	else return FALSE;
}

/*
 * Function:  generate_food
 * ------------------------
 * The food should not be generated anywhere. It must not be placed where the snake 
 * is currently residing. This function iterates over the random location generator
 * until a free space is found.
 *
 *  returns: The location of the generated food.
 *
 */
point_t generate_food(void) {
	point_t food;

	do { food = generate_random_location(); } 
	while(is_wall(food));

	draw(food);
	return food;
}


/*
 * Function:  generate_random_location
 * ------------------------------------
 * Randomly generates a location for the food.
 *
 *  returns: The location.
 *
 */
point_t generate_random_location(void) {
	point_t pt;
	pt.x = rand() % MAX_SNAKE_COLUMN;
	pt.y = rand() % MAX_SNAKE_ROW;
	return pt;
}


/*
 * Function:  check_food_collision
 * --------------------------------
 * Checks if the snake has run into the food. If the snake has, then 
 * a new food is generated and the snake's length is increased. Otherwise,
 * the same food is returned.
 *
 *  snake: The linked-list representing the snake.
 *	food: The location of the food.
 *
 *  returns: The location of the food.
 *
 */
point_t check_food_collision(snake_t* snake, point_t food) {
	
	if (equal_pts(get_head_position(snake), food)) {
		increase_length(snake);
		clear(food);  // Need this? Snake will eat over the food
		return generate_food();
	} else {
		return food;
	}
}


/*
 * Function:  check_wall_collision
 * --------------------------------
 * Checks if the snake has run into itself, resetting the game if that is the case.
 *
 *  snake: The linked-list representing the snake.
 *	food: The location of the food.
 *
 *  returns: void.
 *
 */
void check_wall_collision(snake_t* snake) {
	if (is_wall(get_head_position(snake))) {
		reset_snake_game(snake);
	}
}
