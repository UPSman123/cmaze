#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAZE_WIDTH 239
#define MAZE_HEIGHT 135
#define MAZE_SIZE (MAZE_WIDTH * MAZE_HEIGHT)
#define MAX_ALIVE_VAL ((int)(MAZE_SIZE * 0.32))
#define STACK_SIZE (MAX_ALIVE_VAL + 1)
#define SPEED 5

unsigned int maze[MAZE_WIDTH][MAZE_HEIGHT];

typedef struct Coord {
  int x, y;
} Coord;

typedef struct MazeGen {
  int stack_start;
  int stack_end;
  Coord stack[MAZE_SIZE];
  unsigned int iter_cnt;
} MazeGen;
MazeGen maze_gen;

int alive_value(int x, int y) {
  return (int)(maze[x][y] - maze_gen.iter_cnt);
}

int is_alive(int x, int y) {
  return alive_value(x, y) > 0;
}

void push(int x, int y) {
  Coord coord = {x, y};
  maze_gen.stack[maze_gen.stack_end++] = coord;
  if (maze_gen.stack_end == STACK_SIZE)
    maze_gen.stack_end = 0;
  if (maze_gen.stack_end == maze_gen.stack_start)
    maze_gen.stack_start++;
  if (maze_gen.stack_start == STACK_SIZE)
    maze_gen.stack_start = 0;
}

void pop(void) {
  if (maze_gen.stack_end == 0)
    maze_gen.stack_end = STACK_SIZE - 1;
  else
    maze_gen.stack_end--;
}

Coord peek(void) {
  if (maze_gen.stack_end == 0)
    return maze_gen.stack[STACK_SIZE - 1];
  return maze_gen.stack[maze_gen.stack_end - 1];
}

void iter_maze_gen() {
  Coord neighbors[4];
  neighbors[0].x = -1;
  neighbors[0].y = 0;
  neighbors[1].x = 1;
  neighbors[1].y = 0;
  neighbors[2].x = 0;
  neighbors[2].y = -1;
  neighbors[3].x = 0;
  neighbors[3].y = 1;

  // Shuffle neighbors
  for (int i = 0; i < 3; i++) {
    int j = rand() % (4 - i) + i;
    Coord temp = neighbors[i];
    neighbors[i] = neighbors[j];
    neighbors[j] = temp;
  }

  int done = 0;
  while (!done && maze_gen.stack_end != maze_gen.stack_start) {
    Coord cur = peek();

    for (int i = 0; i < 4; i++) {
      int n1_x = neighbors[i].x + cur.x;
      int n1_y = neighbors[i].y + cur.y;
      if (n1_x < 0 || n1_x >= MAZE_WIDTH || //
          n1_y < 0 || n1_y >= MAZE_HEIGHT || //
          (n1_x % 2 && n1_y % 2) || //
          is_alive(n1_x, n1_y)) {
        continue;
      }
      int can_turn = 1;
      for (int j = 0; j < 4; j++) {
        int n2_x = neighbors[j].x + n1_x;
        int n2_y = neighbors[j].y + n1_y;
        if (n2_x >= 0 && n2_x < MAZE_WIDTH && //
            n2_y >= 0 && n2_y < MAZE_HEIGHT && //
            is_alive(n2_x, n2_y) && (n2_x != cur.x || n2_y != cur.y)) {
          can_turn = 0;
        }
      }
      if (!can_turn) {
        continue;
      }
      maze[n1_x][n1_y] = maze_gen.iter_cnt + MAX_ALIVE_VAL;
      push(n1_x, n1_y);
      done = 1;
      break;
    }
    if (!done)
      pop();
  }

  // Remove dead cells in stack.
  int stack_size = maze_gen.stack_end - maze_gen.stack_start;
  if (stack_size < 0)
    stack_size += STACK_SIZE;
  else if (stack_size >= STACK_SIZE)
    stack_size -= STACK_SIZE;
  for (int i = 0; i < stack_size; i++) {
    int idx = i + maze_gen.stack_start;
    if (idx > STACK_SIZE)
      idx -= STACK_SIZE;
    Coord coord = maze_gen.stack[idx];
    if (is_alive(coord.x, coord.y))
      break;
    maze_gen.stack_start++;
  }

  maze_gen.iter_cnt++;
}

int main(void) {
  int seed = time(NULL);
  // int seed = 1675591889;
  printf("seed: %d\n", seed);
  srand(seed);
  maze_gen.stack_start = 0;
  maze_gen.stack_end = 0;
  push(MAZE_WIDTH / 2, MAZE_HEIGHT / 2);
  maze[MAZE_WIDTH / 2][MAZE_HEIGHT / 2] = MAX_ALIVE_VAL;
  printf("start pos %d %d\n", maze_gen.stack[0].x, maze_gen.stack[0].y);
  maze_gen.iter_cnt = 0;

  InitWindow(MAZE_WIDTH * 8 + 8, MAZE_HEIGHT * 8, "Maze");
  HideCursor();
  SetTargetFPS(60);
  while (!WindowShouldClose() && !IsKeyPressed(KEY_Q)) {

    for (int i = 0; i < SPEED; i++)
      iter_maze_gen();

    // exit(0);

    BeginDrawing();
    ClearBackground(BLACK);

    // Draw the maze
    for (int i = 0; i < MAZE_WIDTH; i++) {
      for (int j = 0; j < MAZE_HEIGHT; j++) {
        int a = alive_value(i, j);
        if (a <= 0)
          continue;
        a = a * 255 / MAX_ALIVE_VAL;
        Color c = {a, a, a, 255};
        DrawRectangle(i * 8 + 4, j * 8, 8, 8, c);
      }
    }

    //if (maze_gen.stack_end == maze_gen.stack_start)
    int stack_size = maze_gen.stack_end - maze_gen.stack_start;
    if (stack_size > STACK_SIZE)
      stack_size -= STACK_SIZE;
    else if (stack_size < 0)
      stack_size += STACK_SIZE;
    for (int i = 0; i < stack_size; i++) {
      int idx = (maze_gen.stack_start + i) % STACK_SIZE;
      Coord coord = maze_gen.stack[idx];
      int a = i * 255 / (stack_size - 1);
      Color c = {0, a, 255 - a, 255};
      DrawRectangle(coord.x * 8 + 4, coord.y * 8, 8, 8, c);
    }

    EndDrawing();
  }
}
