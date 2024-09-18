#include "func.h"

void initTail(struct tail_t t[], size_t size, uint8_t x, uint8_t y) {
	for (size_t i = 0; i < size; ++i) {
		t[i].x = x + i + 1;
		t[i].y = y;
	}
}

void initHead(struct head_t *head, uint8_t x, uint8_t y) {
	head->x = x;
	head->y = y;
	head->direction = LEFT;
}

void initDron(struct head_t *head, size_t size, uint8_t id) {
	tail_t* tail = (tail_t*) malloc(MAX_TAIL_SIZE * sizeof(tail_t));
	initTail(tail, size, head->port.x - 2, head->port.y);
	initHead(head, head->port.x - 2, head->port.y);
	head->id = id;
	head->tail = tail;
	head->tsize = size;
	head->load = 0;
	head->controls = default_controls;

	setColors(id);
	mvprintw(head->y, head->x, "%c", DRON);
}

int checkTargetPoint(uint8_t x, uint8_t y) {
	if (plants[x][y] == 0) {
		plants[x][y] = 2;
		return 1;
	}
	return 0;
}

void getTargetPoint() {
	int x, y;

	y = get_rand_range_int(2, LENGTH - 1);
	while (1) {
		if (y < LENGTH - 1 && !(y & 1)) {
			break;
		}
		y = get_rand_range_int(2, LENGTH - 1);
	}

	x = get_rand_range_int(2, WIDTH - 2);
	while (1) {
		if (x < WIDTH - 2) {
			break;
		}
		x = get_rand_range_int(2, WIDTH - 2);
	}

	if (checkTargetPoint((y - 1), x - 1)) {
		point.x = x;
		point.y = y;
	} else {
		getTargetPoint();
	}
}

void insertTarget() {
	getTargetPoint();

	setColors(TARGET_GOOD_PAIR);
	mvprintw(point.y, point.x, "%c", TARGET);
}

_Bool haveEat(struct head_t *head) {
	return plants[head->y - 1][head->x - 1] == 2;
}

_Bool addTail(struct head_t *head) {
    if(head == NULL || head->tsize >= MAX_TAIL_SIZE) {
        mvprintw(LENGTH + 1, 0, "Can't add tail to dron %d", head->id);
        return 0;
    }
    head->tsize++;
    return 1;
}

_Bool postProcessingOfMovement(struct head_t *head) {
	if (haveEat(head)) {
		plants[head->y - 1][head->x - 1] = 1;
		addTail(head);
		uint32_t amount = 0;
		for (size_t i = 0; i < DRON_NUMBER; ++i) {
			amount += head->tsize + head->load * MAX_TAIL_SIZE;
		}
		return amount < MAX_PLANT_SIZE;
	}
	return 1;
}

void checkUnloadDron(struct head_t *head) {
	if (head->tsize == MAX_TAIL_SIZE && head->x == head->port.x - 2
			&& head->y == head->port.y) {

		setColors(TARGET_GOOD_PAIR);
		tail_t* tail = head->tail;
		uint8_t y = head->port.y + head->load, x = head->port.x + 2;
		for (size_t i = 0; i < head->tsize; ++i) {
			mvprintw(tail[i].y, tail[i].x, " ");
			mvprintw(y, x + i, "%c", TARGET);
		}

		head->tsize = START_TAIL_SIZE;
		++(head->load);
	}
}

uint8_t isCrushDrons(struct head_t *current) {
	uint8_t x = point.x;
	uint8_t y = point.y;
	for (size_t i = 0; i < DRON_NUMBER; ++i) {
		if (current == arr_head[i]) {
			continue;
		}
		head_t *head = arr_head[i];
		if (x == head->x && y == head->y) {
			return 1;
		}
		tail_t *tail = head->tail;
		size_t size = head->tsize;
		for(size_t i = 0; i < size; i++) {
			if (x == tail[i].x && y == tail[i].y) {
				return 1;
			}
		}
	}
	return 0;
}

uint8_t isCrush(head_t *head) {
	tail_t *tail = head->tail;
	size_t size = head->tsize;
	uint8_t x = point.x;
	uint8_t y = point.y;
	for(size_t i = 0; i < size; i++) {
		if (x == tail[i].x && y == tail[i].y) {
			return 1;
		}
	}

	if (isCrushDrons(head)) {
		return 1;
	}

	return 0;
}

uint8_t isCrushExtended(head_t *head, int direction) {
	tail_t *tail = head->tail;
	size_t i, min_size = 5, size = head->tsize;
	uint8_t x = point.x;
	uint8_t y = point.y;
	if (size > min_size) {
		size_t cnt = size - min_size > min_size ? min_size : size - min_size;
		for (; cnt > 0; cnt--) {
			for (i = min_size; i < size; i++) {
				if (x == tail[i].x && y == tail[i].y) {
					return 1;
				}
			}
			if (direction == RIGHT) {
				x++;
			} else {
				x--;
			}
		}
	}

	if (isCrushDrons(head)) {
		return 1;
	}

	return 0;
}
