#include "stdio.h"
#include "stdlib.h"
#include "time.h"

#include "raylib.h"
#include "raymath.h"

#include "std/types.cpp"
#include "std/vector_overload.cpp"
#include "std/log.cpp"
#include "std/math.cpp"
#include "std/shapes.cpp"

#include "src/screen.cpp"

// #include "font.cpp"

typedef long long s64;
Font load_font() {
  s32 count;
  s32* codepoints = LoadCodepoints("qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM!@#$%&().;>:<,[]{}/我是猫", &count);
  Font font = LoadFontEx("../fonts/noto_serif_chinese_regular.ttf", 700, codepoints, count);
  SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
  return font;
}

unsigned char* read_file_binary(const char *filename, s32 *size) {
  FILE *file;
  errno_t err = fopen_s(&file, filename, "rb");
  if(err != 0 || file == NULL) {
    perror("Error opening file");
    return NULL;
  }

  // Get the file size
  if (fseek(file, 0, SEEK_END) != 0) {
    perror("Error seeking to end of file");
    fclose(file);
    return NULL;
  }
  long file_size = ftell(file);
  if (file_size < 0) {
    perror("Error getting file size");
    fclose(file);
    return NULL;
  }

  if (fseek(file, 0, SEEK_SET) != 0) {
    perror("Error seeking to start of file");
    fclose(file);
    return NULL;
  }

  if(file_size < 0) {
    perror("Error getting file size");
    fclose(file);
    return NULL;
  }

  unsigned char *buffer = (unsigned char *)malloc(file_size);

  size_t bytes_read = fread(buffer, 1, file_size, file);
  if(bytes_read != (size_t)file_size) {
    perror("Error reading file");
    free(buffer);
    fclose(file);
    return NULL;
  }

  fclose(file);
  // *size = file_size;
  return buffer;
}


s32 main() {
  init_screen();

  const char* font_generated_file = "../font.cpp";
  Font font = load_font();

  s32 *bin_size;
  unsigned char* music_bin = read_file_binary("../musics/battle.wav", bin_size);
  // unsigned char* music_bin = LoadFileData("../musics/battle.wav", bin_size);

  s32 display = GetCurrentMonitor();
  bool is_fullscreen = false;

  Camera camera = {};
  camera.position = { 0, 0, -10 };
  camera.target = { 0, 0, 0 };
  camera.up = { 0, 1, 0 };
  camera.fovy = 45;
  camera.projection = CAMERA_PERSPECTIVE;

  Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };

  // Music music = LoadMusicStream("../musics/battle.wav");
  Music music = LoadMusicStreamFromMemory(".wav", music_bin, *bin_size);
  PlayMusicStream(music);

  while (!WindowShouldClose()) {
    f32 dt = GetFrameTime();
    UpdateMusicStream(music);
    
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
        // DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
        DrawTriangle3D(cubePosition, {0, 10, cubePosition.z}, {5, 10, cubePosition.z}, GREEN);
      EndMode3D();

      DrawTextEx(font, "我是猫 Congrats! You created your first window!", {10,screen_center.y}, 250, 4, BLACK);
      // draw_text_centered("Congrats! You created your first window!", 190, BLACK);

      // draw_circle({screen_center, 100}, GRAY);
      // draw_circle_sector({screen_center, 50}, 0, 90, 2, BLUE);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}