#include "vendor/raylib.h"
#include "vendor/rlgl.h"
#include "vendor/raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "vendor/raygui.h"

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

#include "src/tailwind_palette.cpp"

s32 main() {
  // puts("-----------");
  // puts("Game Logs |");
  // puts("__________|");

         printf("Selected file\n");
                fflush(stdout);

  init_screen();
  init_i18n();

  GuiLoadStyle("../vendor/style_dark.rgs");

  enum class EngineState {
    IN_GAME,
    LEVEL_EDITOR,
    TILE_SELECTION,
  };

  /// @todo: Should I create a state machine file with procedures that validate the state before changing?
  enum class EngineSubState {
    /// @note: LEVEL_EDITOR SubState
    TILE_PLACEMENT,
    PHYSICS_PLACEMENT,
    /// @note: ...
  };

  EngineState engine_state = EngineState::LEVEL_EDITOR;
  EngineSubState engine_substate = EngineSubState::TILE_PLACEMENT;
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

  const char* tilemap_image_path = "../gfx/monochrome_tilemap_transparent_packed.png";
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

  enum class TileType {
    NOT_ASSIGNED,
    GROUND,
    PLAYER,
    /// @note: Add more here...
  };
  // IF VALUE > NOT ASSIGNED { CHECK COLLISION }

  TileType tilemap_types[20][20] = {
    {},
    {},
    {},
    {},
    {TileType::NOT_ASSIGNED,TileType::NOT_ASSIGNED,TileType::NOT_ASSIGNED,TileType::NOT_ASSIGNED,TileType::NOT_ASSIGNED,TileType::NOT_ASSIGNED,TileType::GROUND,TileType::GROUND,TileType::GROUND,},
    {TileType::NOT_ASSIGNED,TileType::NOT_ASSIGNED,TileType::NOT_ASSIGNED,TileType::NOT_ASSIGNED,TileType::NOT_ASSIGNED,TileType::NOT_ASSIGNED,TileType::GROUND,TileType::GROUND,TileType::GROUND,},
    {TileType::NOT_ASSIGNED,TileType::NOT_ASSIGNED,TileType::NOT_ASSIGNED,TileType::NOT_ASSIGNED,TileType::NOT_ASSIGNED,TileType::NOT_ASSIGNED,TileType::GROUND,TileType::GROUND,TileType::GROUND,},
    {},
    {},
    {},
    {},
    {},
    {TileType::PLAYER,TileType::PLAYER,TileType::PLAYER,TileType::PLAYER,TileType::PLAYER,TileType::PLAYER},
    {TileType::PLAYER,TileType::PLAYER,TileType::PLAYER,TileType::PLAYER,TileType::PLAYER,TileType::PLAYER},
    {TileType::PLAYER,TileType::PLAYER,TileType::PLAYER,TileType::PLAYER,TileType::PLAYER,TileType::PLAYER},
    {TileType::PLAYER,TileType::PLAYER,TileType::PLAYER,TileType::PLAYER,TileType::PLAYER,TileType::PLAYER},
    {},
    {},
    {},
    {},
  };

  enum class EditorTileType {
    DEFAULT_ONE_TILE,
    HORIZONTAL_ONLY,
    VERTICAL_ONLY,
    NINE_PATCH,
  };

  EditorTileType editor_tile_type = EditorTileType::DEFAULT_ONE_TILE;
  Vector2 start_position = {};
  Vector2 end_position = {};
  bool started_dragging = false;

  Vector2 player_position = {};
  Vector2 player_velocity = {};
  f32 move_speed = 200;

  f32 jump_height = 100;
  f32 jump_time_to_peak    = 0.5;
  f32 jump_time_to_descent = 0.4;

  bool showMessageBox = false;
  f32 cooldown_timer = 0;

  struct PhysicsBlock {
    Vector2 top_left;
    Vector2 bottom_right;
  };

  const u32 total_blocks = 50;
  PhysicsBlock blocks[total_blocks] = {};
  for(u32 i = 0; i < total_blocks; i++) {
    blocks[i].top_left = {-1, -1};
    blocks[i].bottom_right = {-1, -1};
  }

  Vector2 physics_block_start_position = {};

  bool is_first_physics_click = true;
  u32 current_physics_block_index = 0;

  while(!WindowShouldClose()) {
    f32 dt = GetFrameTime();
    Vector2 mouse_position = GetMousePosition();
    /// @todo: Maybe create a struct to have all input states.
    bool mouse_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    UpdateMusicStream(music);

    const char* level_file_path = "../level.txt";

    if(IsKeyPressed(KEY_ONE)) {
      engine_state = EngineState::TILE_SELECTION;
      camera2D.target = current_pan_delta;
    }

    if(IsKeyPressed(KEY_TWO)) {
      engine_state = EngineState::LEVEL_EDITOR;
      engine_substate = EngineSubState::TILE_PLACEMENT;
    }

    if(IsKeyPressed(KEY_THREE)) {
      engine_state = EngineState::LEVEL_EDITOR;
      engine_substate = EngineSubState::PHYSICS_PLACEMENT;
    }

    if(IsKeyPressed(KEY_FOUR)) {
      engine_state = EngineState::IN_GAME;
    }

    if(IsKeyPressed(KEY_C)) {
      /// @todo: Cycle trough the available languages.
      dictionary_index = dictionary_index == EN ? CN : EN;
    }

    /// @note: Input(); 
    s32 speed = 400;
    f32 friction = 0.85;
    /// @note: Copied from https://gist.github.com/sjvnnings/5f02d2f2fc417f3804e967daa73cccfd
    /// @todo: Move closer to where it is being used in the update section.
    f32 jump_velocity = -1 * (( 2 * jump_height) / jump_time_to_peak);
    f32 jump_gravity  = -1 * ((-2 * jump_height) / (jump_time_to_peak * jump_time_to_peak));
    f32 fall_gravity  = -1 * ((-2 * jump_height) / (jump_time_to_descent * jump_time_to_descent));

    const char* physics_level_path = "../physics.txt";

    switch(engine_state) {
      case EngineState::IN_GAME: {
        static bool last_was_left = false;
        u32 turn_speed_multiplier = 3;

        if(IsKeyDown(KEY_A)) {
          if(!last_was_left) {
            player_velocity.x *= friction;
            player_velocity.x -= speed * turn_speed_multiplier * dt;
          }
          player_velocity.x -= speed * dt;
          last_was_left = true;
        } else if(IsKeyDown(KEY_D)) {
          if(last_was_left) {
            player_velocity.x *= friction;
            player_velocity.x += speed * turn_speed_multiplier * dt;
          }
          player_velocity.x += speed * dt;
          last_was_left = false;
        } else {
          if(FloatEquals(player_velocity.y, 0))
            player_velocity.x *= friction;
        }

        f32 percent_speed = 0.9;
        player_velocity.x = Clamp(player_velocity.x, percent_speed * -speed, percent_speed * speed);

        if(IsKeyPressed(KEY_SPACE)) {
          // f32 x_grid = x * TILE_SIZE * level_tile_scale + camera2D.target.x + player_position.x;
          // f32 y_grid = y * TILE_SIZE * level_tile_scale + camera2D.target.y + player_position.y;

          // u32 x_grid = player_position.x / (TILE_SIZE * level_tile_scale);
          // u32 y_grid = player_position.y / (TILE_SIZE * level_tile_scale);

          // u32 x = Clamp(x_grid, 0, 19);
          // u32 y = Clamp(y_grid + 1, 0, 19);
          f32 scaled_tile_size = TILE_SIZE * level_tile_scale;
          if(CheckCollisionRecs({player_position.x,player_position.y,scaled_tile_size,scaled_tile_size}, {0,screen_height-scaled_tile_size*2, screen_width, scaled_tile_size*2})) {
            player_velocity.y = jump_velocity;
            /// @todo: super hacky!!!! duplicate code
            player_position.y += player_velocity.y * dt;
          }
        }
        
        if(IsKeyPressed(KEY_F)) {
          if(IsMusicStreamPlaying(music)) PauseMusicStream(music);
          else PlayMusicStream(music);
        }
      } break;

      case EngineState::LEVEL_EDITOR: {
        switch(engine_substate) {
          case EngineSubState::TILE_PLACEMENT: {
            if(IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
              FILE* file = fopen(level_file_path, "w");

              for(u32 col = 0; col < level_height; col++) {
                for(u32 row = 0; row < level_width; row++) {
                  fprintf(file, "%f %f\n", level[col][row].x, level[col][row].y);
                }
              }

              fclose(file);
              // log("Level saved");
            }

            if(mouse_down && FloatEquals(cooldown_timer, 0)) {
            // if(mouse_down) {
              // log("placing tile...");
              Vector2 level_position = (mouse_position + camera2D.target) / (TILE_SIZE * level_tile_scale);
              u32 x = (u32)floor(Clamp(level_position.x, 0, level_width));
              u32 y = (u32)floor(Clamp(level_position.y, 0, level_height));
              level[y][x] = selected_tile;

              if(started_dragging == false) {
                start_position = {(f32)x, (f32)y};
                started_dragging = true;
                // log("Selected Tile", selected_tile);
                // printf("Selected file\n");
                // fflush(stdout);
                if(4 <= selected_tile.y && selected_tile.y <= 6
                && 7 <= selected_tile.x && selected_tile.x <= 9) {
                  editor_tile_type = EditorTileType::NINE_PATCH;
                } else if(3 <= selected_tile.y && selected_tile.y <= 7
                      && 3 <= selected_tile.x && selected_tile.x <= 6) {
                        editor_tile_type = EditorTileType::HORIZONTAL_ONLY;
                        level[y][x] = {3,selected_tile.y};
                      } else {
                          // editor_tile_type = EditorTileType::DEFAULT_ONE_TILE;
                          // level[y][x] = selected_tile;
                      }
              } else {
                /// @note: is dragging, mouse down
                if(!FloatEquals(start_position.x, x) || !FloatEquals(start_position.y, y)) {

                /// @note: hmmmmm, too hardcoded...
                  switch(editor_tile_type) {
                    case EditorTileType::DEFAULT_ONE_TILE: { level[y][x] = selected_tile; break; };
                    case EditorTileType::HORIZONTAL_ONLY: {
                      if(x > start_position.x) {
                        /// @note: right
                        level[(u32)start_position.y][(u32)start_position.x] = {4,(f32)selected_tile.y};
                        for(u32 ii = 0; ii < x - start_position.x; ii++) {
                          level[(u32)start_position.y][(u32)start_position.x+ii+1] = {5,(f32)selected_tile.y};
                        }
                        level[(u32)start_position.y][x] = {6,(f32)selected_tile.y};
                      }
                      if(x < start_position.x) {
                        /// @note: left
                        level[(u32)start_position.y][(u32)start_position.x] = {6,(f32)selected_tile.y};
                        for(u32 ii = 0; ii < start_position.x - x; ii++) {
                          level[(u32)start_position.y][(u32)start_position.x-ii-1] = {5,(f32)selected_tile.y};
                        }
                        level[(u32)start_position.y][x] = {4,(f32)selected_tile.y};
                      }
                      break;
                    };
                    case EditorTileType::VERTICAL_ONLY: { break; };
                    case EditorTileType::NINE_PATCH: { break; };
                  }
                }
                // if(start_position.x < x) {
                  // level[y][x] = {4,3};
                // }
                // end_position = {x,y};
              }
            }

            if(IsKeyPressed(KEY_Q)) {
              FILE* file = fopen(level_file_path, "r");

              for(u32 col = 0; col < level_height; col++) {
                for(u32 row = 0; row < level_width; row++) {
                  f32 x, y;
                  fscanf(file, "%f", &x);
                  fscanf(file, "%f", &y);
                  level[col][row] = {x, y};

                  /// @todo: hack: getting player position
                  if((u32)x == 0 && (u32)y == 12) {
                    player_position = {x, y};
                    // log("Found player");
                  }
                }
              }

              fclose(file);
            }
          } break;

          case EngineSubState::PHYSICS_PLACEMENT: {
            if(IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
              FILE* file = fopen(physics_level_path, "w");

              for(u32 index = 0; index < total_blocks; index++) {
                PhysicsBlock b = blocks[index];
                fprintf(file, "%f %f\n", b.top_left.x, b.top_left.y);
                fprintf(file, "%f %f\n", b.bottom_right.x, b.bottom_right.y);
              }

              fclose(file);
              // log("Physics saved");
            }

            if(IsKeyDown(KEY_Q)) {
              FILE* file = fopen(physics_level_path, "r");
              for(u32 index = 0; index < total_blocks; index++) {
                f32 x, y;
                fscanf(file, "%f", &x);
                fscanf(file, "%f", &y);
                blocks[index].top_left.x = x;
                blocks[index].top_left.y = y;

                fscanf(file, "%f", &x);
                fscanf(file, "%f", &y);
                blocks[index].bottom_right.x = x;
                blocks[index].bottom_right.y = y;
              }
            }

            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
              Vector2 level_position = (mouse_position + camera2D.target) / (TILE_SIZE * level_tile_scale);
              u32 x = (u32)floor(Clamp(level_position.x, 0, level_width));
              u32 y = (u32)floor(Clamp(level_position.y, 0, level_height));
              Vector2 p = {(f32)x, (f32)y};

              if(is_first_physics_click) {
                physics_block_start_position = p;
                is_first_physics_click = false;
              } else {
                for(u32 index = 0; index < total_blocks; index++) {
                  if(FloatEquals(blocks[index].top_left.x, -1)) {
                    blocks[index].top_left = physics_block_start_position;
                    blocks[index].bottom_right = p;
                    break;
                  }
                }
                physics_block_start_position = {-1,-1};
                is_first_physics_click = true;
              }
            }

            if(IsKeyPressed(KEY_X)) {
              for(u32 i = 0; i < total_blocks; i++) {
                PhysicsBlock b = blocks[i];

                /// @todo: Compress this code. Maybe have this rect inside the struct. Or have a function.
                /// This is used for drawing and collision with mouse (for block removal). 
                f32 far_x = b.bottom_right.x - b.top_left.x + 1;
                f32 far_y = b.bottom_right.y - b.top_left.y + 1;
                Rectangle block_rect = {
                  b.top_left.x * TILE_SIZE * level_tile_scale,
                  b.top_left.y * TILE_SIZE * level_tile_scale,
                  far_x * TILE_SIZE * level_tile_scale,
                  far_y * TILE_SIZE * level_tile_scale
                };

                bool collided = CheckCollisionPointRec(mouse_position, block_rect);
                if(collided) {
                  /// @todo: Remove block
                  blocks[i].top_left = {-1, -1};
                  blocks[i].bottom_right = {-1, -1};
                }


          //   b.top_left.x * TILE_SIZE * level_tile_scale,
          //   b.top_left.y * TILE_SIZE * level_tile_scale,
          //   far_x * TILE_SIZE * level_tile_scale,
          //   far_y * TILE_SIZE * level_tile_scale
              }
            }
          } break;
        /// @note: End of engine_substate switch case.
        }
      } break;

      case EngineState::TILE_SELECTION: {
        if(IsKeyPressed(KEY_R)) {
          camera2D.target = {};
          tilemap_scale = 2;
        }

        f32 wheel_delta = GetMouseWheelMove();
        if(!FloatEquals(wheel_delta, 0)) {
          tilemap_scale = Lerp(tilemap_scale,tilemap_scale + 2 * (s64)wheel_delta, 0.1);
          tilemap_scale = Clamp(tilemap_scale, 2, 6);
        }

        if(IsKeyPressed(KEY_SPACE)) {
          SetMouseCursor(PAN_CURSOR);
          last_pan_position = camera2D.target + mouse_position;
        }

        if(IsKeyReleased(KEY_SPACE)) {
          SetMouseCursor(ARROW_CURSOR);
          last_pan_position = {};
        }

        if(IsKeyDown(KEY_SPACE)) {
          current_pan_delta = last_pan_position - mouse_position;
          camera2D.target = current_pan_delta;
        }

        if(IsMouseButtonPressed(0)) {
          engine_state = EngineState::LEVEL_EDITOR;
          engine_substate = EngineSubState::TILE_PLACEMENT;
          cooldown_timer = 0.3;
          camera2D.target = {};
        }

      } break;
    }

    u8 border = 20;
    Rectangle alpha_rect = {selected_tile_rect.x - border, selected_tile_rect.y - border, selected_tile_rect.width + border * 2, selected_tile_rect.height + border * 2};
    f32 opacity = 1;
    if(CheckCollisionPointRec(mouse_position, alpha_rect)) {
      opacity = 0.1;
    }
    selected_tile_alpha = Lerp(selected_tile_alpha, opacity, 0.1);

    /// @note: This prevents placing a tile when transitioning between tile selection and level editor.
    /// I think it should not change directly to level editor after selecting the tile, then I can remove this cooldown.
    cooldown_timer -= dt;
    if(cooldown_timer < 0) cooldown_timer = 0;

    if(IsMouseButtonReleased(0)) {
      if(engine_state == EngineState::LEVEL_EDITOR) {
        if(started_dragging && (editor_tile_type == EditorTileType::HORIZONTAL_ONLY)) {
          Vector2 level_position = (mouse_position + camera2D.target) / (TILE_SIZE * level_tile_scale);
          u32 x = (u32)floor(Clamp(level_position.x, 0, level_width));
          u32 y = (u32)floor(Clamp(level_position.y, 0, level_height));

          end_position = {(f32)x, (f32)y};
          /// @todo: This is hardcoded. Should have a dynamic list of group tiles that can behave like a 9 patch or 3 patch.
          if(x < start_position.x) {
            level[y][x] = {4,(f32)selected_tile.y};
          }
          if(x > start_position.x) {
            level[y][x] = {6,(f32)selected_tile.y};
          }
          // if(y < start_position.y) {
          //   level[y][x] = {6,3};
          // }
          // if(y > start_position.y) {
          //   level[y][x] = {6,3};
          // }
          started_dragging = false;
        }
      }
    }

    /// @note: Update();
    if(engine_state == EngineState::IN_GAME) {
      f32 scaled_tile_size = TILE_SIZE * level_tile_scale;
      Rectangle ground = {0,screen_height-scaled_tile_size*2, screen_width, scaled_tile_size*2};
      bool hit_ground = CheckCollisionRecs({player_position.x,player_position.y,scaled_tile_size,scaled_tile_size}, ground);

      if(hit_ground) {
        player_velocity.y = 0;
        player_position.y = ground.y - scaled_tile_size + 1;
      } else {
        f32 g = player_velocity.y < 0 ? jump_gravity : fall_gravity;
        player_velocity.y += g * dt;
      }

      player_position.y += player_velocity.y * dt;
      player_position.x += player_velocity.x * dt;
    }

    /// @note: Draw();
    BeginDrawing();
    ClearBackground({0x37, 0x41, 0x51});
    BeginMode2D(camera2D);

    switch(engine_state) {
      case EngineState::TILE_SELECTION: {
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
      } break;

      case EngineState::LEVEL_EDITOR: {
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

        for(u32 index = 0; index < total_blocks; index++) {
          PhysicsBlock b = blocks[index];
          f32 far_x = b.bottom_right.x - b.top_left.x + 1;
          f32 far_y = b.bottom_right.y - b.top_left.y + 1;
          u32 thickness = 3;
          // col * TILE_SIZE * level_tile_scale + camera2D.target.x,
          DrawRectangleLinesEx({
            b.top_left.x * TILE_SIZE * level_tile_scale,
            b.top_left.y * TILE_SIZE * level_tile_scale,
            far_x * TILE_SIZE * level_tile_scale,
            far_y * TILE_SIZE * level_tile_scale
          }, thickness, GOLD);
        }
      } break;

      case EngineState::IN_GAME: {
        for(u32 col = 0; col < level_width; col++) {
          for(u32 row = 0; row < level_height; row++) {
            Vector2 tile_position = level[row][col];
            if(!FloatEquals(tile_position.x, -1) && !FloatEquals(tile_position.y, -1)) {
              /// @todo: very hacky thing, beware
              Vector2 target_position = {
                (f32)col * TILE_SIZE * level_tile_scale + camera2D.target.x,
                (f32)row * TILE_SIZE * level_tile_scale + camera2D.target.y
              };
              if((u32)tile_position.x == 0 && (u32)tile_position.y == 12) {
                target_position = player_position;
                u32 font_size = 70;
                u8 spacing = 0;
                // DrawTextEx(font, TextFormat("{%f, %f}", target_position.x, target_position.y), {0,0}, font_size, spacing, MAGENTA);
                // DrawTextEx(font, TextFormat("{%f, %f}", player_velocity.x, player_velocity.y), {0,(f32)font_size}, font_size, spacing, MAGENTA);
              }
              DrawTexturePro(tilemap, 
                {
                  (f32)tile_position.x * TILE_SIZE,
                  (f32)tile_position.y * TILE_SIZE,
                  TILE_SIZE, TILE_SIZE
                }, 
                {
                  target_position.x,
                  target_position.y,
                  TILE_SIZE * level_tile_scale, TILE_SIZE * level_tile_scale
                }, {0,0}, 0, WHITE);


            }
            // DrawLineV({0, (f32)TILE_SIZE * level_tile_scale * row}, {(f32)screen_width * level_tile_scale, (f32)TILE_SIZE * level_tile_scale * row}, WHITE);
          }
          // DrawLineV({(f32)TILE_SIZE * level_tile_scale * col, 0}, {(f32)TILE_SIZE * level_tile_scale * col, (f32)screen_height * level_tile_scale}, WHITE);
        }
      } break;
    }

    EndMode2D();

    if(engine_state != EngineState::IN_GAME) {
      DrawTexturePro(tilemap,
        {selected_tile.x * TILE_SIZE,selected_tile.y * TILE_SIZE, TILE_SIZE, TILE_SIZE},
        selected_tile_rect, {0,0}, 0, {255, 255, 255, (u8)(255 * selected_tile_alpha)});
    }

    // u32 font_size = 70;
    // u8 spacing = 0;

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

    switch(engine_state) {
      case EngineState::TILE_SELECTION: {} break;

      case EngineState::LEVEL_EDITOR: {} break;

      case EngineState::IN_GAME: {
        f32 scaled_tile_size = TILE_SIZE * level_tile_scale;
        Rectangle p = {player_position.x,player_position.y,scaled_tile_size,scaled_tile_size};
        Rectangle ground = {0,screen_height-scaled_tile_size*2, screen_width, scaled_tile_size*2};
        DrawRectangleLinesEx(p, 3, MAGENTA);
        DrawRectangleLinesEx(ground, 3, GOLD);

        Rectangle slider = {10, 10, 200, 20};
        u32 slider_count = 4;
        u32 gap = 10;
        DrawRectangleRec({0, 0, screen_width, (slider.height * slider_count) + (gap * (slider_count + 1))}, BLACK);

        GuiSlider(slider, NULL, TextFormat("jump_height: %.2fs", jump_height), &jump_height, 1, 1000);

        slider.y += slider.height + gap;
        GuiSlider(slider, NULL, TextFormat("jump_time_to_peak: %.2fs", jump_time_to_peak), &jump_time_to_peak, 0, 1);

        slider.y += slider.height + gap;
        GuiSlider(slider, NULL, TextFormat("jump_time_to_descent: %.2fs", jump_time_to_descent), &jump_time_to_descent, 0, 1);

        slider.y += slider.height + gap;
        GuiSlider(slider, NULL, TextFormat("friction: %.2fs", friction), &friction, 0, 2);
      } break;
    }

    EndDrawing();
  }

  CloseWindow();
  return 0;
}