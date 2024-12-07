#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "vendor/raylib.h"
#include "vendor/rlgl.h"
#include "vendor/raymath.h"

#include "std/types.cpp"
#include "std/vector_overload.cpp"
#include "std/log.cpp"
#include "std/math.cpp"
#include "std/shapes.cpp"
#include "std/mouse.cpp"

#define DEV 1
#define EXPORT_FONT  0
#define EXPORT_MUSIC 0
#define EXPORT_IMG   0
#define EXPORT_ICON  0
#define EXPORT_I18N  0

#if DEV == 0
  #if EXPORT_FONT == 0
  #include "bundle/font.cpp"
  #endif

  #if EXPORT_MUSIC == 0
  #include "bundle/music.cpp"
  #endif

  #if EXPORT_IMG == 0
  #include "bundle/img.cpp"
  #endif

  #if EXPORT_ICON == 0
  #include "bundle/icon.cpp"
  #endif

  #if EXPORT_I18N == 0
  #include "bundle/i18n.cpp"
  #endif
#endif

#include "src/screen.cpp"
#include "src/text.cpp"

enum class EngineState {
  IN_GAME,
  LEVEL_EDITOR,
  TILE_SELECTION,
};

u64 hash_key_markov(const char *key, u64 length) {
  u64 hash = 0;
  while(length--) hash = hash * 31 + key[length];
  return hash;
}

