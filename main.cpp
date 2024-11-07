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

#if 0
#include "music.cpp"
#endif

enum class EngineState {
  IN_GAME,
  LEVEL_EDITOR,
  TILE_SELECTION,
};

enum Languages {
  EN,
  CN,
  PT_BR,
};

s32 main() {
  puts("-----------\nGame Logs |\n__________|");
  EngineState engine_state = EngineState::TILE_SELECTION;

  init_screen();
  i18n_init();
  Languages dictionary_index = CN;

  // const char* font_generated_file = "../font.cpp";
  Font font = load_font();

  s32 *bin_size = (s32*)MemAlloc(sizeof(s32));
  #if 0
  unsigned char* music_bin = LoadFileData("../musics/battle.wav", bin_size);
  ExportDataAsCode(music_bin, *bin_size, "../music.cpp");
  #endif

  Camera2D camera2D = {};
  camera2D.zoom = 1;

  Camera3D camera3D = {};
  camera3D.position = {0, 5, -10};
  camera3D.up = {0, 1, 0};
  camera3D.fovy = 45;
  camera3D.projection = CAMERA_PERSPECTIVE;

  Vector3 cube_position = {};
  f32 cube_rotation = 0;

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

  Vector2 last_pan_position = {};
  Vector2 current_pan_delta = {};

  f32 TILE_SIZE = 16;
  u32 selected_tile_x = 0;
  u32 selected_tile_y = 0;

  // Vector2 level[level_height][level_width] = {};

  RenderTexture2D render_texture = LoadRenderTexture(monitor_width, monitor_height);

  /// @todo:
  // asset embed
  // level editor

  while (!WindowShouldClose()) {
    f32 dt = GetFrameTime();
    // UpdateMusicStream(music);

    u32 level_width  = screen_width  / (TILE_SIZE * 1);
    u32 level_height = screen_height / (TILE_SIZE * 1);

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

    // BeginTextureMode(render_texture);
    //   ClearBackground(MAGENTA);
    //   DrawRectangle(half_screen_width - 128, half_screen_height - 128, 256, 256, BLACK);
    //   DrawRectangle(half_screen_width - 112, half_screen_height - 112, 224, 224, RAYWHITE);
    //   DrawText("raylib", half_screen_width - 44, half_screen_height + 48, 50, BLACK);
    // EndTextureMode();

    BeginDrawing();
    ClearBackground(GRAY);

      BeginMode2D(camera2D);
        DrawTexturePro(
          render_texture.texture,
          { 0, 0, (f32)render_texture.texture.width, (f32)render_texture.texture.height },
          { (f32)0, (f32)0, (f32)1000, (f32)1000 },
          {0,0}, 0, WHITE
        );

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