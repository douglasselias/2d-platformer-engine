#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "string.h"
#include "wchar.h"
#include "locale.h"

#include "raylib.h"
#include "raymath.h"

#include "std/types.cpp"
#include "std/vector_overload.cpp"
#include "std/log.cpp"
#include "std/math.cpp"
#include "std/shapes.cpp"

#include "src/screen.cpp"

#if 0
#include "music.cpp"
#endif

#define HASH_TABLE_SIZE 4096

u64 hash_key(const char *key) {
  u64 hash = 0;

  while(*key) {
    // hash += *key;
    hash = 31 * hash + *key;
    key++;
  }

  return hash % HASH_TABLE_SIZE;
}

char *chinese_chars[4096] = {};

char *english_texts[4096] = {};
char *chinese_texts[4096] = {};

Font load_font() {
  u64 total_chars = 0;
  for(s16 i = 0; i < 4096; i++) {
    if(chinese_chars[i]) {
      total_chars++;
    }
  }
  // s32 count_alpha;
  // s32* alphabet = LoadCodepoints("qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM!@#$%&().;>:<,[]{}/我是猫", &count_alpha);
  char* alphabet = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM?!@#$%&().;>:<,[]{}/'";
  u64 alphabet_length = strlen(alphabet);
  char* all_codepoints = (char*)MemAlloc(alphabet_length * sizeof(char) + sizeof(char) * 3 * total_chars);
  // char* all_codepoints = (char*)malloc(sizeof(char*)*1000);

  strcpy(all_codepoints, alphabet);
   for(s16 i = 0; i < 4096; i++) {
    if(chinese_chars[i]) {
      strcat(all_codepoints, chinese_chars[i]);
    }
  }

  printf("Chinese! %ls", L"我是猫");

  s32 count;
  s32* codepoints = LoadCodepoints(all_codepoints, &count);
  puts(all_codepoints);
  Font font = LoadFontEx("../fonts/noto_serif_chinese_regular.ttf", 100, codepoints, count);
  SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
  return font;
}

void i18n_init() {
  puts("--- Init ---");
  char *csv = LoadFileText("../i18n.csv");
  // puts(csv);
  for(char *line = strtok(csv, "\n"); line != null; line = strtok(null, "\n")) {
    char key[200];
    char en[200];
    char cn[200];
    sscanf(line, "%[^,],%[^,],%[^\n]", key, en, cn);
    printf("%s - %s - %s, %zd\n", key, en, cn, strlen(cn));
    u64 key_hash = hash_key(key);
    english_texts[key_hash] = strdup(en);
    chinese_texts[key_hash] = strdup(cn);

    u8 index = 0;
    char current_char[3] = {};
    u64 len = strlen(cn) / 3;
    while(index < len) {
      strncpy(current_char, cn + 3 * index, 3);
      chinese_chars[hash_key(current_char)] = strdup(current_char);
      index++;
    }

    // wchar_t text_wchar[30];
    // u64 length = 0;
    // mbstowcs_s(&length, text_wchar, current_char, length);
    // setlocale(LC_ALL, "china");
    // wprintf (L"%s \n", text_wchar);
    puts("---");
  }
}

char* i18n(u64 dictionary_index, const char* key) {
  switch(dictionary_index) {
    case 0: return english_texts[hash_key(key)];
    case 1: return chinese_texts[hash_key(key)];
  }
  return "";
}

#include <io.h>
#include <fcntl.h>
#ifndef _O_U16TEXT
  #define _O_U16TEXT 0x20000
#endif

void go_fullscreen(s32 display) {
  // if we are not full screen, set the window size to match the monitor we are on
  SetWindowPosition(0,0);
  SetWindowSize(GetMonitorWidth(display), GetMonitorHeight(display));
  SetWindowState(FLAG_WINDOW_UNDECORATED);
}

u32 selected_tile_x = 0;
u32 selected_tile_y = 0;

