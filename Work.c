#include <ncurses.h>   // Библиотека для работы с текстовым интерфейсом
#include <stdlib.h>    // Для стандартных функций C, таких как malloc, rand
#include <stdint.h>    // Для использования целочисленных типов данных фиксированной ширины
#include <time.h>      // Для работы с временем (например, для генерации случайных чисел)
#include <stdbool.h>   // Для работы с булевыми типами данных (true/false)
#include <string.h>    // Для работы с функциями строк, такими как memcpy

// Определение размеров поля, символов дрона и мишеней
#define WIDTH 45
#define LENGTH 26
#define TARGET 'o'  // Символ мишени
#define DRON '@'    // Символ дрона
#define TAIL '+'    // Символ хвоста дрона
#define SIDE_BOARD '|'
#define HORIZONT_BOARD '-'

// Определение цветовых пар для дронов и целей
#define DRON1_PAIR 1
#define DRON2_PAIR 2
#define TARGET_GREEN_PAIR 3
#define TARGET_GOOD_PAIR 4

// Коды клавиш для выхода и задержки
#define CODE_KEY_END 27
#define LAG 10
#define DELAY 200

// Определение направлений и других режимов управления
enum {LEFT = 1, RIGHT, UP, DOWN, ROW_SCAN, STOP_GAME = KEY_END, AUTO_MODE = 'a', HAND_MODE = 'h', PAUSE_PLAY = 'p', SEMI_AUTO_MODE = 's'};
enum {START_TAIL_SIZE = 0, SPEED_LEVEL = 5, DRON_NUMBER = 2, ROW = LENGTH - 1, COLUMN = WIDTH - 2, MAX_TAIL_SIZE = 10, MAX_PLANT_SIZE = COLUMN * ROW / 2 };

uint8_t plants[ROW][COLUMN]; // Матрица для хранения состояния клеток поля

// Описание управления
char *DESCRIPTION = {"Robot-collector implements the automatic mode on default\n"
    "However, you can control the yellow drone.\n"
    "You can select mode control press the keys:\n"
    "'a' - full-auto, 's' - semi-auto and 'h' - hand\n"
    "To set the direction of movement, press the keys UP, DOWN, RIGHT and LEFT.\n"
    "To make pause, press the keys 'p'.\n"
    "To exit the game, press END key.\n"};

// Функция для отображения стартового меню
int showStartMenu() {
    int ret;
    mvprintw(0, 0, "%s", DESCRIPTION);  // Вывод текста управления на экран
    ret = getch();  // Ожидание ввода от пользователя
    return ret;  // Возвращаем код нажатой клавиши
}

// Структура для хранения кнопок управления
struct control_buttons {
    int down;
    int up;
    int right;
    int left;
} control_buttons;

struct control_buttons default_controls = {KEY_DOWN, KEY_UP, KEY_RIGHT, KEY_LEFT};  // Определение стандартных кнопок управления

// Структура для хранения точки (координат)
typedef struct point_t {
    uint8_t x;
    uint8_t y;
} point_t;

// Стартовые координаты точек
struct point_t point = {0, 0};
struct point_t port1 = {WIDTH, 1};
struct point_t port2 = {WIDTH, LENGTH - 1};

// Структура для хранения растения (мишени)
typedef struct plant_t {
    uint8_t x;
    uint8_t y;
} plant_t;

// Структура для хранения хвоста дрона
typedef struct tail_t {
    uint8_t x;
    uint8_t y;
} tail_t;

// Структура для хранения данных о дроне (голова)
typedef struct head_t {
    uint8_t id;
    uint8_t x;
    uint8_t y;
    uint8_t direction;  // Текущее направление движения дрона
    size_t tsize;       // Размер хвоста
    size_t load;        // Текущая нагрузка
    struct tail_t *tail;  // Указатель на массив хвоста
    struct plant_t *target;  // Указатель на мишень
    struct point_t port;     // Координаты порта (базы)
    struct control_buttons controls;  // Структура для хранения кнопок управления дрона
} head_t;

struct head_t *arr_head[DRON_NUMBER];  // Массив для хранения нескольких дронов

// Функция для автоматического перемещения дрона
uint8_t autoMotionDron(struct head_t*);

