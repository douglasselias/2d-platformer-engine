#include <stdio.h>
#include <time.h>
#include <string.h>

#include "vendor/raylib.h"
#include "vendor/rlgl.h"
#include "vendor/raymath.h"

#include "std/types.cpp"
#include "std/vector_overload.cpp"
#include "std/log.cpp"
#include "std/math.cpp"
#include "std/shapes.cpp"
#include "std/mouse.cpp"

#include "src/screen.cpp"
#include "src/text.cpp"

#define DEV 0
#define EXPORT_FONT  0
#define EXPORT_MUSIC 0
#define EXPORT_IMG   0

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
#endif

enum class EngineState {
  IN_GAME,
  LEVEL_EDITOR,
  TILE_SELECTION,
};

s32 main() {
  puts("-----------\nGame Logs |\n__________|");

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

  f32 TILE_SIZE = 16;
  u32 selected_tile_x = 0;
  u32 selected_tile_y = 0;
  // Vector2 level[level_height][level_width] = {};

  Camera2D camera2D = {};
  camera2D.zoom = 1;

  Vector2 last_pan_position = {};
  Vector2 current_pan_delta = {};

  while (!WindowShouldClose()) {
    f32 dt = GetFrameTime();
    UpdateMusicStream(music);

    u32 level_width  = screen_width  / (TILE_SIZE * 1);
    u32 level_height = screen_height / (TILE_SIZE * 1);

    if(IsKeyPressed(KEY_U)) {
      if(IsMusicStreamPlaying(music)) PauseMusicStream(music);
      else PlayMusicStream(music);
    }

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

    Vector2 mouse_position = GetMousePosition();
    if(IsKeyPressed(KEY_SPACE)) {
      SetMouseCursor(PAN_CURSOR);
      last_pan_position = camera2D.target + mouse_position;
    }

    if(IsKeyReleased(KEY_SPACE)) {
      SetMouseCursor(ARROW_CURSOR);
      last_pan_position = {};
      current_pan_delta = {};
    }

    if(IsKeyDown(KEY_SPACE)) {
      current_pan_delta = last_pan_position - mouse_position;
      camera2D.target = current_pan_delta;
    }

    if(IsKeyPressed(KEY_J)) {
      dictionary_index = dictionary_index == EN ? CN : EN;
    }

    BeginDrawing();
    ClearBackground(GRAY);

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
                selected_tile_x = col;
                selected_tile_y = row;
              }
            }
          }
        } else if(engine_state == EngineState::LEVEL_EDITOR) {
          for(u32 col = 0; col < level_width/tilemap_scale; col++) {
            for(u32 row = 0; row < level_height/tilemap_scale; row++) {
              f32 tile_scale = TILE_SIZE * 1;
              DrawTexturePro(tilemap, 
                {
                  (f32)selected_tile_x * tile_scale,
                  (f32)selected_tile_y * tile_scale,
                  tile_scale, tile_scale
                }, 
                {
                  (f32)col * tile_scale * tilemap_scale + camera2D.target.x,
                  (f32)row * tile_scale * tilemap_scale + camera2D.target.y,
                  tile_scale * tilemap_scale, tile_scale * tilemap_scale
                }, {0,0}, 0, WHITE);
            }
          }
        }

      EndMode2D();

      DrawTexturePro(tilemap, {(f32)selected_tile_x*TILE_SIZE,(f32)selected_tile_y*TILE_SIZE,TILE_SIZE,TILE_SIZE}, {(f32)screen_width-110,10,100,100}, {0,0}, 0, WHITE);

      u32 font_size = 70;
      u8 spacing = 0;

      {
        char* hello_world_text = i18n(dictionary_index, "hello_world");
        Vector2 pos = {screen_center.x - MeasureTextEx(font, hello_world_text, font_size, spacing).x / 2, screen_center.y - font_size};
        DrawTextEx(font, hello_world_text, pos, font_size, spacing, BLACK);
        DrawTextEx(font, hello_world_text, pos + 3, font_size, spacing, WHITE);
      }

      {
        char* main_menu_play_text = i18n(dictionary_index, "main_menu_play");
        Vector2 pos = {screen_center.x - MeasureTextEx(font, main_menu_play_text, font_size, spacing).x / 2, screen_center.y};
        DrawTextEx(font, main_menu_play_text, pos, font_size, spacing, BLACK);
        DrawTextEx(font, main_menu_play_text, pos + 3, font_size, spacing, WHITE);
      }

      {
        char* hello_sailor_text = i18n(dictionary_index, "hello_sailor");
        Vector2 pos = {screen_center.x - MeasureTextEx(font, hello_sailor_text, font_size, spacing).x / 2, screen_center.y + font_size};
        DrawTextEx(font, hello_sailor_text, pos, font_size, spacing, BLACK);
        DrawTextEx(font, hello_sailor_text, pos + 3, font_size, spacing, WHITE);
      }

    EndDrawing();
  }

  CloseWindow();
  return 0;
}