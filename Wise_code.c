#include "func.h"

void findTargetPoint(uint8_t *target_x, uint8_t *target_y, uint8_t x, uint8_t y) {
	size_t j, i = y - ROW_SCAN < 0 ? 0 : y - ROW_SCAN, i_max = y + 5 > ROW ? ROW : y + 5;
	uint32_t temp, min_distance = ROW * ROW + COLUMN * COLUMN;

	if (!(y & 1)) {
		*target_x = 0;
		*target_y = 0;
		for (j = 0; j < COLUMN; ++j) {
			if (y > 0 && plants[y - 1][j] == 2) {
				temp = (j - x) * (j - x);
				if (temp < min_distance) {
					min_distance = temp;
					*target_x = j;
					*target_y = y - 1;
				}
			} else if (y < ROW - 1 && plants[y + 1][j] == 2) {
				temp = (j - x) * (j - x);
				if (temp < min_distance) {
					min_distance = temp;
					*target_x = j;
					*target_y = y + 1;
				}
			}
		}

		if (*target_x != 0 && *target_y != 0) {
			return;
		}
	}

	for (; i < i_max; ++i) {
		if (i == y - 1 || i == y + 1) {
			continue;
		}
		for (j = 0; j < COLUMN; ++j) {
			if (plants[i][j] == 2) {
				temp = (j - x) * (j - x) + (i - y) * (i - y);
				if (temp < min_distance) {
					min_distance = temp;
					*target_x = j;
					*target_y = i;
				}
			}
		}
	}

}

uint8_t autoChangeDirection(struct head_t* head) {
	uint8_t x = head->x - 1, y = head->y - 1, res = 1;
	switch (head->direction) {
		case LEFT:
		case RIGHT:
			if (y < head->target->y) {
				point.x = head->x; point.y = head->y + 2;
				if (y < ROW - 1 && !isCrush(head) && plants[y + 1][x] > 0) {
					head->direction = DOWN;
				} else if (y > 0 && plants[y - 1][x] > 0) {
					head->direction = UP;
				} else {
					res = 0;
				}

			} else {
				point.x = head->x; point.y = head->y - 2;
				if (y > 0 && !isCrush(head) && plants[y - 1][x] > 0) {
					head->direction = UP;
				} else if (y < ROW - 1 && plants[y + 1][x] > 0) {
					head->direction = DOWN;
				} else {
					res = 0;
				}
			}
			break;
		case UP:
		case DOWN:
			if (x < head->target->x) {
				point.x = head->x + 1; point.y = head->y;
				if (x < COLUMN - 1 && !isCrushExtended(head, RIGHT) && plants[y][x + 1] > 0) {
					head->direction = RIGHT;
				} else if (x > 0 && plants[y][x - 1] > 0){
					head->direction = LEFT;
				} else {
					res = 0;
				}

			} else {
				point.x = head->x - 1; point.y = head->y;
				if (x > 0 && !isCrushExtended(head, LEFT) && plants[y][x - 1] > 0) {
					head->direction = LEFT;
				} else if (x < COLUMN - 1 && plants[y][x + 1] > 0) {
					head->direction = RIGHT;
				} else {
					res = 0;
				}
			}
			break;
		default:
			res = 0;
			break;
	}

	return res;
}

