#include <ncurses.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
//#include "func.h"

#define WIDTH 45
#define LENGTH 26
#define TARGET 'o'
#define DRON '@'
#define TAIL '+'
#define SIDE_BOARD '|'
#define HORIZONT_BOARD '-'
#define DRON1_PAIR 1
#define DRON2_PAIR 2
#define TARGET_GREEN_PAIR 3
#define TARGET_GOOD_PAIR 4
#define CODE_KEY_END 27
#define LAG 10
#define DELAY 200


enum {LEFT = 1, RIGHT, UP, DOWN, ROW_SCAN,STOP_GAME = KEY_END
	, AUTO_MODE = 'a', HAND_MODE = 'h', PAUSE_PLAY = 'p', SEMI_AUTO_MODE = 's'};
enum {START_TAIL_SIZE = 0, SPEED_LEVEL = 5, DRON_NUMBER = 2
	, ROW = LENGTH - 1, COLUMN = WIDTH - 2, MAX_TAIL_SIZE = 10
	, MAX_PLANT_SIZE = COLUMN * ROW / 2 };

uint8_t plants[ROW][COLUMN];

char *DESCRIPTION = {"Robot-collector "
		"implements the automatic mode on default\n"
		"However, you can control the yellow drone.\n"
		"You can select mode control press the keys:\n"
		"'a' - full-auto, 's' - semi-auto and 'h' - hand\n"
		"To set the direction of movement, press the keys UP, DOWN, RIGHT and LEFT.\n"
		"To make pause, press the keys 'p'.\n"
		"To exit the game, press END key.\n"};


int showStartMenu() {
	int ret;
	mvprintw(0, 0, "%s", DESCRIPTION);
	ret = getch();
	return ret;
}

struct control_buttons {
	int down;
	int up;
	int right;
	int left;
} control_buttons;

struct control_buttons default_controls = {KEY_DOWN, KEY_UP, KEY_RIGHT, KEY_LEFT};

typedef struct point_t {
	uint8_t x;
	uint8_t y;
} point_t;

struct point_t point = {0,0};
struct point_t port1 = {WIDTH,1};
struct point_t port2 = {WIDTH,LENGTH - 1};

typedef struct plant_t {
	uint8_t x;
	uint8_t y;
} plant_t;

typedef struct tail_t {
	uint8_t x;
	uint8_t y;
} tail_t;

typedef struct head_t {
	uint8_t id;
	uint8_t x;
	uint8_t y;
	uint8_t direction;
	size_t tsize;
	size_t load;
	struct tail_t *tail;
	struct plant_t *target;
	struct point_t port;
	struct control_buttons controls;
} head_t;

struct head_t *arr_head[DRON_NUMBER];

uint8_t autoMotionDron(struct head_t*);

void setColors(uint8_t id) {
	attroff(COLOR_PAIR(DRON1_PAIR));
	attroff(COLOR_PAIR(DRON2_PAIR));
	attroff(COLOR_PAIR(TARGET_GREEN_PAIR));
	attroff(COLOR_PAIR(TARGET_GOOD_PAIR));
	switch (id) {
		case 1:
			attron(COLOR_PAIR(DRON1_PAIR));
			break;
		case 2:
			attron(COLOR_PAIR(DRON2_PAIR));
			break;
		case 3:
			attron(COLOR_PAIR(TARGET_GREEN_PAIR));
			break;
		case 4:
			attron(COLOR_PAIR(TARGET_GOOD_PAIR));
			break;
		default:
			break;
	}
}

void drawField() {
	size_t i, j;
	setColors(TARGET_GREEN_PAIR);
	for (i = 0; i < WIDTH; ++i) {
		mvprintw(0, i, "%c", HORIZONT_BOARD);
	}

	for (i = 0; i < ROW; i = i + 2) {
		for (j = 0; j < COLUMN; ++j) {
			plants[i][j] = 1;
		}
	}

	for (i = 1; i < LENGTH; ++i) {
		mvprintw(i, 0, "%c", SIDE_BOARD);

		plants[i][0] = 1;
		plants[i][COLUMN - 1] = 1;

		if (!(i & 1)) {
			for (j = 2; j < WIDTH - 2; ++j) {
				mvprintw(i, j, "%c", TARGET);
			}
		}

		if (i != port1.y && i != port2.y) {
			mvprintw(i, WIDTH - 1, "%c", SIDE_BOARD);
		}

	}

	for (i = 0; i < WIDTH; ++i) {
		mvprintw(LENGTH, i, "%c", HORIZONT_BOARD);
	}

}

void initTail(struct tail_t t[], size_t size, uint8_t x, uint8_t y) {
	for (size_t i = 0; i < size; ++i) {
		t[i].x = x + i + 1;
		t[i].y = y;
	}
}

void initTarget(struct head_t *head) {
	plant_t* target = (plant_t*) malloc(sizeof(plant_t));
	target->x = COLUMN - 1;
	target->y = 0;
	head->target = target;
}

