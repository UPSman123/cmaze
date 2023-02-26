#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAZE_WIDTH 239
#define MAZE_HEIGHT 135
#define MAZE_SIZE (MAZE_WIDTH * MAZE_HEIGHT)
#define MAX_ALIVE_VAL ((int)(MAZE_SIZE * 0.5))
// #define MAX_ALIVE_VAL ((int)(MAZE_SIZE * 0.05))
#define RAT_TRAIL_LENGTH 50
#define RAT_SAFETY_DISTANCE (MAX_ALIVE_VAL / 2)

unsigned int maze[MAZE_WIDTH][MAZE_HEIGHT] = {0};

typedef struct Coord {
    int x, y;
} Coord;

typedef struct MazeGen {
    int stack_start;
    int stack_end;
    Coord stack[MAZE_SIZE];
    unsigned int head_val;
} MazeGen;
MazeGen maze_gen;

typedef struct MazeRat {
    Coord trail[RAT_TRAIL_LENGTH];
} MazeRat;
MazeRat maze_rat;

typedef enum PROG_STATE {
    PS_PLAYING,
    PS_PAUSED,
} PROG_STATE;
PROG_STATE prog_state = PS_PLAYING;

int alive_value(int x, int y) {
    return (int)(maze[x][y] - maze_gen.head_val + MAX_ALIVE_VAL);
}

int is_alive(int x, int y) { return alive_value(x, y) > 0; }

void push(int x, int y) {
    Coord coord = {x, y};
    maze_gen.stack[maze_gen.stack_end++] = coord;
    if (maze_gen.stack_end == MAX_ALIVE_VAL + 1) {
        maze_gen.stack_end = 0;
    }
    if (maze_gen.stack_end == maze_gen.stack_start) {
        printf("pushing on full stack\n");
        prog_state = PS_PAUSED;
    }
}

Coord pop_head(void) {
    if (maze_gen.stack_end == 0)
        maze_gen.stack_end = MAX_ALIVE_VAL;
    else
        maze_gen.stack_end--;
    return maze_gen.stack[maze_gen.stack_end];
}

void pop_tail(void) {
    if (maze_gen.stack_start == MAX_ALIVE_VAL)
        maze_gen.stack_start = 0;
    else
        maze_gen.stack_start++;
}

Coord peek(void) {
    if (maze_gen.stack_end == 0)
        return maze_gen.stack[MAX_ALIVE_VAL];
    return maze_gen.stack[maze_gen.stack_end - 1];
}

unsigned int stack_size(void) {
    int stack_size = maze_gen.stack_end - maze_gen.stack_start;
    if (stack_size < 0)
        stack_size += MAX_ALIVE_VAL + 1;
    return stack_size;
}

Coord __neighbors[4] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
Coord *rand_neighbors() {
    for (int i = 0; i < 3; i++) {
        int j = rand() % (4 - i) + i;
        Coord temp = __neighbors[i];
        __neighbors[i] = __neighbors[j];
        __neighbors[j] = temp;
    }
    return __neighbors;
}

void iter_maze_gen() {
    if (stack_size() == 0)
        return;
    Coord *neighbors = rand_neighbors();

    int done = 0;
    Coord cur = peek();

    // Remove the last element if the stack is full
    if (stack_size() == MAX_ALIVE_VAL) {
        printf("STACK FULL\n");
        printf("start %u end %u size %u max %u\n", maze_gen.stack_start, maze_gen.stack_end, stack_size(), MAX_ALIVE_VAL);
        prog_state = PS_PAUSED;
    }

    // Iterate over neighbors of the current cell
    for (int i = 0; i < 4; i++) {
        int n1_x = neighbors[i].x + cur.x;
        int n1_y = neighbors[i].y + cur.y;
        if (n1_x < 0 || n1_x >= MAZE_WIDTH ||  //
                n1_y < 0 || n1_y >= MAZE_HEIGHT || //
                (n1_x % 2 && n1_y % 2) ||          //
                is_alive(n1_x, n1_y)) {
            continue;
        }
        int can_move = 1;
        // Iterate over neighbors of neighbor
        for (int j = 0; j < 4; j++) {
            int n2_x = neighbors[j].x + n1_x;
            int n2_y = neighbors[j].y + n1_y;
            if (n2_x >= 0 && n2_x < MAZE_WIDTH &&   //
                    n2_y >= 0 && n2_y < MAZE_HEIGHT &&  //
                    (n2_x != cur.x || n2_y != cur.y) && //
                    is_alive(n2_x, n2_y)) {
                can_move = 0;
                break;
            }
        }
        if (!can_move) {
            continue;
        }

        push(n1_x, n1_y);
        done = 1;
        break;
    }
    // The current cell is saturated. pop and try again.
    if (!done)
        pop_head();

    Coord head = peek();
    maze[head.x][head.y] = ++maze_gen.head_val;

    // Check the oldest cell in the main path to see if it should be popped.
    Coord last = maze_gen.stack[maze_gen.stack_start];
    while (!is_alive(last.x, last.y)) {
        pop_tail();
        last = maze_gen.stack[maze_gen.stack_start];
    }
}

