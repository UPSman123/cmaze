#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAZE_WIDTH 239
#define MAZE_HEIGHT 135
#define MAZE_SIZE (MAZE_WIDTH * MAZE_HEIGHT)
#define MAX_ALIVE_VAL ((int)(MAZE_SIZE * 0.32))
#define STACK_SIZE (MAX_ALIVE_VAL + 1)
#define RAT_TRAIL_LENGTH 50
#define SPEED 10

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

typedef struct MazeRat {
  Coord trail[RAT_TRAIL_LENGTH];
} MazeRat;
MazeRat maze_rat;

int alive_value(int x, int y) { return (int)(maze[x][y] - maze_gen.iter_cnt); }

int is_alive(int x, int y) { return alive_value(x, y) > 0; }

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
  Coord *neighbors = rand_neighbors();

  int done = 0;
  while (!done && maze_gen.stack_end != maze_gen.stack_start) {
    Coord cur = peek();

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
      if (neighbors[i].x) {
        // It's a horizontal move.
        int diagonal_x = n1_x + neighbors[i].x;
        if (diagonal_x >= 0 && diagonal_x < MAZE_WIDTH) {
          if (is_alive(diagonal_x, n1_y + 1) ||
              is_alive(diagonal_x, n1_y - 1)) {
            can_move = 0;
          }
        }
      } else {
        // It's a vertical move.
        int diagonal_y = n1_y + neighbors[i].y;
        if (diagonal_y >= 0 && diagonal_y < MAZE_HEIGHT) {
          if (is_alive(n1_x + 1, diagonal_y) ||
              is_alive(n1_x - 1, diagonal_y)) {
            can_move = 0;
          }
        }
      }
      if (!can_move) {
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

void iter_maze_rat() {
  Coord head = maze_rat.trail[RAT_TRAIL_LENGTH - 1];
  Coord prev = maze_rat.trail[RAT_TRAIL_LENGTH - 2];
  Coord *neighbors = rand_neighbors();
  Coord next = prev;
  for (int i = 0; i < 4; i++) {
    int n_x = head.x + neighbors[i].x;
    int n_y = head.y + neighbors[i].y;
    if (n_x < 0 || n_x >= MAZE_WIDTH ||     //
        n_y < 0 || n_y >= MAZE_HEIGHT ||    //
        (n_x == prev.x && n_y == prev.y) || //
        alive_value(n_x, n_y) <= RAT_TRAIL_LENGTH)
      continue;
    next.x = n_x;
    next.y = n_y;
  }

  for (int i = 0; i < RAT_TRAIL_LENGTH - 1; i++)
    maze_rat.trail[i] = maze_rat.trail[i + 1];
  maze_rat.trail[RAT_TRAIL_LENGTH - 1] = next;
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
  push(center.x, center.y);
  maze[center.x][center.y] = MAX_ALIVE_VAL;
  printf("start pos %d %d\n", maze_gen.stack[0].x, maze_gen.stack[0].y);
  maze_gen.iter_cnt = 0;

  for (int i = 0; i < RAT_TRAIL_LENGTH; i++)
    maze_rat.trail[i] = center;

  InitWindow(MAZE_WIDTH * 8 + 8, MAZE_HEIGHT * 8, "Maze");
  HideCursor();
  SetTargetFPS(30);
  while (!WindowShouldClose() && !IsKeyPressed(KEY_Q)) {

    for (int i = 0; i < SPEED; i++) {
      iter_maze_gen();
      iter_maze_rat();
    }

    // exit(0);

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

    // if (maze_gen.stack_end == maze_gen.stack_start)
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

    for (int i = 0; i < RAT_TRAIL_LENGTH; i++) {
      Coord coord = maze_rat.trail[i];
      DrawRectangle(coord.x * 8 + 4, coord.y * 8, 8, 8, RED);
    }

    EndDrawing();
  }
}
