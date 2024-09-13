#include <ncurses.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

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

enum {LEFT = 1, RIGHT, UP, DOWN, ROW_SCAN, STOP_GAME = KEY_END
    , AUTO_MODE = 'a', HAND_MODE = 'h', PAUSE_PLAY = 'p', SEMI_AUTO_MODE = 's'};
enum {START_TAIL_SIZE = 0, SPEED_LEVEL = 5, DRON_NUMBER = 2
    , ROW = LENGTH - 1, COLUMN = WIDTH - 2, MAX_TAIL_SIZE = 10
    , MAX_PLANT_SIZE = COLUMN * ROW / 2 };

uint8_t plants[ROW][COLUMN];

char *DESCRIPTION = {"Robot-collector"
        "implements the automatic mode on default\n"
        "However, you can control the yellow drone.\n"
        "You can select mode control press the keys:\n"
        "'a' - full-auto, 's' - semi-auto and 'h' - hand\n"
        "To set the direction of movement, press the keys UP, DOWN, RIGHT and LEFT.\n"
        "To make pause, press the keys 'p'.\n"
        "To exit the game, press END key.\n"};

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

int showStartMenu() {
    clear();  // Очистить экран
    printw("Welcome to the Robot Collector game!\n");
    printw("Press any key to start the game...\n");
    refresh();
    getch();  // Ожидать нажатия клавиши
    return 0;
}

void setColors(uint8_t id);

void drawField();

int get_rand_range_int(const int min, const int max);

void makePause();

void printExit();

void initDron(struct head_t *head, size_t size, uint8_t id);

void insertTarget();

_Bool postProcessingOfMovement(struct head_t *head);

uint8_t autoChangeDirection(struct head_t* head);

uint8_t autoMotionDron(struct head_t*);

uint8_t move_dron(struct head_t* head);

void changeDirection(head_t *head, const int32_t key);

int checkDirection(head_t *head, int32_t key);

void checkUnloadDron(struct head_t *head);

uint8_t isCrush(head_t *head);

uint8_t isCrushExtended(head_t *head, int direction);

int main() {
    // Инициализация библиотеки ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    // Вызов функции стартового меню
    showStartMenu();

    // Окончание работы с ncurses
    endwin();

    return 0;
}