// Установка цветовых пар
void setColors(uint8_t id) {
    // Отключаем все активные цветовые пары
    attroff(COLOR_PAIR(DRON1_PAIR));
    attroff(COLOR_PAIR(DRON2_PAIR));
    attroff(COLOR_PAIR(TARGET_GREEN_PAIR));
    attroff(COLOR_PAIR(TARGET_GOOD_PAIR));

    // Включаем соответствующую цветовую пару в зависимости от ID
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

// Отрисовка поля
void drawField() {
    size_t i, j;
    setColors(TARGET_GREEN_PAIR);  // Устанавливаем цвет для мишеней

    // Отрисовка горизонтальных границ
    for (i = 0; i < WIDTH; ++i) {
        mvprintw(0, i, "%c", HORIZONT_BOARD);  // Верхняя граница
    }

    // Заполнение растений (мишеней)
    for (i = 0; i < ROW; i = i + 2) {
        for (j = 0; j < COLUMN; ++j) {
            plants[i][j] = 1;  // Растение (мишень)
        }
    }

    // Отрисовка вертикальных границ и заполнение игрового поля
    for (i = 1; i < LENGTH; ++i) {
        mvprintw(i, 0, "%c", SIDE_BOARD);  // Левая граница
        plants[i][0] = 1;                  // Заполнение растения у границы
        plants[i][COLUMN - 1] = 1;         // Заполнение правой границы

        if (!(i & 1)) {  // Если строка четная, рисуем мишени
            for (j = 2; j < WIDTH - 2; ++j) {
                mvprintw(i, j, "%c", TARGET);  // Мишени
            }
        }

        // Отрисовка правой границы, исключая порты
        if (i != port1.y && i != port2.y) {
            mvprintw(i, WIDTH - 1, "%c", SIDE_BOARD);  // Правая граница
        }
    }

    // Отрисовка нижней границы
    for (i = 0; i < WIDTH; ++i) {
        mvprintw(LENGTH, i, "%c", HORIZONT_BOARD);  // Нижняя граница
    }
}

// Инициализация хвоста дрона
void initTail(struct tail_t t[], size_t size, uint8_t x, uint8_t y) {
    for (size_t i = 0; i < size; ++i) {
        t[i].x = x + i + 1;  // Задаем начальные координаты для каждого элемента хвоста
        t[i].y = y;
    }
}

// Инициализация цели для дрона
void initTarget(struct head_t *head) {
    plant_t* target = (plant_t*) malloc(sizeof(plant_t));  // Выделяем память для новой цели
    target->x = COLUMN - 1;  // Задаем координаты цели
    target->y = 0;
    head->target = target;  // Привязываем цель к дрону
}

// Генерация случайного числа в диапазоне
int get_rand_range_int(const int min, const int max) {
    return (rand() % (max - min + 1)) + min;  // Возвращаем случайное число в диапазоне [min, max]
}

// Инициализация головы дрона
void initHead(struct head_t *head, uint8_t x, uint8_t y) {
    head->x = x;
    head->y = y;
    head->direction = LEFT;  // Начальное направление движения - влево
}

// Инициализация дрона (головы и хвоста)
void initDron(struct head_t *head, size_t size, uint8_t id) {
    tail_t* tail = (tail_t*) malloc(MAX_TAIL_SIZE * sizeof(tail_t));  // Выделение памяти под хвост
    initTail(tail, size, head->port.x - 2, head->port.y);  // Инициализация хвоста
    head->tail = tail;  // Привязка хвоста к голове
    head->id = id;  // Присваиваем ID дрону
    initTarget(head);  // Инициализация цели для дрона
    initHead(head, head->port.x - 1, head->port.y);  // Инициализация координат головы дрона
    head->tsize = size;  // Задаем размер хвоста
}
// Перемещение дрона вручную на основе ввода клавиш
uint8_t handMotionDron(struct head_t* head, int key) {
    // Проверка нажатой клавиши и изменение направления дрона
    if (key == head->controls.down) {
        head->direction = DOWN;  // Вниз
    } else if (key == head->controls.up) {
        head->direction = UP;    // Вверх
    } else if (key == head->controls.right) {
        head->direction = RIGHT; // Вправо
    } else if (key == head->controls.left) {
        head->direction = LEFT;  // Влево
    } else {
        return 0;  // Если не нажата клавиша управления, ничего не делаем
    }
    return 1;  // Возвращаем 1, если было нажато действие
}

// Перемещение дрона в автоматическом режиме
uint8_t autoMotionDron(struct head_t* head) {
    // Если дрон достигает цели, выбираем новую случайную цель
    if (head->x == head->target->x && head->y == head->target->y) {
        head->target->x = get_rand_range_int(1, COLUMN - 2);  // Случайная координата по X
        head->target->y = get_rand_range_int(1, ROW - 2);     // Случайная координата по Y
    }

    // Определение направления движения для достижения цели
    if (head->x < head->target->x) {
        head->direction = RIGHT;  // Двигаемся вправо
    } else if (head->x > head->target->x) {
        head->direction = LEFT;   // Двигаемся влево
    } else if (head->y < head->target->y) {
        head->direction = DOWN;   // Двигаемся вниз
    } else if (head->y > head->target->y) {
        head->direction = UP;     // Двигаемся вверх
    }

    return 1;  // Возвращаем 1, так как дрон движется автоматически
}

// Функция для обновления позиции хвоста дрона
void updateTail(struct head_t* head) {
    // Сохраняем предыдущие позиции элементов хвоста
    for (int i = head->tsize - 1; i > 0; --i) {
        head->tail[i] = head->tail[i - 1];  // Перемещаем хвост вперед
    }
    // Голова дрона становится началом хвоста
    head->tail[0].x = head->x;
    head->tail[0].y = head->y;
}

// Перемещение дрона в зависимости от текущего направления
void moveDron(struct head_t* head) {
    updateTail(head);  // Обновляем позиции хвоста

    // Меняем координаты головы в зависимости от текущего направления
    switch (head->direction) {
        case UP:
            head->y--;  // Двигаемся вверх
            break;
        case DOWN:
            head->y++;  // Двигаемся вниз
            break;
        case LEFT:
            head->x--;  // Двигаемся влево
            break;
        case RIGHT:
            head->x++;  // Двигаемся вправо
            break;
    }
}

// Функция для отрисовки дрона на экране
void drawDron(struct head_t* head) {
    setColors(head->id);  // Устанавливаем цвет дрона по его ID
    mvprintw(head->y, head->x, "%c", DRON);  // Отрисовываем голову дрона

    // Отрисовываем хвост
    for (int i = 0; i < head->tsize; ++i) {
        mvprintw(head->tail[i].y, head->tail[i].x, "%c", TAIL);  // Хвост дрона
    }
}

// Функция для обработки логики движения дронов
void dronMotionLogic(struct head_t* head, bool auto_mode) {
    // Если включен автоматический режим, вызываем автофункцию
    if (auto_mode) {
        autoMotionDron(head);
    }
    moveDron(head);  // Выполняем перемещение дрона
    drawDron(head);  // Рисуем дрона на экране
}

// Функция для инициализации и запуска игры
void startGame() {
    int ch, key;  // Переменные для хранения нажатых клавиш
    bool auto_mode = true;  // Переменная для хранения состояния автоматического режима

    // Основной цикл игры
    while (1) {
        clear();  // Очищаем экран
        drawField();  // Рисуем игровое поле

        // Обработка каждого дрона
        for (int i = 0; i < DRON_NUMBER; ++i) {
            dronMotionLogic(arr_head[i], auto_mode);  // Логика перемещения дрона
        }

        // Обработка ввода клавиш
        key = getch();
        switch (key) {
            case AUTO_MODE:
                auto_mode = true;  // Включаем автоматический режим
                break;
            case HAND_MODE:
                auto_mode = false;  // Включаем ручной режим
                break;
            case PAUSE_PLAY:
                getch();  // Пауза игры
                break;
            case CODE_KEY_END:
                return;  // Завершаем игру при нажатии клавиши выхода
        }

        refresh();  // Обновляем экран
        napms(DELAY);  // Задержка перед следующей итерацией
    }
}

// Основная функция программы
int main() {
    initscr();  // Инициализация ncurses
    cbreak();   // Отключение буферизации ввода
    noecho();   // Отключение эхо-ввода с клавиатуры
    keypad(stdscr, TRUE);  // Включение работы с функциональными клавишами
    curs_set(0);  // Отключаем отображение курсора
    start_color();  // Включение режима работы с цветами

    // Инициализация цветовых пар
    init_pair(DRON1_PAIR, COLOR_YELLOW, COLOR_BLACK);
    init_pair(DRON2_PAIR, COLOR_BLUE, COLOR_BLACK);
    init_pair(TARGET_GREEN_PAIR, COLOR_GREEN, COLOR_BLACK);
    init_pair(TARGET_GOOD_PAIR, COLOR_RED, COLOR_BLACK);

    // Инициализация генератора случайных чисел
    srand(time(NULL));

    // Создаем два дрона и инициализируем их
    struct head_t dron1, dron2;
    arr_head[0] = &dron1;
    arr_head[1] = &dron2;

    dron1.port.x = WIDTH - 1;  // Порт дрона 1
    dron1.port.y = 1;
    dron2.port.x = WIDTH - 1;  // Порт дрона 2
    dron2.port.y = LENGTH - 2;

    initDron(arr_head[0], START_TAIL_SIZE, 1);  // Инициализация первого дрона
    initDron(arr_head[1], START_TAIL_SIZE, 2);  // Инициализация второго дрона

    showStartMenu();  // Показать стартовое меню
    startGame();  // Запуск игры

    endwin();  // Завершаем работу с ncurses
    return 0;
}