uint8_t autoMotionDron(struct head_t *head) {
	uint8_t target_x, target_y, x = head->x - 1, y = head->y - 1, res = 1;
	_Bool may_loading, predicat1, predicat2;

	if (head->tsize == MAX_TAIL_SIZE) {
		target_x = head->port.x - 3;
		target_y = head->port.y - 1;
		may_loading = FALSE;
	} else {
		uint8_t *ptr_x = &target_x;
		uint8_t *ptr_y = &target_y;
		findTargetPoint(ptr_x, ptr_y, x, y);
		may_loading = TRUE;
	}


	if (head->target == NULL) {
		plant_t* target = (plant_t*) malloc(sizeof(plant_t));
		target->x = target_x;
		target->y = target_y;
		head->target = target;
	} else {
		head->target->x = target_x;
		head->target->y = target_y;
	}

	switch (head->direction) {
		case UP:

			if (may_loading) {
				predicat1 = plants[y][x + 1] > 0;
				predicat2 = plants[y][x - 1] > 0;
			} else {
				predicat1 = plants[y][x + 1] == 1;
				predicat2 = plants[y][x - 1] == 1;
			}

			if (x < target_x && target_y + 1 == y && predicat1) {
				point.x = head->x + 1; point.y = head->y;
				if (!isCrushExtended(head, RIGHT)) {
					head->direction = RIGHT;
				}

			} else if (x > target_x && target_y + 1 == y && predicat2) {
				point.x = head->x - 1; point.y = head->y;
				if (!isCrushExtended(head, LEFT)) {
					head->direction = LEFT;
				}
			} else {
				point.x = head->x; point.y = head->y - 2;
				if (y == 0 || plants[y - 1][x] == 0 || isCrush(head)) {
					res = autoChangeDirection(head);
				}
			}

			if (res) {
				res = move_dron(head);
			}

			break;

		case RIGHT:

			if (may_loading) {
				predicat1 = plants[y - 1][x] > 0;
				predicat2 = plants[y + 1][x] > 0;
			} else {
				predicat1 = plants[y - 1][x] == 1;
				predicat2 = plants[y + 1][x] == 1;
			}

			if (y >= target_y + 1 && y > 0 && predicat1) {
				point.x = head->x; point.y = head->y - 2;
				if (!isCrush(head)) {
					head->direction = UP;
				}
			} else if (y + 1 <= target_y && y < ROW - 1 && predicat2) {
				point.x = head->x; point.y = head->y + 2;
				if (!isCrush(head)) {
					head->direction = DOWN;
				}

			} else {
				point.x = head->x + 1; point.y = head->y;
				if (head->x == WIDTH - 2 || plants[y][x + 1] == 0 || isCrushExtended(head, RIGHT)) {
					res = autoChangeDirection(head);
				}
			}

			if (res) {
				res = move_dron(head);
			}

			break;

		case DOWN:

			if (may_loading) {
				predicat1 = plants[y][x + 1] > 0;
				predicat2 = plants[y][x - 1] > 0;
			} else {
				predicat1 = plants[y][x + 1] == 1;
				predicat2 = plants[y][x - 1] == 1;
			}

			if (x < target_x && target_y - 1 == y && predicat1) {
				point.x = head->x + 1; point.y = head->y;
				if (!isCrushExtended(head, RIGHT)) {
					head->direction = RIGHT;
				}

			} else if (x > target_x && target_y - 1 == y && predicat2) {
				point.x = head->x - 1; point.y = head->y;
				if (!isCrushExtended(head, LEFT)) {
					head->direction = LEFT;
				}
			} else {
				point.x = head->x; point.y = head->y + 2;
				if (head->y == LENGTH - 1 || plants[y + 1][x] == 0 || isCrush(head)) {
					res = autoChangeDirection(head);
				}
			}

			if (res) {
				res = move_dron(head);
			}

			break;

		case LEFT:

			if (may_loading) {
				predicat1 = plants[y - 1][x] > 0;
				predicat2 = plants[y + 1][x] > 0;
			} else {
				predicat1 = plants[y - 1][x] == 1;
				predicat2 = plants[y + 1][x] == 1;
			}

			if (y >= target_y + 1 && y > 0 && predicat1) {
				point.x = head->x; point.y = head->y - 2;
				if (!isCrush(head)) {
					head->direction = UP;
				}

			} else if (y + 1 <= target_y && y < ROW - 1 && predicat2) {
				point.x = head->x; point.y = head->y + 2;
				if (!isCrush(head)) {
					head->direction = DOWN;
				}
			} else {
				point.x = head->x - 1; point.y = head->y;
				if (x == 0 || plants[y][x - 1] == 0 || isCrushExtended(head, LEFT)) {
					res = autoChangeDirection(head);
				}
			}

			if (res) {
				res = move_dron(head);
			}

			break;
		default:
			break;
	}

	return res;
}
