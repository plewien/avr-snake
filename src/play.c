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
extern direction_t selected_direction;


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
	direction_t direction = RIGHT;
	snake_t* snake = create_snake(head, direction);
	point_t food = generate_food();
	
	while (TRUE) {
		direction = update_direction(direction);
		head = add_to_head(snake, direction); 
		food = check_food_collision(snake, food);
		if (is_wall(head))  break;
		
		// Only draw head once the collision has been checked
		draw(head);
		write_score(snake->length);  
		while (snake->length >= snake->max_length) {
			tail = remove_from_tail(snake);
			clear(tail);
		}
		//draw_minimap();

		// Pause before drawing next pixel
		_delay_ms(SPEED); 
	}
	
	end_snake_game(snake);
}

direction_t update_direction(direction_t current) {
	direction_t update = current;
	switch (selected_direction) {
		case UP:
			if (current != DOWN) update = UP;
			break;
		case DOWN:
			if (current != UP) update = DOWN;
			break;
		case LEFT:
			if (current != RIGHT) update = LEFT;
			break;
		case RIGHT:
			if (current != LEFT) update = RIGHT;
			break;
		case NONE:
		default:
			update = current;
	}
	return update;
}

/*
 * Function:  end_snake_game
 * ----------------------------
 * The game should be reset whenever it is game-over. This involves clearing the
 * screen and freeing the memory associated with the walls and snake. A game-over
 * message should be displayed on the screen for the user too.
 *
 */
void end_snake_game(snake_t* snake) {
	LCD_clear();
	clear_snake(snake);
	clear_walls();
	return;
}

void update_buffer(point_t pt, obj_t object) {
	address_t location = pt2bufferaddress(pt);
	byte mask = _BV(location.bit) | _BV(location.bit+1);
	byte msg = (object&0b11) << location.bit;
	SET(walls[location.column][location.page], mask, msg);		
	return;
}

address_t pt2bufferaddress(point_t pt) {
	address_t location;
	location.column = pt.x % MAX_SNAKE_COLUMN;
	location.page = (pt.y / SNAKE_ROWS_PER_PAGE) % MAX_SNAKE_PAGE;
	location.bit = SNAKE_ROW_BIT_SIZE*(pt.y % SNAKE_ROWS_PER_PAGE);
	return location;
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
	address_t loc = pt2bufferaddress(pt);
	byte object = GET(0x11, walls[loc.column][loc.page] >> loc.bit);
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

	draw_food(food);
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
