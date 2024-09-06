#include "stdio.h"
#include "time.h"

#include "raylib.h"
#include "raymath.h"

#include "std/types.cpp"
#include "std/vector_overload.cpp"
#include "std/log.cpp"
#include "std/math.cpp"

#include "src/screen.cpp"

// #include "font.cpp"

Font load_font() {
  s32 count;
  s32* codepoints = LoadCodepoints("qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM!@#$%&().;>:<,[]{}/我是猫", &count);
  Font font = LoadFontEx("../fonts/noto_serif_chinese_regular.ttf", 700, codepoints, count);
  SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
  return font;
}

s32 main() {
  init_screen();

  const char* font_generated_file = "../font.cpp";
  Font font = load_font();

  if(!FileExists(font_generated_file)) {
    // font = LoadFont_Font();
  } 
  // else {
  {
    // font = LoadFont("../fonts/ibm_plex_mono.ttf");
    // font = LoadFontEx("../fonts/ibm_plex_mono.ttf", 1000, codepoints, count);
    // font = LoadFontEx("../fonts/noto_sans_chinese_regular.ttf", 1000, codepoints, count);
    // bool success = ExportFontAsCode(font, font_generated_file);
    // UnloadCodepoints(codepoints);
    // if(!success) return -1;
  }

  s32 display = GetCurrentMonitor();
  bool is_fullscreen = false;
  Vector2 s = {screen_width, screen_height};
  // log("If float equal", s == s);

  log("Hello new log");
  log("Key?", "Value!");
  log("A vector log", s);
  f64 valuef64 = 100;
  log("A f64 value", valuef64);
  log("A f32 value", s.x);

  log("A int value", 1000);
  // s32 a = (s32)s.x;
  // s32 b = (s32)s.y;
  // log("Max", (s32)mod(10, 3));

  Camera camera = {};
  camera.position = { 0, 0, -10 };
  camera.target = { 0, 0, 0 };
  camera.up = { 0, 1, 0 };
  camera.fovy = 45;
  camera.projection = CAMERA_PERSPECTIVE;

  Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };

  while (!WindowShouldClose()) {
    if(IsKeyPressed(KEY_F)) {
      // log("Hello");
      // printf("Hi");
      if (is_fullscreen) {
        // if we are full screen, then go back to the windowed size
        SetWindowPosition(GetMonitorWidth(display)/2-screen_width/2, GetMonitorHeight(display)/2-screen_height/2);
        SetWindowSize(screen_width, screen_height);
        ClearWindowState(FLAG_WINDOW_UNDECORATED);
      } else {
        // if we are not full screen, set the window size to match the monitor we are on
        SetWindowPosition(0,0);
        SetWindowSize(GetMonitorWidth(display), GetMonitorHeight(display));
        SetWindowState(FLAG_WINDOW_UNDECORATED);
      }
      // ToggleFullscreen();
      is_fullscreen = !is_fullscreen;
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);

      BeginMode3D(camera);
        DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
      EndMode3D();

      DrawTextEx(font, "我是猫 Congrats! You created your first window!", {10,screen_center.y}, 250, 4, BLACK);
      // draw_text_centered("Congrats! You created your first window!", 190, BLACK);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}