int get_rand_range_int(const int min, const int max) {
    return (rand() % (max - min + 1)) + min;
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

void makePause() {
	nodelay(stdscr, FALSE);
	mvprintw(LENGTH + 1, 0, "For continue game press \'p\'");
	int button;
	while ((button = getch()) != PAUSE_PLAY) {}
	mvprintw(LENGTH + 1, 0, "                           ");
	nodelay(stdscr, TRUE);
}

_Bool haveEat(struct head_t *head) {
	return plants[head->y - 1][head->x - 1] == 2;
}

/*
Увеличение хвоста на 1 элемент
*/
_Bool addTail(struct head_t *head) {
    if(head == NULL || head->tsize >= MAX_TAIL_SIZE) {
        mvprintw(LENGTH + 1, 0, "Can't add tail to dron %d", head->id);
        return 0;
    }
    head->tsize++;
    return 1;
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

void printExit() {
	for (size_t i = 0; i < DRON_NUMBER; ++i) {
		mvprintw(LENGTH + 2 + i, 0, "Total harvested by dron %d = %lu",
					arr_head[i]->id, arr_head[i]->tsize - START_TAIL_SIZE
							+ arr_head[i]->load * (MAX_TAIL_SIZE - START_TAIL_SIZE));
	}
}

int main(void) {
	initscr();
	if (has_colors()) {
		start_color();
		init_pair(TARGET_GOOD_PAIR, COLOR_RED, COLOR_BLACK);
		init_pair(DRON1_PAIR, COLOR_YELLOW, COLOR_BLACK);
		init_pair(TARGET_GREEN_PAIR, COLOR_GREEN, COLOR_BLACK);
		init_pair(DRON2_PAIR, COLOR_CYAN, COLOR_BLACK);
	} else {
		endwin();
		exit(1);
	}

	int res = showStartMenu();
	if (res == CODE_KEY_END) {
		endwin();
		return 0;
	}
	clear();

	int key_pressed, semi_auto_mode = 0, hand_mode = 0;

	raw();
	keypad(stdscr, TRUE);   // Включаем режим чтения функциональных клавиш
	noecho();               // Выключаем отображение вводимых символов, нужно для getch()
	curs_set(0);
	nodelay(stdscr, TRUE);  // Отключаем задержку для getch() и используем для задержки  timeout(5000) или sleep(5)

	drawField();

	time_t start_time = time(NULL);
	srand((unsigned) start_time / 2);

	head_t* head = (head_t*) malloc(sizeof(head_t));
	head->port = port1;
	initDron(head, START_TAIL_SIZE, 1);
	arr_head[0] = head;

	head_t *head1 = (head_t*) malloc(sizeof(head_t));
	head1->port = port2;
	initDron(head1, START_TAIL_SIZE, 2);
	arr_head[1] = head1;

	size_t i, cnt = 0, start_step = 3;
	while ((key_pressed = getch()) != STOP_GAME) {
		move(0, 0);
		if (start_step) {
			move_dron(head);
			move_dron(head1);
			start_step--;
			timeout(DELAY);
			continue;
		}
		if (key_pressed == PAUSE_PLAY) {
			makePause();
			continue;
		}

		if (key_pressed == AUTO_MODE) {
			hand_mode = 0;
			semi_auto_mode = 0;
			nodelay(stdscr, TRUE);
			mvprintw(LENGTH + 1, 0, "                         ");
			mvprintw(LENGTH + 1, 0, "AUTO");
		} else if (key_pressed == HAND_MODE) {
			hand_mode = 1;
			semi_auto_mode = 0;
			nodelay(stdscr, FALSE);
			mvprintw(LENGTH + 1, 0, "                         ");
			mvprintw(LENGTH + 1, 0, "HAND");
			continue;
		} else if (key_pressed == SEMI_AUTO_MODE) {
			semi_auto_mode = 1;
			hand_mode = 1;
			nodelay(stdscr, TRUE);
			mvprintw(LENGTH + 1, 0, "                         ");
			mvprintw(LENGTH + 1, 0, "SEMI-AUTO");
		}

		cnt++;

		if (cnt == LAG) {
			insertTarget();
			cnt = 0;
		}

		if (hand_mode && checkDirection(head, key_pressed)) {
			changeDirection(head, key_pressed);
			if (semi_auto_mode) {
				if (!autoMotionDron(head)) {
					mvprintw(LENGTH + 1, 0,	"--Something went wrong with dron %d !!!", head->id);
					break;
				}
			} else if (!move_dron(head)) {
				mvprintw(LENGTH + 1, 0,	"--Something went wrong with dron %d !!!", head->id);
				break;
			}

			if (!postProcessingOfMovement(head)) {
				break;
			}
			checkUnloadDron(head);
		}

		for (i = hand_mode; i < DRON_NUMBER; ++i) {
			if(!autoMotionDron(arr_head[i])) {
				mvprintw(LENGTH + 1, 0,	"--Something went wrong with dron %d !!!", arr_head[i]->id);
				break;
			}

			if (!postProcessingOfMovement(arr_head[i])) {
				break;
			}
			checkUnloadDron(arr_head[i]);
		}

		if (hand_mode && !semi_auto_mode) {
			timeout(0);
			nodelay(stdscr, FALSE);
		} else {
			timeout(DELAY);
		}

	}


	timeout(0);

	printExit();
	refresh();
	nodelay(stdscr, FALSE);
	getch();

	free(head->tail);
	free(head->target);
	free(head);
	free(head1->tail);
	free(head1->target);
	free(head1);
	endwin();
	return 0;
}
