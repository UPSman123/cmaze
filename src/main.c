#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAZE_SIZE 100

char maze[MAZE_SIZE][MAZE_SIZE];

typedef struct Coord {
  int x, y;
} Coord;

typedef struct MazeGen {
  int stack_size;
  Coord stack[MAZE_SIZE * MAZE_SIZE];
} MazeGen;
MazeGen maze_gen;

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
  while (!done && maze_gen.stack_size) {
    Coord cur = maze_gen.stack[maze_gen.stack_size - 1];

    for (int i = 0; i < 4; i++) {
      int n1_x = neighbors[i].x + cur.x;
      int n1_y = neighbors[i].y + cur.y;
      if (n1_x < 0 || n1_x >= MAZE_SIZE || //
          n1_y < 0 || n1_y >= MAZE_SIZE || //
          maze[n1_x][n1_y]) {
        continue;
      }
      int can_turn = 1;
      for (int j = 0; j < 4; j++) {
        int n2_x = neighbors[j].x + n1_x;
        int n2_y = neighbors[j].y + n1_y;
        if (n2_x >= 0 && n2_x < MAZE_SIZE && //
            n2_y >= 0 && n2_y < MAZE_SIZE && //
            maze[n2_x][n2_y] && (n2_x != cur.x || n2_y != cur.y)) {
          can_turn = 0;
        }
      }
      if (!can_turn) {
        continue;
      }
      maze[n1_x][n1_y] = 1;
      Coord new = {n1_x, n1_y};
      maze_gen.stack[maze_gen.stack_size++] = new;
      done = 1;
      break;
    }
    if (!done)
      maze_gen.stack_size--;
  }
}

int main(void) {
  int seed = time(NULL);
  // int seed = 1675591889;
  printf("seed: %d\n", seed);
  srand(seed);
  maze_gen.stack_size = 1;
  maze_gen.stack[0].x = maze_gen.stack[0].y = MAZE_SIZE / 2;
  maze[MAZE_SIZE / 2][MAZE_SIZE / 2] = 1;
  printf("start pos %d %d\n", maze_gen.stack[0].x, maze_gen.stack[0].y);

  InitWindow(MAZE_SIZE * 8, MAZE_SIZE * 8, "Maze");
  SetTargetFPS(60);
  while (!WindowShouldClose() && !IsKeyPressed(KEY_Q)) {
    iter_maze_gen();

    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Draw the maze
    for (int i = 0; i < MAZE_SIZE; i++) {
      for (int j = 0; j < MAZE_SIZE; j++) {
        Color c = BLACK;
        if (maze[i][j]) {
          c = WHITE;
        }
        DrawRectangle(i * 8, j * 8, 8, 8, c);
      }
    }

    for (int i = 0; i < maze_gen.stack_size; i++) {
      Coord coord = maze_gen.stack[i];
      DrawRectangle(coord.x * 8, coord.y * 8, 8, 8, GREEN);
    }

    EndDrawing();
  }
}