s32 main() {
  // setlocale(LC_ALL, "");
    setlocale(LC_ALL, "C.UTF-8"); // this also works
    
     printf("%ls", L"Δικαιοπολις εν αγρω εστιν");

    wchar_t hello_eng[] = L"Hello World!";
    wchar_t *hello_china = L"世界, 你好!";
    wchar_t *hello_japan = L"こんにちは日本!";
    printf("1: %ls\n", hello_eng);
    // printf("2: %s\n", hello_china);
    printf("3: %ls\n", hello_japan);

_setmode(_fileno(stdout), _O_WTEXT);
    wprintf(hello_china);

    wprintf(L"%s\n", L"Δικαιοπολις εν αγρω εστιν");
_setmode(_fileno(stdout), _O_TEXT );

 _setmode(_fileno(stdout), _O_U16TEXT);
    wprintf(L"\x043a\x043e\x0448\x043a\x0430 \x65e5\x672c\x56fd\n");
_setmode(_fileno(stdout), _O_TEXT );

// s32 lpNumberOfCharsWritten;
// WriteConsoleW(
//   stdout,
// hello_china,
//   wcslen(hello_china),
// lpNumberOfCharsWritten,
// NULL
// );


  init_screen();

  i18n_init();
  u64 dictionary_index = 1;

  for(s16 i = 0; i < 4096; i++) {
    if(chinese_chars[i]) {
      printf("%d:%s\n", i, chinese_chars[i]);
    }
  }

  const char* font_generated_file = "../font.cpp";
  Font font = load_font();

  s32 *bin_size = (s32*)MemAlloc(sizeof(s32));
  #if 0
  unsigned char* music_bin = LoadFileData("../musics/battle.wav", bin_size);
  ExportDataAsCode(music_bin, *bin_size, "../music.cpp");
  #endif

  s32 display = GetCurrentMonitor();
  bool is_fullscreen = true;

  // Camera camera = {};
  // camera.position = {0, 0, -10};
  // // camera.target = {0, 0, 0};
  // camera.up = {0, 1, 0};
  // camera.fovy = 45;
  // camera.projection = CAMERA_PERSPECTIVE;

  Camera2D camera2D = {};
  // camera2D.offset = {screen_width/2, screen_height/2};
  // camera2D.offset = {};
  // camera2D.target = {0, 0};
  camera2D.target = {-screen_width/2, -screen_height/2};
  // camera2D.rotation = 0;
  camera2D.zoom = 1;
  // camera2D. = {0, 1, 0};
  // camera2D.fovy = 45;
  // camera2D.projection = CAMERA_PERSPECTIVE;

  Vector3 cube_position = {0, 0, 0};

  #if 1
  Music music = LoadMusicStream("../musics/battle.wav");
  #elif 0
  Music music = LoadMusicStreamFromMemory(".wav", music_bin, *bin_size);
  #else
  Music music = LoadMusicStreamFromMemory(".wav", MUSIC_DATA, MUSIC_DATA_SIZE);
  #endif
  PlayMusicStream(music);

  Texture2D tilemap = LoadTexture("../gfx/monochrome_tilemap_packed.png");
  f32 tilemap_scale = 2;
  s32 mouse_cursor = 0;
  #define PAN_CURSOR 9

  Vector2 last_pan_position = {};
  Vector2 current_pan_delta = {};

  #define TILE_SIZE 16

  enum class Mode {
    EDITOR,
    ASSET,
  };

  go_fullscreen(display);

  while (!WindowShouldClose()) {
    f32 dt = GetFrameTime();
    // UpdateMusicStream(music);

    f32 wheel_delta = GetMouseWheelMove();
    if(!FloatEquals(wheel_delta, 0)) {
      // wheel_delta = Clamp(wheel_delta, -1, 1);
      tilemap_scale = Lerp(tilemap_scale,tilemap_scale + 2 * (s64)wheel_delta, 0.1);
      tilemap_scale = Clamp(tilemap_scale, 2, 6);
      // log("DT: %.2f", wheel_delta);
    }

    // if()

    // if(IsKeyPressed(KEY_D))
    //   SetMouseCursor(++mouse_cursor);
    // if(IsKeyPressed(KEY_A))
    //   SetMouseCursor(--mouse_cursor);

    // log("Mouse: %d", mouse_cursor);

    Vector2 mouse_position = GetMousePosition();
    if(IsKeyPressed(KEY_SPACE)) {
      // dictionary_index = dictionary_index == 0 ? 1 : 0;
      SetMouseCursor(PAN_CURSOR);
      last_pan_position = camera2D.target + mouse_position;
      // log("Last pan pos", last_pan_position);
    }

    if(IsKeyReleased(KEY_SPACE)) {
      SetMouseCursor(0);
      last_pan_position = {};
      current_pan_delta = {};
    }

    if(IsKeyDown(KEY_SPACE)) {
      current_pan_delta = last_pan_position - mouse_position;
      camera2D.target = current_pan_delta;
    }
    
    if(IsKeyPressed(KEY_F)) {
      if (is_fullscreen) {
        // if we are full screen, then go back to the windowed size
        SetWindowPosition(GetMonitorWidth(display)/2-screen_width/2, GetMonitorHeight(display)/2-screen_height/2);
        SetWindowSize(screen_width, screen_height);
        ClearWindowState(FLAG_WINDOW_UNDECORATED);
      } else {
        go_fullscreen(display);
      }
      // ToggleFullscreen();
      is_fullscreen = !is_fullscreen;
    }

    BeginDrawing();
    ClearBackground(GRAY);

      // BeginMode3D(camera);
      //   // DrawCube(cube_position, 2.0f, 2.0f, 2.0f, RED);
      //   // DrawTriangle3D(cube_position, {0, 10, cube_position.z}, {5, 10, cube_position.z}, GREEN);
      // EndMode3D();


      BeginMode2D(camera2D);
        // DrawTexturePro(tilemap, {0,0,(f32)tilemap.width,(f32)tilemap.height}, {0,0,16*70,16*70}, {0,0}, 0, WHITE);
        DrawTextureEx(tilemap, {0,0}, 0, tilemap_scale, WHITE);

        for(u32 col = 0; col <= (u32)(tilemap.width / TILE_SIZE); col++) {
          DrawLineV({(f32)TILE_SIZE * tilemap_scale * col, 0}, {(f32)TILE_SIZE * tilemap_scale * col, (f32)tilemap.height * tilemap_scale}, WHITE);
        }
        for(u32 row = 0; row <= (u32)(tilemap.height / TILE_SIZE); row++) {
          DrawLineV({0, (f32)TILE_SIZE * tilemap_scale * row}, {(f32)tilemap.width * tilemap_scale, (f32)TILE_SIZE * tilemap_scale * row}, WHITE);
        }

        for(u32 col = 0; col < (u32)(tilemap.width / TILE_SIZE); col++) {
          for(u32 row = 0; row < (u32)(tilemap.height / TILE_SIZE); row++) {
            if(CheckCollisionPointRec(mouse_position + camera2D.target, {col * TILE_SIZE * tilemap_scale, row * TILE_SIZE * tilemap_scale, TILE_SIZE * tilemap_scale, TILE_SIZE * tilemap_scale})) {
              DrawRectangleRec({col * TILE_SIZE * tilemap_scale, row * TILE_SIZE * tilemap_scale, TILE_SIZE * tilemap_scale, TILE_SIZE * tilemap_scale}, {0,255,0,(u8)(255*0.5)});
              selected_tile_x = col;
              selected_tile_y = row;
            }
          }
        }

      EndMode2D();

      // DrawTexturePro(tilemap, {camera2D.target.x,camera2D.target.y,TILE_SIZE * tilemap_scale,TILE_SIZE * tilemap_scale}, {0,0,100,100}, {0,0}, 0, WHITE);
      DrawTexturePro(tilemap, {(f32)selected_tile_x*TILE_SIZE,(f32)selected_tile_y*TILE_SIZE,TILE_SIZE,TILE_SIZE}, {0,0,100,100}, {0,0}, 0, WHITE);

      // DrawTextEx(font, i18n(dictionary_index, "question"), {10,screen_center.y}, 70, 4, BLACK);


      // DrawTextEx(font, "我是猫 Congrats! You created your first window!", {10,screen_center.y}, 250, 4, BLACK);
      // DrawTextEx(font, i18n("main_menu_play"), {10,screen_center.y}, 250, 4, BLACK);
      // draw_text_centered("Congrats! You created your first window!", 190, BLACK);

      // draw_circle({screen_center, 100}, GRAY);
      // draw_circle_sector({screen_center, 50}, 0, 90, 2, BLUE);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}