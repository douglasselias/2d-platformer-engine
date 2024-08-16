#include "raylib.h"

#include "std/types.cpp"
#include "std/vector_overload.cpp"
#include "std/log.cpp"

#include "src/screen.cpp"

s32 main(void) {
  init_screen();

  while (!WindowShouldClose()) {
    BeginDrawing();
      ClearBackground(RAYWHITE);
      // DrawText("Congrats! You created your first window!", 190, 200, 20, BLACK);
      draw_text_centered("Congrats! You created your first window!", 190, BLACK);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}