void iter_maze_rat() {
    Coord head = maze_rat.trail[RAT_TRAIL_LENGTH - 1];
    Coord prev = maze_rat.trail[RAT_TRAIL_LENGTH - 2];
    Coord *neighbors = rand_neighbors();
    Coord best = head;
    int best_val = alive_value(best.x, best.y);
    for (int i = 0; i < 4; i++) {
        int n_x = head.x + neighbors[i].x;
        int n_y = head.y + neighbors[i].y;
        if (n_x < 0 || n_x >= MAZE_WIDTH || n_y < 0 || n_y >= MAZE_HEIGHT)
            continue;
        int val = alive_value(n_x, n_y);
        if (val > RAT_SAFETY_DISTANCE && (n_x != prev.x || n_y != prev.y)) {
            best.x = n_x;
            best.y = n_y;
            break;
        }
        if (val > best_val) {
            best.x = n_x;
            best.y = n_y;
            best_val = val;
        }
    }
    if (best_val < 0)
        best = head;

    for (int i = 0; i < RAT_TRAIL_LENGTH - 1; i++)
        maze_rat.trail[i] = maze_rat.trail[i + 1];
    maze_rat.trail[RAT_TRAIL_LENGTH - 1] = best;
}

unsigned int stress_level = 0;
unsigned int last_head_val = MAX_ALIVE_VAL;
unsigned int max_stress_level = 0;

unsigned int measure_stress(void) {
    stress_level += maze_gen.head_val - last_head_val;
    last_head_val = maze_gen.head_val;
    if (stress_level > 0)
        stress_level--;

    if (stress_level > max_stress_level) {
        max_stress_level = stress_level;
        printf("stress: %d\n", stress_level);
    }
    return stress_level;
}

void do_iter(void) {
    iter_maze_gen();
    iter_maze_rat();
    measure_stress();
}

int main(void) {
    int seed = time(NULL);
    // int seed = 1675591889;
    printf("seed: %d\n", seed);
    srand(seed);

    Coord center = {MAZE_WIDTH / 2, MAZE_HEIGHT / 2};
    center.x += center.x % 2;
    center.y += center.y % 2;

    maze_gen.stack_start = 0;
    maze_gen.stack_end = 0;
    maze_gen.head_val = MAX_ALIVE_VAL;
    push(center.x, center.y);
    maze[center.x][center.y] = maze_gen.head_val;
    printf("start pos %d %d\n", maze_gen.stack[0].x, maze_gen.stack[0].y);

    for (int i = 0; i < RAT_TRAIL_LENGTH; i++)
        maze_rat.trail[i] = center;

    InitWindow(MAZE_WIDTH * 8 + 8, MAZE_HEIGHT * 8, "Maze");
    HideCursor();
    SetTargetFPS(30);
    int speed = 1;
    while (!WindowShouldClose() && !IsKeyPressed(KEY_Q)) {
        if (IsKeyPressed(KEY_P)) {
            if (prog_state == PS_PLAYING)
                prog_state = PS_PAUSED;
            else
                prog_state = PS_PLAYING;
        }

        if (IsKeyPressed(KEY_UP)) {
            speed *= 1.5;
            if (speed == 1)
                speed = 2;
            printf("speed %d\n", speed);
        }
        if (IsKeyPressed(KEY_DOWN)) {
            if (speed > 1)
                speed /= 1.5;
            printf("speed %d\n", speed);
        }
        if (IsKeyPressed(KEY_D)) {
            printf("DEBUG stack_size %u start %u end %u\n", stack_size(), maze_gen.stack_start, maze_gen.stack_end);
        }

        if (prog_state == PS_PLAYING) {
            for (int i = 0; i < speed; i++) {
                do_iter();
            }
        } else if (IsKeyPressed(KEY_S)) {
            do_iter();
        }

        BeginDrawing();
        ClearBackground(BLACK);

        // Draw the maze
        for (int i = 0; i < MAZE_WIDTH; i++) {
            for (int j = 0; j < MAZE_HEIGHT; j++) {
                int a = alive_value(i, j);
                if (a <= 0)
                    continue;
#define LOW_COLOR 20
                a = a * (255 - LOW_COLOR) / MAX_ALIVE_VAL + LOW_COLOR;
                Color c = {a, a, a, 255};
                DrawRectangle(i * 8 + 4, j * 8, 8, 8, c);
            }
        }

        int size = stack_size();
        if (size >= MAX_ALIVE_VAL) {
            printf("stack full in draw %d / %d\n", size, MAX_ALIVE_VAL);
        }
        for (int i = 0; i < size; i++) {
            int idx = (maze_gen.stack_start + i) % (MAX_ALIVE_VAL + 1);
            Coord coord = maze_gen.stack[idx];
            int a = i * 255 / size;
            Color c = {0, a, 255 - a, 255};
            DrawRectangle(coord.x * 8 + 4, coord.y * 8, 8, 8, c);
        }

        for (int i = 0; i < RAT_TRAIL_LENGTH; i++) {
            Coord coord = maze_rat.trail[i];
            DrawRectangle(coord.x * 8 + 4, coord.y * 8, 8, 8, RED);
        }

        EndDrawing();
    }
}
