/*************************************************************************
Title: Snake
Author: Patrick Lewien (694555)
Software: AVR-GCC 
Hardware: ATMEGA16 @ 8Mhz 

DESCRIPTION:

	Arrow keys: Move direction
	A-button: Clear screen
	B-button: Change brightness

*************************************************************************/

#include "console.h"
#include "snake.h"

	
/*
 * Function:  create_snake
 * -----------------------
 * Creates a snake to be displayed on the screen. Acts as a linked list.
 *
 *  starting_pos: The initial location of the snake.
 *
 *  returns: The initialised snake.
 *
 */
snake_t* create_snake(point_t starting_pos, direction_t dir) {
	snake_t* snake = malloc(sizeof(snake_t));
	snake->length = 1;
	snake->max_length = START_LENGTH;
	push_head(snake, starting_pos, dir);
	draw(starting_pos);
	return snake;
}

/*
 * Function:  increase_length
 * ---------------------------
 * Adds to the snake's total possible length by a specified delta.
 *
 *  snake: The linked-list representing the snake.
 *
 */
void increase_length(snake_t* snake) {
	snake->max_length += LENGTH_DELTA;
	return;
}

/*
 * Function:  add_to_head
 * ----------------------
 * Shifts the snake's head forward by one, in the currently polled direction. There
 * are two options:
 *	  (1) the snake is still moving in the same direction. If this is the case, move 
 * 			the head forward by one and add to the head node's length.
 *	  (2) the snake has changed direction. When this happens, a new node is required. 
 *
 *  snake: The linked list representing the snake.
 *  dir: The currently-polled direction.
 *
 *  returns: void.
 *
 */
 point_t add_to_head(snake_t* snake, direction_t dir) {
	
	//Move snake head in currently-polled direction
	point_t move = move_pos(get_head_position(snake), dir, 1);
	
	// Check whether snake is moving in the same direction
	if (dir == snake->head->dir) {
		snake->head->pos = move;  // Update the snake head
		(snake->head->length)++;
	} else {
		push_head(snake, move, dir);
	}

	(snake->length)++;
	return move;
}


/*
 * Function:  push_head
 * --------------------
 * Adds a data point onto the front of the linked list.
 *
 *  snake: The linked list.
 *  s_pos: The data point.
 *
 *  returns: True, if the data is successfully pushed. (TODO).
 *
 */
void push_head(snake_t* snake, point_t s_pos, direction_t dir) {
	node_t *n = malloc(sizeof(node_t));
	assert(n != NULL);
	n->pos = s_pos;
	n->length = 1;
	n->dir = dir;
	n->ptr = NULL;
	
	if (snake->head == NULL) {	
		snake->tail = n;
	} else {
		snake->head->ptr = n;
	}
	snake->head = n;
}


/*
 * Function:  get_head_position
 * --------------------
 * The snake's head is the most important object in the game. It is used to check 
 * collisions with walls, food and itself. This function returns the head's current 
 * position.
 *
 *  snake: The linked-list representation of the snake.
 *
 *  returns: The position of the snake's head.
 *
 */
point_t get_head_position(snake_t* snake) {

	return snake->head->pos;
}

/*
 * Function:  remove_from_tail 
 * ----------------------------
 * Recieves the end of the snake from the queue. This can be one of two cases.
 * 	 (1) the snake section is of length 1. If this is the case, then the section needs
 *			to be removed from the queue. The function pop_tail() is called.
 *	 (2) the snake section is of length greater than 1. If this is the case, then the 
 *			section of snake only needs to be reduced, with no adjustment to the linked
 *			list is necessary.
 * In either situation, the snake's total length reduces by 1.
 *
 *  snake: The linked list.
 *
 *  returns: The position corresponding to the last pixel of the snake.
 *
 */
point_t remove_from_tail(snake_t* snake) {
	
	(snake->length)--;	

	if (snake->tail->length == 1) {
		return pop_tail(snake);
	} else {
		return pop_tail_tip(snake->tail);
	}
}

/*
 * Function:  pop_tail 
 * --------------------
 * Removes the tail from a linked list, returning its value.
 *
 *  snake: The linked list.
 *
 *  returns: The position corresponding to the last element of the list.
 *
 */
point_t pop_tail(snake_t* snake) {	
	
	// generate the value to be returned before freeing the memory
	point_t retval = snake->tail->pos;
	node_t* temp = snake->tail->ptr;
	
    // if there is only one item in the list, remove it
    if (temp == NULL) {
        free(snake->tail);
		snake->head = NULL;
		snake->tail = NULL;
    } 
	
	// otherwise, remove tail and make it the next one
	else {
		free(snake->tail);
		snake->tail = temp;
	}
	
	// now return the tail's position
    return retval;
}

/*
 * Function:  pop_tail_tip 
 * ------------------------
 * Returns the tail tip from the snake.
 *
 *  snake: The linked list.
 *
 *  returns: The position corresponding to the tail tip.
 *
 */
point_t pop_tail_tip(node_t* tail) {

	point_t pos = tail->pos;
	direction_t dir = tail->dir;
	byte dist = 1-(tail->length);  // negative, since dir is opposite.
	
	(tail->length)--;
	return move_pos(pos, dir, dist);
}

/*
 * Function:  clear_snake 
 * ------------------------
 * Clears all memory associated with the snake.
 *
 *  snake: The linked list.
 *
 *  returns: void.
 *
 */
void clear_snake(snake_t* snake) {
	while (snake->tail != NULL) {
		pop_tail(snake);
	}
	free(snake);
}

/*
 * Function:  move_pos
 * ----------------------
 *  Finds a new position relative to a given position.
 *
 *  pos: The position to be moved.
 *	dir: The direction to be moved in.
 *	dist: How far the point moves in that direction.
 *
 *  returns: A new point, shifted from the input point.
 *
 */
point_t move_pos(point_t pos, direction_t dir, byte dist) {

	//Shift the new position in the given direction
	switch (dir) {
		case UP: 	pos.y -= dist; break;
		case DOWN: 	pos.y += dist; break;
		case LEFT: 	pos.x -= dist; break;
		case RIGHT:	pos.x += dist; break;
	}

	//Handle reaching the edge of the screen
	pos.y = bound_check(pos.y, 0, MAX_SNAKE_ROW);
	pos.x = bound_check(pos.x, 0, MAX_SNAKE_COLUMN);

	return pos;
}



