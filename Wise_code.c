#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ncurses.h>

// Константы и макросы
#define ROW 20
#define COLUMN 20
#define ROW_SCAN 5
#define WIDTH 80
#define LENGTH 24
#define DRON_NUMBER 2
#define MAX_TAIL_SIZE 10
#define START_TAIL_SIZE 3
#define TARGET 'T'
#define HORIZONT_BOARD '-'
#define SIDE_BOARD '|'
#define TARGET_GREEN_PAIR 3
#define TARGET_GOOD_PAIR 4
#define DRON1_PAIR 1
#define DRON2_PAIR 2
#define PAUSE_PLAY 'p'
#define LEFT 0
#define RIGHT 1
#define UP 2
#define DOWN 3

// Структуры
typedef struct {
    uint8_t x;
    uint8_t y;
} point_t;

typedef struct {
    uint8_t x;
    uint8_t y;
} plant_t;

typedef struct head_t {
    uint8_t x;
    uint8_t y;
    uint8_t direction;
    uint8_t id;
    uint8_t tsize;
    uint8_t load;
    struct tail_t* tail;
    point_t* target;
    point_t port;
} head_t;

typedef struct tail_t {
    uint8_t x;
    uint8_t y;
} tail_t;

// Глобальные переменные
uint8_t plants[ROW][COLUMN];
point_t point;
head_t* arr_head[DRON_NUMBER];

// Прототипы функций
void drawField();
int get_rand_range_int(const int min, const int max);
void makePause();
void printExit();
void findTargetPoint(uint8_t *target_x, uint8_t *target_y, uint8_t x, uint8_t y);
uint8_t autoChangeDirection(struct head_t* head);
uint8_t autoMotionDron(struct head_t *head);
bool isCrush(struct head_t* head);
bool isCrushExtended(struct head_t* head, int direction);
int move_dron(struct head_t* head);

// Функции
void drawField() {
    size_t i, j;
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

        if (i != 0 && i != LENGTH - 1) {
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
        mvprintw(LENGTH + 2 + i, 0, "Total harvested by dron %d = %u",
                 arr_head[i]->id, arr_head[i]->tsize - START_TAIL_SIZE
                 + arr_head[i]->load * (MAX_TAIL_SIZE - START_TAIL_SIZE));
    }
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
            break;
    }
    return res;
}

uint8_t autoMotionDron(struct head_t *head) {
    uint8_t x = head->x, y = head->y, target_x, target_y;
    bool may_loading = TRUE;
    uint8_t res = 1;
    bool predicat1, predicat2;

    if (head->direction == LEFT || head->direction == RIGHT) {
        target_x = head->port.x;
        target_y = head->port.y;
        may_loading = FALSE;
    } else {
        uint8_t *ptr_x = &target_x;
        uint8_t *ptr_y = &target_y;
        findTargetPoint(ptr_x, ptr_y, x, y);
        may_loading = TRUE;
    }

    if (head->target == NULL) {
        head->target = (point_t*) malloc(sizeof(point_t));
    }
    head->target->x = target_x;
    head->target->y = target_y;

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

// Заглушки для функций, отсутствующих в коде
bool isCrush(struct head_t* head) {
    // Заглушка для проверки столкновений
    return false;
}

bool isCrushExtended(struct head_t* head, int direction) {
    // Заглушка для проверки расширенных столкновений
    return false;
}

int move_dron(struct head_t* head) {
    // Заглушка для перемещения дрона
    return 1;
}

int main() {
    // Пример вызова функций
    return 0;
}