s32 main() {
  puts("-----------");
  puts("Game Logs |");
  puts("__________|");

  init_screen();
  init_i18n();

  EngineState engine_state = EngineState::TILE_SELECTION;
  Languages dictionary_index = CN;

  #if DEV == 1
  Font font = load_font();
  #elif EXPORT_FONT == 1
  Font font = load_font();
  if(ExportFontAsCode(font, "../bundle/font.cpp")) { puts("Success! (Font Exported)"); }
  #else
  Font font = LoadFont_Font();
  SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
  #endif
  
  const char* battle_music_path = "../musics/battle.wav";
  #if DEV == 1
  Music music = LoadMusicStream(battle_music_path);
  #elif EXPORT_MUSIC == 1
  s32 *bin_size = (s32*)MemAlloc(sizeof(s32));
  u8* music_bin = LoadFileData(battle_music_path, bin_size);
  s32 *compressed_size = (s32*)MemAlloc(sizeof(s32));
  u8* compressed_music = CompressData(music_bin, *bin_size, compressed_size);
  if(ExportDataAsCode(compressed_music, *compressed_size, "../bundle/music.cpp")) { puts("Success! (Music Exported)"); }
  MemFree(compressed_music);
  UnloadFileData(music_bin);
  #else
  s32 *decompressed_music_size = (s32*)MemAlloc(sizeof(s32));
  u8* decompressed_music = DecompressData(MUSIC_DATA, MUSIC_DATA_SIZE, decompressed_music_size);
  Music music = LoadMusicStreamFromMemory(".wav", decompressed_music, *decompressed_music_size);
  #endif

  const char* tilemap_image_path = "../gfx/monochrome_tilemap_packed.png";
  #if DEV == 1
  Texture2D tilemap = LoadTexture(tilemap_image_path);
  #elif EXPORT_IMG == 1
  Texture2D tilemap = LoadTexture(tilemap_image_path);
  Image img = LoadImage(tilemap_image_path);
  if(ExportImageAsCode(img, "../bundle/img.cpp")) {
    puts("Success! (Img Exported)");
    FILE* bundle_file = fopen("../bundle/img.cpp", "a");
    const char* i = TextToUpper("img");
    fprintf(bundle_file, 
      "\nTexture2D load_texture_%s() {\n"
      "\tImage img = {}; \n"
      "\timg.data    = %s_DATA; \n"
      "\timg.width   = %s_WIDTH; \n"
      "\timg.height  = %s_HEIGHT; \n"
      "\timg.mipmaps = 1; \n"
      "\timg.format  = %s_FORMAT; \n"
      "\treturn LoadTextureFromImage(img);\n}\n\n", i, i, i, i, i);
    puts("Success! (Added img struct)");
  }
  UnloadImage(img);
  #else
  Texture2D tilemap = load_texture_IMG();
  #endif

  f32 tilemap_scale = 2;

  const f32 TILE_SIZE = 16;
  Vector2 selected_tile = {};
  f32 selected_tile_alpha = 1;
  Rectangle selected_tile_rect = {(f32)screen_width - 110, 10, 100, 100};

  const u8 level_tile_scale = 3;
  const u32 level_width  = screen_width  / (TILE_SIZE * level_tile_scale);
  const u32 level_height = screen_height / (TILE_SIZE * level_tile_scale);
  Vector2 level[level_height][level_width] = {};
  for(u32 col = 0; col < level_height; col++) {
    for(u32 row = 0; row < level_width; row++) {
      level[col][row] = {-1, -1};
    }
  }

  Camera2D camera2D = {};
  camera2D.zoom = 1;

  Vector2 last_pan_position = {};
  Vector2 current_pan_delta = {};

  Vector2 player_position = {};

  //// Markov
  const u8 map_height = 12;
  const u8 map_width = 12;

  char input_map[] =
    "111111111111"
    "110001110001"
    "111011111011"
    "110101110101"
    "101101101101"
    "100101100101"
    "110101110101"
    "101001101001"
    "111111111111"
    "100111100111"
    "111101111101"
    "100101100101"
  ;

  const u64 map_area = map_height * map_width;
  #define HASHTABLE_SIZE 4096
  #define POSSIBILITIES_SIZE 50
  char* ngrams[HASHTABLE_SIZE] = {};

  const u8 order = 12;
  for(u32 index = 0; index <= map_area - order; index++) {
    char gram[order] = {};
    memcpy(gram, &input_map[index], order);

    u64 key = hash_key_markov(gram, order) % HASHTABLE_SIZE;
    if(ngrams[key] == NULL) {
      ngrams[key] = (char*)calloc(POSSIBILITIES_SIZE, sizeof(char));
      memset(ngrams, '\0', POSSIBILITIES_SIZE);
      ngrams[key][0] = input_map[index + order];
    } else {
      u64 n_size = strlen(ngrams[key]);
      if(n_size == POSSIBILITIES_SIZE) continue; // full, moving on...
      ngrams[key][n_size-1] = input_map[index + order];
    }
  }

  char current_gram[order] = {};
  memcpy(current_gram, &input_map, order);
  char result[map_area] = {};
  memset(ngrams, '\0', map_area);
  memcpy(result, &current_gram, order);

  for(u32 i = 0; i < map_area; i++) {
    u64 key = hash_key_markov(current_gram, order) % HASHTABLE_SIZE;
    char *possibilities = ngrams[key];
    u64 p_len = strlen(possibilities);
    if(p_len == 0) {
      memcpy(current_gram, &input_map[i-order], order);
      continue;
    };
    u64 random_index = GetRandomValue(0, p_len-1);
    char next = possibilities[random_index];
    u64 len = strlen(result);
    result[len] = next;
    memcpy(current_gram, result + len + 1 - order, order);
  }

  for(u64 index = 0; index < map_area; index += map_width) {
    printf("Result: %.*s\n", map_width, result+index);
  }

  //// End Markov

  while(!WindowShouldClose()) {
    f32 dt = GetFrameTime();
    Vector2 mouse_position = GetMousePosition();
    UpdateMusicStream(music);

    if(IsKeyPressed(KEY_P)) {
      engine_state = EngineState::IN_GAME;
    }

    const char* level_file_path = "../level.txt";
    if(IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
      if(engine_state == EngineState::LEVEL_EDITOR) {
        FILE* file = fopen(level_file_path, "w");

        for(u32 col = 0; col < level_height; col++) {
          for(u32 row = 0; row < level_width; row++) {
            fprintf(file, "%f %f\n", level[col][row].x, level[col][row].y);
          }
        }

        log("Level Saved");
      }
    }

    if(IsKeyPressed(KEY_L)) {
      if(engine_state == EngineState::LEVEL_EDITOR) {
        FILE* file = fopen(level_file_path, "r");

        for(u32 col = 0; col < level_height; col++) {
          for(u32 row = 0; row < level_width; row++) {
            f32 x, y;
            fscanf(file, "%f", &x);
            fscanf(file, "%f", &y);
            level[col][row] = {x, y};
          }
        }
      }
    }

    if(IsKeyPressed(KEY_U)) {
      if(IsMusicStreamPlaying(music)) PauseMusicStream(music);
      else PlayMusicStream(music);
    }

    if(IsKeyPressed(KEY_R)) {
      camera2D.target = {};
      tilemap_scale = 2;
    }

    u8 border = 20;
    Rectangle alpha_rect = {selected_tile_rect.x - border, selected_tile_rect.y - border, selected_tile_rect.width + border * 2, selected_tile_rect.height + border * 2};
    if(CheckCollisionPointRec(mouse_position, alpha_rect)) {
      selected_tile_alpha = Lerp(selected_tile_alpha, 0.1, 0.1);
    } else {
      selected_tile_alpha = Lerp(selected_tile_alpha, 1, 0.1);
    }

    static f32 cooldown_timer = 0;
    if(IsMouseButtonPressed(0)) {
      if(engine_state == EngineState::TILE_SELECTION) {
        engine_state = EngineState::LEVEL_EDITOR;
        cooldown_timer = 0.3;
        camera2D.target = {};
      }
    }

    /// @note: this prevents placing a tile when transitioning between tile selection and level editor
    cooldown_timer -= dt;
    if(cooldown_timer < 0) cooldown_timer = 0;

    if(IsMouseButtonDown(0)) {
      if(engine_state == EngineState::LEVEL_EDITOR && FloatEquals(cooldown_timer, 0)) {
        Vector2 level_position = (mouse_position + camera2D.target) / (TILE_SIZE * level_tile_scale);
        u32 x = (u32)floor(Clamp(level_position.x, 0, level_width));
        u32 y = (u32)floor(Clamp(level_position.y, 0, level_height));
        level[y][x] = selected_tile;
      }
    }

    if(IsMouseButtonPressed(1)) {
      engine_state = EngineState::TILE_SELECTION;
      camera2D.target = current_pan_delta;
    }

    f32 wheel_delta = GetMouseWheelMove();
    if(!FloatEquals(wheel_delta, 0)) {
      tilemap_scale = Lerp(tilemap_scale,tilemap_scale + 2 * (s64)wheel_delta, 0.1);
      tilemap_scale = Clamp(tilemap_scale, 2, 6);
    }

    if(IsKeyPressed(KEY_SPACE)) {
      if(engine_state == EngineState::TILE_SELECTION) {
        SetMouseCursor(PAN_CURSOR);
        last_pan_position = camera2D.target + mouse_position;
      }
    }

    if(IsKeyReleased(KEY_SPACE)) {
      SetMouseCursor(ARROW_CURSOR);
      last_pan_position = {};
    }

    if(IsKeyDown(KEY_SPACE)) {
      if(engine_state == EngineState::TILE_SELECTION) {
        current_pan_delta = last_pan_position - mouse_position;
        camera2D.target = current_pan_delta;
      }
    }

    if(IsKeyPressed(KEY_J)) {
      dictionary_index = dictionary_index == EN ? CN : EN;
    }

    BeginDrawing();
    if(engine_state == EngineState::IN_GAME) ClearBackground(BLACK);
    else ClearBackground(GRAY);

      BeginMode2D(camera2D);
        if(engine_state == EngineState::TILE_SELECTION) {
          DrawTextureEx(tilemap, {}, 0, tilemap_scale, WHITE);

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

                if(engine_state == EngineState::LEVEL_EDITOR) {
                  continue;
                }
                selected_tile = {(f32)col, (f32)row};
              }
            }
          }
        } else if(engine_state == EngineState::LEVEL_EDITOR) {
          for(u32 col = 0; col < level_width; col++) {
            for(u32 row = 0; row < level_height; row++) {
              Vector2 tile_position = level[row][col];
              if(!FloatEquals(tile_position.x, -1) && !FloatEquals(tile_position.y, -1)) {
                DrawTexturePro(tilemap, 
                  {
                    (f32)tile_position.x * TILE_SIZE,
                    (f32)tile_position.y * TILE_SIZE,
                    TILE_SIZE, TILE_SIZE
                  }, 
                  {
                    (f32)col * TILE_SIZE * level_tile_scale + camera2D.target.x,
                    (f32)row * TILE_SIZE * level_tile_scale + camera2D.target.y,
                    TILE_SIZE * level_tile_scale, TILE_SIZE * level_tile_scale
                  }, {0,0}, 0, WHITE);
              }
              DrawLineV({0, (f32)TILE_SIZE * level_tile_scale * row}, {(f32)screen_width * level_tile_scale, (f32)TILE_SIZE * level_tile_scale * row}, WHITE);
            }
            DrawLineV({(f32)TILE_SIZE * level_tile_scale * col, 0}, {(f32)TILE_SIZE * level_tile_scale * col, (f32)screen_height * level_tile_scale}, WHITE);
          }
        } else if(engine_state == EngineState::IN_GAME) {
          for(u32 col = 0; col < level_width; col++) {
            for(u32 row = 0; row < level_height; row++) {
              Vector2 tile_position = level[row][col];
              if(!FloatEquals(tile_position.x, -1) && !FloatEquals(tile_position.y, -1)) {
                DrawTexturePro(tilemap, 
                  {
                    (f32)tile_position.x * TILE_SIZE,
                    (f32)tile_position.y * TILE_SIZE,
                    TILE_SIZE, TILE_SIZE
                  }, 
                  {
                    (f32)col * TILE_SIZE * level_tile_scale + camera2D.target.x,
                    (f32)row * TILE_SIZE * level_tile_scale + camera2D.target.y,
                    TILE_SIZE * level_tile_scale, TILE_SIZE * level_tile_scale
                  }, {0,0}, 0, WHITE);
              }
              // DrawLineV({0, (f32)TILE_SIZE * level_tile_scale * row}, {(f32)screen_width * level_tile_scale, (f32)TILE_SIZE * level_tile_scale * row}, WHITE);
            }
            // DrawLineV({(f32)TILE_SIZE * level_tile_scale * col, 0}, {(f32)TILE_SIZE * level_tile_scale * col, (f32)screen_height * level_tile_scale}, WHITE);
          }
        }

      EndMode2D();

      if(engine_state != EngineState::IN_GAME) {
        DrawTexturePro(tilemap,
          {selected_tile.x * TILE_SIZE,selected_tile.y * TILE_SIZE,TILE_SIZE,TILE_SIZE},
          selected_tile_rect, {0,0}, 0, {255, 255, 255, (u8)(255 * selected_tile_alpha)});
      }

      u32 font_size = 70;
      u8 spacing = 0;

      // DrawRectangleRec(alpha_rect, WHITE);

      // {
      //   char* hello_world_text = i18n(dictionary_index, "hello_world");
      //   Vector2 pos = {screen_center.x - MeasureTextEx(font, hello_world_text, font_size, spacing).x / 2, screen_center.y - font_size};
      //   DrawTextEx(font, hello_world_text, pos, font_size, spacing, BLACK);
      //   DrawTextEx(font, hello_world_text, pos + 3, font_size, spacing, WHITE);
      // }

      // {
      //   char* main_menu_play_text = i18n(dictionary_index, "main_menu_play");
      //   Vector2 pos = {screen_center.x - MeasureTextEx(font, main_menu_play_text, font_size, spacing).x / 2, screen_center.y};
      //   DrawTextEx(font, main_menu_play_text, pos, font_size, spacing, BLACK);
      //   DrawTextEx(font, main_menu_play_text, pos + 3, font_size, spacing, WHITE);
      // }

      // {
      //   char* hello_sailor_text = i18n(dictionary_index, "hello_sailor");
      //   Vector2 pos = {screen_center.x - MeasureTextEx(font, hello_sailor_text, font_size, spacing).x / 2, screen_center.y + font_size};
      //   DrawTextEx(font, hello_sailor_text, pos, font_size, spacing, BLACK);
      //   DrawTextEx(font, hello_sailor_text, pos + 3, font_size, spacing, WHITE);
      // }

    EndDrawing();
  }

  CloseWindow();
  return 0;
}