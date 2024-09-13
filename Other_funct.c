#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ncurses.h> 

typedef struct {
    uint8_t x;
    uint8_t y;
} point_t;

typedef struct {
    uint8_t x;
    uint8_t y;
} tail_t;

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t direction;
    uint8_t id;
    tail_t *tail;
    size_t tsize;
    uint8_t load;
    uint8_t controls;
    point_t port;
} head_t;

// Константы и макросы
#define LEFT 0
#define RIGHT 1
#define DRON 'D'
#define TARGET 'T'
#define MAX_TAIL_SIZE 100
#define START_TAIL_SIZE 1
#define MAX_PLANT_SIZE 1000
#define DRON_NUMBER 10
#define TARGET_GOOD_PAIR 1
#define HORIZONT_BOARD '='
#define SIDE_BOARD '|'
#define TARGET_GREEN_PAIR 2
#define DRON1_PAIR 3
#define DRON2_PAIR 4
#define DESCRIPTION "Welcome to the Game!"
#define PAUSE_PLAY 'p'
#define LENGTH 20 
#define WIDTH 20  
#define ROW 20    
#define COLUMN 20 

// Глобальные переменные
uint8_t plants[WIDTH][LENGTH] = {0}; // Инициализация массива тыкв
head_t *arr_head[DRON_NUMBER]; // Массив указателей на дроны
point_t point;
point_t port1 = {0, 0}; 
point_t port2 = {0, 0}; 

// Прототипы функций
void initTail(tail_t t[], size_t size, uint8_t x, uint8_t y);
void initHead(head_t *head, uint8_t x, uint8_t y);
void initDron(head_t *head, size_t size, uint8_t id);
int checkTargetPoint(uint8_t x, uint8_t y);
void getTargetPoint();
void insertTarget();
_Bool haveEat(head_t *head);
_Bool addTail(head_t *head);
_Bool postProcessingOfMovement(head_t *head);
void checkUnloadDron(head_t *head);
uint8_t isCrushDrons(head_t *current);
uint8_t isCrush(head_t *head);
uint8_t isCrushExtended(head_t *head, int direction);
uint8_t get_rand_range_int(const int min, const int max); // Объявление
void setColors(uint8_t id);
void drawField();
int showStartMenu();
void makePause();
void printExit();

// Реализация функций
void initTail(tail_t t[], size_t size, uint8_t x, uint8_t y) {
    for (size_t i = 0; i < size; ++i) {
        t[i].x = x + i + 1;
        t[i].y = y;
    }
}

void initHead(head_t *head, uint8_t x, uint8_t y) {
    head->x = x;
    head->y = y;
    head->direction = LEFT;
}

void initDron(head_t *head, size_t size, uint8_t id) {
    tail_t* tail = (tail_t*) malloc(MAX_TAIL_SIZE * sizeof(tail_t));
    initTail(tail, size, head->port.x - 2, head->port.y);
    initHead(head, head->port.x - 2, head->port.y);
    head->id = id;
    head->tail = tail;
    head->tsize = size;
    head->load = 0;
    head->controls = 0; 

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

_Bool haveEat(head_t *head) {
    return plants[head->y - 1][head->x - 1] == 2;
}

_Bool addTail(head_t *head) {
    if (head == NULL || head->tsize >= MAX_TAIL_SIZE) {
        mvprintw(LENGTH + 1, 0, "Can't add tail to dron %d", head->id);
        return 0;
    }
    head->tsize++;
    return 1;
}

_Bool postProcessingOfMovement(head_t *head) {
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

void checkUnloadDron(head_t *head) {
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

uint8_t isCrushDrons(head_t *current) {
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
        for (size_t i = 0; i < size; i++) {
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
    for (size_t i = 0; i < size; i++) {
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

uint8_t get_rand_range_int(const int min, const int max) {
    return (rand() % (max - min + 1)) + min;
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

int showStartMenu() {
    int ret;
    mvprintw(0, 0, "%s", DESCRIPTION);
    ret = getch();
    return ret;
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

int main() {
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    start_color();
    
    // Инициализация цветов
    init_pair(DRON1_PAIR, COLOR_RED, COLOR_BLACK);
    init_pair(DRON2_PAIR, COLOR_GREEN, COLOR_BLACK);
    init_pair(TARGET_GREEN_PAIR, COLOR_GREEN, COLOR_BLACK);
    init_pair(TARGET_GOOD_PAIR, COLOR_YELLOW, COLOR_BLACK);

    // Показ стартового меню
    int start = showStartMenu();


    endwin(); 
    return 0;
}

