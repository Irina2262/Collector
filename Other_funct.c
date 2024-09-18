#include "func.h"

int showStartMenu() {
	int ret;
	mvprintw(0, 0, "%s", DESCRIPTION);
	ret = getch();
	return ret;
}

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

int get_rand_range_int(const int min, const int max) {
    return (rand() % (max - min + 1)) + min;
}

void makePause() {
	nodelay(stdscr, FALSE);
	mvprintw(LENGTH + 1, 0, "For continue game press \'p\'");
	int button;
	while ((button = getch()) != PAUSE_PLAY) {}
	mvprintw(LENGTH + 1, 0, "                           ");
	nodelay(stdscr, TRUE);
}

void printExit() {
	for (size_t i = 0; i < DRON_NUMBER; ++i) {
		mvprintw(LENGTH + 2 + i, 0, "Total harvested by dron %d = %lu",
					arr_head[i]->id, arr_head[i]->tsize - START_TAIL_SIZE
							+ arr_head[i]->load * (MAX_TAIL_SIZE - START_TAIL_SIZE));
	}
}
