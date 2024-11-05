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

  strcpy(all_codepoints, alphabet);
   for(s16 i = 0; i < 4096; i++) {
    if(chinese_chars[i]) {
      strcat(all_codepoints, chinese_chars[i]);
    }
  }

  // printf("Chinese! %ls", L"我是猫");

  s32 count;
  s32* codepoints = LoadCodepoints(all_codepoints, &count);
  // puts(all_codepoints);
  Font font = LoadFontEx("../fonts/noto_serif_chinese_regular.ttf", 100, codepoints, count);
  SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
  return font;
}

void i18n_init() {
  // puts("--- Init ---");
  char *csv = LoadFileText("../i18n.csv");
  // puts(csv);
  for(char *line = strtok(csv, "\n"); line != null; line = strtok(null, "\n")) {
    char key[200];
    char en[200];
    char cn[200];
    sscanf(line, "%[^,],%[^,],%[^\n]", key, en, cn);
    // printf("%s - %s - %s, %zd\n", key, en, cn, strlen(cn));
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
    // puts("---");
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

enum class EngineState {
  IN_GAME,
  LEVEL_EDITOR,
  TILE_SELECTION,
};

struct Rect {
  f32 min_x, min_y, max_x, max_y;
};

Rect calculate_optimal_size_for_render_size(f32 window_width, f32 window_height, f32 render_width, f32 render_height) {
  Rect r = {};
  u32 optimal_width  = window_height * (render_width  / render_height);
  u32 optimal_height = window_width  * (render_height / render_width);

  if(optimal_width > window_width) {
    /// @note: width contrained
    r.min_x = 0;
    r.max_x = window_width;

    f32 half_empty_space = (window_height - optimal_height) / 2;
    r.min_y = half_empty_space;
    r.max_y = r.min_y + optimal_height;
  } else {
    /// @note: height contrained
    r.min_y = 0;
    r.max_y = window_height;

    f32 half_empty_space = (window_width - optimal_width) / 2;
    r.min_x = half_empty_space;
    r.max_x = r.min_x + optimal_width;
  }

  return r;
}

s32 main() {
  puts("Logs.........");
  EngineState engine_state = EngineState::TILE_SELECTION;

  // setlocale(LC_ALL, "");
  // setlocale(LC_ALL, "C.UTF-8"); // this also works
  
  // printf("%ls", L"Δικαιοπολις εν αγρω εστιν");

  // wchar_t hello_eng[] = L"Hello World!";
  // wchar_t *hello_china = L"世界, 你好!";
  // wchar_t *hello_japan = L"こんにちは日本!";
  // printf("1: %ls\n", hello_eng);
  // // printf("2: %s\n", hello_china);
  // printf("3: %ls\n", hello_japan);

// _setmode(_fileno(stdout), _O_WTEXT);
//     wprintf(hello_china);

//     wprintf(L"%s\n", L"Δικαιοπολις εν αγρω εστιν");
// _setmode(_fileno(stdout), _O_TEXT );

//  _setmode(_fileno(stdout), _O_U16TEXT);
//     wprintf(L"\x043a\x043e\x0448\x043a\x0430 \x65e5\x672c\x56fd\n");
// _setmode(_fileno(stdout), _O_TEXT );

  init_screen();

  i18n_init();
  u64 dictionary_index = 1;

  // for(s16 i = 0; i < 4096; i++) {
  //   if(chinese_chars[i]) {
  //     printf("%d:%s\n", i, chinese_chars[i]);
  //   }
  // }

  const char* font_generated_file = "../font.cpp";
  Font font = load_font();

  s32 *bin_size = (s32*)MemAlloc(sizeof(s32));
  #if 0
  unsigned char* music_bin = LoadFileData("../musics/battle.wav", bin_size);
  ExportDataAsCode(music_bin, *bin_size, "../music.cpp");
  #endif

  s32 display = GetCurrentMonitor();
  bool is_fullscreen = true;

  Camera2D camera2D = {};
  camera2D.zoom = 1;

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

  // #define DEFAULT_CURSOR 0
  #define ARROW_CURSOR 1
  #define BEAM_CURSOR 2
  #define CROSS_CURSOR 3
  #define HAND_CURSOR 4
  #define HORIZONTAL_RESIZE_CURSOR 5
  #define VERTICAL_RESIZE_CURSOR 6
  #define DIAGONAL_1_RESIZE_CURSOR 7
  #define DIAGONAL_2_RESIZE_CURSOR 8
  #define PAN_CURSOR 9
  #define UNAVAILABLE_CURSOR 10

  Vector2 last_pan_position = {};
  Vector2 current_pan_delta = {};

  #define TILE_SIZE 16

  u32 monitor_width  = GetMonitorWidth(display);
  u32 monitor_height = GetMonitorHeight(display);

  u32 current_screen_width  = monitor_width;
  u32 current_screen_height = monitor_height;

  // const u32 level_width  = monitor_width  / TILE_SIZE;
  // const u32 level_height = monitor_height / TILE_SIZE;


  // #define level_init(level_height, level_width)
  // Vector2 level[level_height][level_width] = {};

  // level_init(level_height, level_width);

  go_fullscreen(display);

  RenderTexture2D render_texture = LoadRenderTexture(monitor_width, monitor_height);

  printf("MW %d, MH %d\n", monitor_width, monitor_height);

  while (!WindowShouldClose()) {
    f32 dt = GetFrameTime();
    // UpdateMusicStream(music);

    u32 level_width  = current_screen_width  / (TILE_SIZE * 1);
    u32 level_height = current_screen_height / (TILE_SIZE * 1);

    if(IsMouseButtonPressed(0)) {
      engine_state = EngineState::LEVEL_EDITOR;
    }

    if(IsMouseButtonPressed(1)) {
      engine_state = EngineState::TILE_SELECTION;
    }

    f32 wheel_delta = GetMouseWheelMove();
    if(!FloatEquals(wheel_delta, 0)) {
      tilemap_scale = Lerp(tilemap_scale,tilemap_scale + 2 * (s64)wheel_delta, 0.1);
      tilemap_scale = Clamp(tilemap_scale, 2, 6);
    }

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
        SetWindowPosition(monitor_width/2-half_screen_width, monitor_height/2-half_screen_height);
        SetWindowSize(screen_width, screen_height);
        ClearWindowState(FLAG_WINDOW_UNDECORATED);
        current_screen_width = screen_width;
        current_screen_height = screen_height;
      } else {
        go_fullscreen(display);
        current_screen_width = monitor_width;
        current_screen_height = monitor_height;
      }
      is_fullscreen = !is_fullscreen;
    }

    BeginTextureMode(render_texture);
      ClearBackground(MAGENTA);
      DrawRectangle(half_screen_width - 128, half_screen_height - 128, 256, 256, BLACK);
      DrawRectangle(half_screen_width - 112, half_screen_height - 112, 224, 224, RAYWHITE);
      DrawText("raylib", half_screen_width - 44, half_screen_height + 48, 50, BLACK);
    EndTextureMode();

    // Calculate aspect ratios
    f32 textureAspect = (f32)render_texture.texture.width / (f32)render_texture.texture.height;
    f32 screenAspect = (f32)screen_width / (f32)screen_height;
    // GetScreenWidth();
    // GetScreenHeight();
    // Rect r = calculate_optimal_size_for_render_size(monitor_width, monitor_height, screen_width, screen_height);

    // Determine new width and height based on aspect ratio
    u32 newWidth = 0;
    u32 newHeight = 0;
    if (screenAspect > textureAspect) {
      // Screen is wider than texture
      newWidth = screen_width;
      newHeight = screen_width / textureAspect;
    } else {
      // Texture is wider than screen
      newWidth = screen_height * textureAspect;
      newHeight = screen_height;
    }

    // Calculate the position to center the texture
    u32 posX = (screen_width - newWidth)   / 2;
    u32 posY = (screen_height - newHeight) / 2;

    BeginDrawing();
    ClearBackground(GRAY);

      // BeginMode3D(camera);
        // DrawCube(cube_position, 2.0f, 2.0f, 2.0f, RED);
        // DrawTriangle3D(cube_position, {0, 10, cube_position.z}, {5, 10, cube_position.z}, GREEN);
      // EndMode3D();

      BeginMode2D(camera2D);

        DrawTexturePro(
          render_texture.texture,
          { 0, 0, (f32)render_texture.texture.width, (f32)render_texture.texture.height },
          { (f32)posX, (f32)posY, (f32)newWidth, (f32)newHeight },
          {0,0}, 0, WHITE
        );

        if(engine_state == EngineState::TILE_SELECTION) {
          // DrawTexturePro(tilemap, {0,0,(f32)tilemap.width,(f32)tilemap.height}, {0,0,16*70,16*70}, {0,0}, 0, WHITE);
          Vector2 tilemap_position = {};
          if(is_fullscreen) {
            // tilemap_position.x = monitor_width / 2 - tilemap.width / 2;
            // tilemap_position.y = monitor_height / 2 - tilemap.height / 2;
          }
          DrawTextureEx(tilemap, tilemap_position, 0, tilemap_scale, WHITE);

          for(u32 col = 0; col <= (u32)(tilemap.width / TILE_SIZE); col++) {
            DrawLineV({(f32)TILE_SIZE * tilemap_scale * col, 0}, {(f32)TILE_SIZE * tilemap_scale * col, (f32)tilemap.height * tilemap_scale}, WHITE);
          }
          for(u32 row = 0; row <= (u32)(tilemap.height / TILE_SIZE); row++) {
            DrawLineV({0, (f32)TILE_SIZE * tilemap_scale * row}, {(f32)tilemap.width * tilemap_scale, (f32)TILE_SIZE * tilemap_scale * row}, WHITE);
          }

          for(u32 col = 0; col < (u32)(tilemap.width / TILE_SIZE); col++) {
            for(u32 row = 0; row < (u32)(tilemap.height / TILE_SIZE); row++) {
              if(CheckCollisionPointRec(mouse_position + camera2D.target, {col * TILE_SIZE * tilemap_scale, row * TILE_SIZE * tilemap_scale, TILE_SIZE * tilemap_scale, TILE_SIZE * tilemap_scale})) {
                f32 gap_x = 0;
                f32 gap_y = 0;
                if(is_fullscreen) {
                  // gap_x = monitor_width  / 2 - tilemap.width  / 2;
                  // gap_y = monitor_height / 2 - tilemap.height / 2;
                }
                DrawRectangleRec({col * TILE_SIZE * tilemap_scale + gap_x, row * TILE_SIZE * tilemap_scale + gap_y, TILE_SIZE * tilemap_scale, TILE_SIZE * tilemap_scale}, {0,255,0,(u8)(255*0.5)});

                if(engine_state == EngineState::LEVEL_EDITOR) {
                  continue;
                }
                selected_tile_x = col;
                selected_tile_y = row;
              }
            }
          }
        } else if(engine_state == EngineState::LEVEL_EDITOR) {
          for(u32 col = 0; col < level_width/tilemap_scale; col++) {
            for(u32 row = 0; row < level_height/tilemap_scale; row++) {
              u32 gap_x = 0;
              u32 gap_y = 0;
              if(is_fullscreen) {
                gap_x = monitor_width  / 2 - TILE_SIZE * level_width  / 2;
                gap_y = monitor_height / 2 - TILE_SIZE * level_height / 2 - 4; // @todo: this (- 4) is a hack
                // gap_y *= row;
                static bool printed = false;
                if(!printed) {
                  printed = true;
                  printf("%d, %d, %d\n",(u32)(monitor_height  / 2),(u32)(TILE_SIZE * level_height / 2), gap_y);
                }
              }
              f32 tile_scale = TILE_SIZE * 1;
              DrawTexturePro(tilemap, 
                {
                  (f32)selected_tile_x * tile_scale,
                  (f32)selected_tile_y * tile_scale,
                  tile_scale, tile_scale
                }, 
                {
                  (f32)col * tile_scale * tilemap_scale + camera2D.target.x + gap_x,
                  (f32)row * tile_scale * tilemap_scale + camera2D.target.y + gap_y,
                  tile_scale * tilemap_scale, tile_scale * tilemap_scale
                }, {0,0}, 0, WHITE);
            }
          }
        }


      EndMode2D();

      // DrawTexturePro(tilemap, {camera2D.target.x,camera2D.target.y,TILE_SIZE * tilemap_scale,TILE_SIZE * tilemap_scale}, {0,0,100,100}, {0,0}, 0, WHITE);
      DrawTexturePro(tilemap, {(f32)selected_tile_x*TILE_SIZE,(f32)selected_tile_y*TILE_SIZE,TILE_SIZE,TILE_SIZE}, {(f32)current_screen_width-110,10,100,100}, {0,0}, 0, WHITE);

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