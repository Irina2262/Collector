#include "func.h"

uint8_t move_dron(struct head_t* head) {
	size_t size = head->tsize;
	uint8_t temp_x, temp_y, x = head->x, y = head->y, x1 = head->x - 1, y1 = head->y - 1, res = 1;
	setColors(head->id);
	mvprintw(head->y, head->x, "%s", " ");
	switch (head->direction) {
		case LEFT:
			if (head->x == 1) {
				if (autoChangeDirection(head)) {
					autoMotionDron(head);
				} else {
					res = 0;
				}
			}
			mvprintw(head->y, --(head->x), "%c", DRON);
			break;
		case RIGHT:
			if (head->x == WIDTH - 2) {
				if (autoChangeDirection(head)) {
					autoMotionDron(head);
				} else {
					res = 0;
				}
			}
			mvprintw(head->y, ++(head->x), "%c", DRON);
			break;
		case UP:
			if(head->y == 1) {
				if (autoChangeDirection(head)) {
					autoMotionDron(head);
				} else {
					res = 0;
				}
			}
			mvprintw(--(head->y), head->x, "%c", DRON);
			break;
		case DOWN:
			if(head->y == LENGTH - 1) {
				if (autoChangeDirection(head)) {
					autoMotionDron(head);
				} else {
					res = 0;
				}
			}
			mvprintw(++(head->y), head->x, "%c", DRON);
			break;
		default:
			break;
	}

	tail_t* tail = head->tail;
	for (size_t i = 0; i < size; ++i) {
		temp_x = tail[i].x;
		temp_y = tail[i].y;

		tail[i].x = x;
		tail[i].y = y;

		mvprintw(y, x, "%c", TAIL);

		x = temp_x;
		y = temp_y;
	}
	mvprintw(y, x, " ");
	return res;
}

void changeDirection(head_t *head, const int32_t key) {
	if (key == head->controls.down)
		head->direction = DOWN;
	else if (key == head->controls.up)
		head->direction = UP;
	else if (key == head->controls.right)
		head->direction = RIGHT;
	else if (key == head->controls.left)
		head->direction = LEFT;
}

int checkDirection(head_t *head, int32_t key) {
	if (key == head->controls.down && head->direction == UP) {
		return 0;
	} else if (key == head->controls.up && head->direction == DOWN) {
		return 0;
	} else if (key == head->controls.right && head->direction == LEFT) {
		return 0;
	} else if (key == head->controls.left && head->direction == RIGHT) {
		return 0;
	} else {
		return 1;
	}
}
