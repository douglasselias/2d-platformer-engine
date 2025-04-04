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

#include "src/text.cpp"
#include "src/tailwind_palette.cpp"

const char *game_title = "Game";
const u16 screen_width  = 1280;
const u16 screen_height = 720;
const u16 half_screen_width  = screen_width  / 2;
const u16 half_screen_height = screen_height / 2;
const Vector2 screen_center = {half_screen_width, half_screen_height};
u32 monitor_width  = 0;
u32 monitor_height = 0;

void init_screen() {
  // SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  SetTraceLogLevel(LOG_WARNING);
  InitWindow(screen_width, screen_height, game_title);
  // SetWindowState(FLAG_WINDOW_UNDECORATED);

  #if DEV == 1
  Image icon = LoadImage("../icons/game16.png");
  #elif EXPORT_ICON == 1
  Image icon = LoadImage("../icons/game16.png");
  if(ExportImageAsCode(icon, "../bundle/icon.cpp")) {
    puts("Success! (Icon Exported)");
    FILE* bundle_file = fopen("../bundle/icon.cpp", "a");
    const char* i = TextToUpper("icon");
    fprintf(bundle_file, 
      "\nImage icon = { \n"
      "\t%s_DATA, \n"
      "\t%s_WIDTH, \n"
      "\t%s_HEIGHT, \n"
      "\t1, \n"
      "\t%s_FORMAT, \n"
      "};\n", i, i, i, i);
    puts("Success! (Added icon struct)");
  }
  #endif

  SetWindowIcon(icon);
  SetTargetFPS(120);

  InitAudioDevice();
  SetRandomSeed((u32)GetTime());

  s32 display = GetCurrentMonitor();
  monitor_width  = GetMonitorWidth(display);
  monitor_height = GetMonitorHeight(display);
}

void draw_texture(Texture2D texture, Vector2 position, f32 scale = 1, Color tint = WHITE) {
  Rectangle source = {0, 0, (f32)texture.width, (f32)texture.height};
  Rectangle dest = {position.x, position.y, (f32)texture.width * scale, (f32)texture.height * scale};
  Vector2 origin = {0,0};
  u8 rotation = 0;
  DrawTexturePro(texture, source, dest, origin, rotation, tint);
}

const char* level_file_path    = "../level.txt";
const char* physics_level_path = "../physics.txt";
const char* player_file_path   = "../player.txt";

void load_level();
void load_physics_blocks();
void load_player_position();

const u8 level_tile_scale = 3;
const f32 TILE_SIZE = 16;
const u32 level_width  = screen_width  / (TILE_SIZE * level_tile_scale);
const u32 level_height = screen_height / (TILE_SIZE * level_tile_scale);
Vector2 level[level_height][level_width] = {};

struct PhysicsBlock {
  Vector2 top_left;
  Vector2 bottom_right;
};

const u32 total_blocks = 50;
PhysicsBlock blocks[total_blocks] = {};

Vector2 player_position = {};

Camera2D player_camera = {};
Camera2D camera2D = {};

Camera2D current_camera;

s32 main() {
  init_screen();
  init_i18n();

  GuiLoadStyle("../vendor/style_dark.rgs");

  player_camera.zoom = 1;
  camera2D.zoom = 1;

  current_camera = camera2D;

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
    PLAYER_PLACEMENT,
  };

  EngineState engine_state = EngineState::LEVEL_EDITOR;
  EngineSubState engine_substate = EngineSubState::PHYSICS_PLACEMENT;
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

  Vector2 selected_tile = {};
  f32 selected_tile_alpha = 1;
  Rectangle selected_tile_rect = {(f32)screen_width - 110, 10, 100, 100};

  for(u32 col = 0; col < level_height; col++) {
    for(u32 row = 0; row < level_width; row++) {
      level[col][row] = {-1, -1};
    }
  }

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

  Vector2 player_velocity = {};
  f32 move_speed = 200;

  f32 jump_height = 150;
  f32 jump_time_to_peak    = 0.4;
  f32 jump_time_to_descent = 0.3;

  bool showMessageBox = false;
  f32 cooldown_timer = 0;

  for(u32 i = 0; i < total_blocks; i++) {
    blocks[i].top_left     = {-1, -1};
    blocks[i].bottom_right = {-1, -1};
  }

  Vector2 physics_block_start_position = {};

  bool is_first_physics_click = true;
  u32 current_physics_block_index = 0;

  /// @todo: Should it be deceleration instead of friction?
  f32 friction = 0.85;
  f32 air_friction = 1;

  struct Input {
    bool is_pressing_left;
    bool is_pressing_right;
    bool was_jump_pressed;
  };

  Input player_input = {};

  bool is_player_grounded = false;
  bool player_jumped = false;

  f32 coyote_time = 0;
  bool has_coyote_time = false;

  f32 jump_buffer_time = 0;
  bool jump_buffer = false;

  u64 number_of_frames_that_jump_button_is_being_held = 0;
  bool is_variable_height = false;

  u8 player_walk_animation_index = 1;
  u64 frame_counter = 0;

  bool is_player_facing_right = true;

  u8 heart_animation_index = 0;
  bool heart_is_growing = true;

  while(!WindowShouldClose()) {
    f32 dt = GetFrameTime();
    Vector2 mouse_position = GetMousePosition();
    /// @todo: Maybe create a struct to have all input states.
    bool mouse_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    UpdateMusicStream(music);

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
      engine_state = EngineState::LEVEL_EDITOR;
      engine_substate = EngineSubState::PLAYER_PLACEMENT;
    }

    if(IsKeyPressed(KEY_FIVE)) {
      engine_state = EngineState::IN_GAME;
      current_camera = player_camera;
    }

    if(engine_state != EngineState::IN_GAME) {
      current_camera = camera2D;
    }

    if(engine_state == EngineState::IN_GAME) {
      Vector2 offset = {500, 400};
      current_camera.target = player_position - offset;
      // current_camera.offset = screen_center;
    }

    if(IsKeyPressed(KEY_Z)) {
      load_level();
      load_physics_blocks();
      load_player_position();
      engine_state = EngineState::IN_GAME;
    }
    // log("Vel.X", player_velocity.x);

    if(IsKeyPressed(KEY_C)) {
      /// @todo: Cycle trough the available languages.
      dictionary_index = dictionary_index == EN ? CN : EN;
    }

    /// @note: Input(); 
    s32 speed = 400;
    /// @note: Copied from https://gist.github.com/sjvnnings/5f02d2f2fc417f3804e967daa73cccfd
    /// @todo: Move closer to where it is being used in the update section.
    f32 jump_velocity = -1 * (( 2 * jump_height) / jump_time_to_peak);
    f32 jump_gravity  = -1 * ((-2 * jump_height) / (jump_time_to_peak * jump_time_to_peak));
    f32 fall_gravity  = -1 * ((-2 * jump_height) / (jump_time_to_descent * jump_time_to_descent));

    switch(engine_state) {
      case EngineState::IN_GAME: {
        if(IsKeyDown(KEY_A)) {
          player_velocity.x -= speed * dt;
        } else if(IsKeyDown(KEY_D)) {
          player_velocity.x += speed * dt;
        }

        f32 percent_speed = 0.9;
        player_velocity.x = Clamp(player_velocity.x, percent_speed * -speed, percent_speed * speed);

        if(IsKeyPressed(KEY_SPACE)) {
          if(has_coyote_time && coyote_time > 0) {
            /// @todo: Duplicated code!!!
            player_velocity.y = jump_velocity;
            player_position.y += player_velocity.y * dt;
            is_player_grounded = false;
            coyote_time = 0;
            has_coyote_time = false;
            // player_jumped = true;
          } else {
            f32 scaled_tile_size = TILE_SIZE * level_tile_scale;

            for(u32 index = 0; index < total_blocks; index++) {
              /// @todo: Copy-Pasta from editor rendering.
              PhysicsBlock b = blocks[index];
              if(FloatEquals(b.top_left.x, -1)) continue;
              f32 far_x = b.bottom_right.x - b.top_left.x + 1;
              f32 far_y = b.bottom_right.y - b.top_left.y + 1;
              u32 thickness = 3;
              /// @todo: This could be on the struct itself. It only calculates this on creation.
              Rectangle ground = {
                b.top_left.x * TILE_SIZE * level_tile_scale,
                b.top_left.y * TILE_SIZE * level_tile_scale,
                far_x * TILE_SIZE * level_tile_scale,
                far_y * TILE_SIZE * level_tile_scale
              };

              bool hit_ground = CheckCollisionRecs({player_position.x,player_position.y,scaled_tile_size,scaled_tile_size}, ground);

              if(hit_ground) {
                /// @todo: Super hacky!!!! Duplicated code!!!
                player_velocity.y = jump_velocity;
                player_position.y += player_velocity.y * dt;
                is_player_grounded = false;
                player_jumped = true;
              } else {
                /// @note: Jump buffer logic
                f32 bottom_y = player_position.y + scaled_tile_size;
                if(ground.y > bottom_y && (ground.y - bottom_y) < (scaled_tile_size / 3)) {
                  /// @todo: Super hacky!!!! Duplicated code!!!
                  player_velocity.y = jump_velocity;
                  player_position.y += player_velocity.y * dt;
                  is_player_grounded = false;
                  player_jumped = true;
                }
              }
            }
          }
        }

        if(IsKeyDown(KEY_SPACE)) {
          u8 fps = 1 / dt;

          if(number_of_frames_that_jump_button_is_being_held == (fps / 3)) {
            // log("enable variable height");
            is_variable_height = true;
            // log("not grounded", !is_player_grounded);
            // log("player jumped", player_jumped);
            // log("frames held is less than fps", number_of_frames_that_jump_button_is_being_held < (fps * 1));
          }

          if(!is_player_grounded
          // && player_jumped
          && is_variable_height
          && number_of_frames_that_jump_button_is_being_held < (fps * 1)) {
            /// @todo: Super hacky!!!! Duplicated code!!!
            // log("Variable height", fps);
            player_velocity.y += jump_velocity * 1.8 * dt;
            player_position.y += player_velocity.y * dt;
          } else {
            is_variable_height = false;
          }
          number_of_frames_that_jump_button_is_being_held++;
        } else {
          number_of_frames_that_jump_button_is_being_held = 0;
          is_variable_height = false;
          // log("disabling variable height");
        }

        /// @todo: This code should be removed on release.
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
              log("Level saved");
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
              load_level();
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
              log("Physics saved");
            }

            if(IsKeyDown(KEY_Q)) {
              load_physics_blocks();
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
                  blocks[i].top_left = {-1, -1};
                  blocks[i].bottom_right = {-1, -1};
                }
              }
            }
          } break;

          case EngineSubState::PLAYER_PLACEMENT: {
            if(IsMouseButtonDown(0)) {
              // Vector2 p = {floorf(mouse_position.x), floorf(mouse_position.y)};
              Vector2 level_position = (mouse_position + camera2D.target) / (1);
              player_position = level_position;
            }

            if(IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
              FILE* file = fopen(player_file_path, "w");
              fprintf(file, "%f %f\n", player_position.x, player_position.y);
              fclose(file);
              log("Player position saved");
            }

            if(IsKeyPressed(KEY_Q)) {
              load_player_position();
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
      u8 physics_step = 0;
      u8 total_steps = 60;
      physics_loop:

      bool hit_ground = false;
      bool is_moving_left  = player_velocity.x < 0;
      bool is_moving_right = player_velocity.x > 0;
      bool is_moving_up    = player_velocity.y < 0;
      bool is_moving_down  = player_velocity.y > 0;

      // player_velocity.x *= hit_ground && is_moving_down ? friction : air_friction * (dt / total_steps);

      f32 scaled_tile_size = TILE_SIZE * level_tile_scale;
      for(u32 index = 0; index < total_blocks; index++) {
        /// @todo: Copy-Pasta from editor rendering.
        PhysicsBlock b = blocks[index];

        f32 far_x = b.bottom_right.x - b.top_left.x + 1;
        f32 far_y = b.bottom_right.y - b.top_left.y + 1;
        u32 thickness = 3;
        Rectangle ground = {
          b.top_left.x * scaled_tile_size,
          b.top_left.y * scaled_tile_size,
          far_x        * scaled_tile_size,
          far_y        * scaled_tile_size
        };

        /// @todo: Probably, it is better to just have a Rectangle player.
        Rectangle player_rect = {
          player_position.x,
          player_position.y,
          scaled_tile_size,
          scaled_tile_size
        };
        f32 px = player_position.x + player_velocity.x * (dt / total_steps);
        f32 py = player_position.y + player_velocity.y * (dt / total_steps);
        player_rect.y = py;
        hit_ground = CheckCollisionRecs(player_rect, ground);
        if(hit_ground) {
          if(is_moving_down) {
            player_position.y = ground.y - scaled_tile_size;
            is_player_grounded = true;
            has_coyote_time = true;
          }
          else if(is_moving_up) player_position.y = ground.y + ground.height;
          player_velocity.y = 0;
        }

        player_rect.x = px;
        player_rect.y = player_position.y;
        hit_ground = CheckCollisionRecs(player_rect, ground);
        if(hit_ground) {
          // if(is_moving_down) player_position.y = ground.y - scaled_tile_size;
          player_velocity.x = 0;
        }
      }

      {
        // bool is_moving_up = player_velocity.y < 0;
        f32 gravity = is_moving_up ? jump_gravity : fall_gravity;
        player_velocity.y += gravity * (dt / total_steps);
      }

      if(player_velocity.x > 0)
        player_velocity.x -= (is_player_grounded ? (friction * 200) : (air_friction * 100)) * (dt / total_steps);
      else
        player_velocity.x += (is_player_grounded ? (friction * 200) : (air_friction * 100)) * (dt / total_steps);
      // player_velocity.x = Clamp(player_velocity.x, 0, 1000);
      player_position += player_velocity * (dt / total_steps);
      physics_step++;
      if(physics_step < total_steps) goto physics_loop;

      if(has_coyote_time) {
        coyote_time = 0.5;
      }
      // else coyote_time = 0;

      if(coyote_time > 0) coyote_time -= dt;
      else coyote_time = 0;

      // if(jump_buffer_time > 0) jump_buffer_time -= dt;
      // else jump_buffer_time = 0;
    }

    /// @note: Draw();
    BeginDrawing();
    Color bg_color = {0x37, 0x41, 0x51};
    ClearBackground(bg_color);
    BeginMode2D(current_camera);

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
            // u64 remainder = (screen_height / (s32)TILE_SIZE) / 2;
            u64 remainder = 0;
            DrawLineV({0, (f32)TILE_SIZE * level_tile_scale * row + remainder}, {(f32)screen_width * level_tile_scale, (f32)TILE_SIZE * level_tile_scale * row + remainder}, WHITE);
          }
          // u64 remainder = (screen_width / (s32)TILE_SIZE) / 2;
          u64 remainder = 0;
          // log("Remainder", remainder);
          DrawLineV({(f32)TILE_SIZE * level_tile_scale * col + remainder, 0}, {(f32)TILE_SIZE * level_tile_scale * col + remainder, (f32)screen_height * level_tile_scale}, WHITE);
        }

        for(u32 index = 0; index < total_blocks; index++) {
          PhysicsBlock b = blocks[index];
          f32 far_x = b.bottom_right.x - b.top_left.x + 1;
          f32 far_y = b.bottom_right.y - b.top_left.y + 1;
          u32 thickness = 3;
          DrawRectangleLinesEx({
            b.top_left.x * TILE_SIZE * level_tile_scale,
            b.top_left.y * TILE_SIZE * level_tile_scale,
            far_x * TILE_SIZE * level_tile_scale,
            far_y * TILE_SIZE * level_tile_scale
          }, thickness, GOLD);
        }

        // DrawRectangleRec();
        f32 scaled_tile_size = TILE_SIZE * level_tile_scale;
        Rectangle p = {player_position.x,player_position.y,scaled_tile_size,scaled_tile_size};
        // Rectangle ground = {0,screen_height-scaled_tile_size*2, screen_width, scaled_tile_size*2};
        // u32 thickness = 3;
        DrawRectangleLinesEx(p, 3, MAGENTA);
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
              // if((u32)tile_position.x == 0 && (u32)tile_position.y == 12) {
                // target_position = player_position;
                // u32 font_size = 70;
                // u8 spacing = 0;
                // DrawTextEx(font, TextFormat("{%f, %f}", target_position.x, target_position.y), {0,0}, font_size, spacing, MAGENTA);
                // DrawTextEx(font, TextFormat("{%f, %f}", player_velocity.x, player_velocity.y), {0,(f32)font_size}, font_size, spacing, MAGENTA);
              // }
              // u64 remainder = screen_width % (s32)TILE_SIZE;
              u64 remainder = 0;
              DrawTexturePro(tilemap, 
                {
                  (f32)tile_position.x * TILE_SIZE + remainder,
                  (f32)tile_position.y * TILE_SIZE,
                  TILE_SIZE, TILE_SIZE
                }, 
                {
                  target_position.x,
                  target_position.y,
                  TILE_SIZE * level_tile_scale, TILE_SIZE * level_tile_scale
                }, {0,0}, 0, WHITE);
            }
          }
        }
      } break;
    }

    if(engine_state != EngineState::IN_GAME) {
      DrawTexturePro(tilemap,
        {selected_tile.x * TILE_SIZE,selected_tile.y * TILE_SIZE, TILE_SIZE, TILE_SIZE},
        selected_tile_rect, {0,0}, 0, {255, 255, 255, (u8)(255 * selected_tile_alpha)});
    }

    switch(engine_state) {
      case EngineState::TILE_SELECTION: {} break;

      case EngineState::LEVEL_EDITOR: {} break;

      case EngineState::IN_GAME: {
        f32 scaled_tile_size = TILE_SIZE * level_tile_scale;
        Rectangle p = {player_position.x,player_position.y,scaled_tile_size,scaled_tile_size};

        u32 _thickness = 3;
        // DrawRectangleLinesEx(p, _thickness, MAGENTA);

        Rectangle player_rect = { (f32)1  * TILE_SIZE, (f32)12 * TILE_SIZE, TILE_SIZE, TILE_SIZE };
        Rectangle dst_rect = { p.x, p.y, TILE_SIZE * level_tile_scale, TILE_SIZE * level_tile_scale};
        Vector2 origin = {0, 0};
        f32 rotation = 0;
        Color color = WHITE;
        f32 fps = 1 / dt;

        f32 vel_threshold = 0.5;
        f32 has_left_velocity  = player_velocity.x < -vel_threshold;
        f32 has_right_velocity = player_velocity.x > vel_threshold;

        bool is_pressing_left  = IsKeyDown(KEY_A);
        bool is_pressing_right = IsKeyDown(KEY_D);

        if(is_pressing_left) {
          Rectangle anim_rect = {(f32)player_walk_animation_index * TILE_SIZE, player_rect.y, -player_rect.width, player_rect.height};
          DrawTexturePro(tilemap, anim_rect, dst_rect, origin, rotation, color);
          is_player_facing_right = false;
        } else if(is_pressing_right) {
          Rectangle anim_rect = {(f32)player_walk_animation_index * TILE_SIZE, player_rect.y, player_rect.width, player_rect.height};
          DrawTexturePro(tilemap, anim_rect, dst_rect, origin, rotation, color);
          is_player_facing_right = true;
        } else {
          if(has_left_velocity) {
            Rectangle anim_rect = {(f32)player_walk_animation_index * TILE_SIZE, player_rect.y, -player_rect.width, player_rect.height};
            DrawTexturePro(tilemap, anim_rect, dst_rect, origin, rotation, color);
          } else if(has_right_velocity) {
            Rectangle anim_rect = {(f32)player_walk_animation_index * TILE_SIZE, player_rect.y, player_rect.width, player_rect.height};
            DrawTexturePro(tilemap, anim_rect, dst_rect, origin, rotation, color);
          } else {
            Rectangle anim_rect = {
              (f32)1 * TILE_SIZE, player_rect.y,
              (is_player_facing_right ? 1 : -1) * player_rect.width, player_rect.height
            };
            DrawTexturePro(tilemap, anim_rect, dst_rect, origin, rotation, color);
            // DrawTexturePro(tilemap, player_rect, dst_rect, origin, rotation, color);
          }
        }

        if(frame_counter % (s32)(fps * 0.2) == 0) {
          player_walk_animation_index = Wrap(player_walk_animation_index++, 1, 3);
        }

        Rectangle heart_rect = {(f32)heart_animation_index * TILE_SIZE, 2 * TILE_SIZE, TILE_SIZE, TILE_SIZE};
        Rectangle heart_dst_rect = { 4, 10 * TILE_SIZE * level_tile_scale, TILE_SIZE * level_tile_scale, TILE_SIZE * level_tile_scale};
        DrawTexturePro(tilemap, heart_rect, heart_dst_rect, origin, rotation, color);

        if(frame_counter % (s32)(fps * 0.2) == 0) {
          if(heart_is_growing) {
            // heart_animation_index = Wrap(heart_animation_index++, 0, 3);
            heart_animation_index++;
            if(heart_animation_index == 2) heart_is_growing = false;
          } 
        }

        if(frame_counter % (s32)(fps * 0.3) == 0) {
          if(!heart_is_growing) {
            // heart_animation_index = Wrap(heart_animation_index--, 0, 3);
            heart_animation_index--;
            if(heart_animation_index == 0) heart_is_growing = true;
          }
        }

        frame_counter = Wrap(frame_counter++, 0, fps);

        // DrawRectangleLinesEx(ground, 3, GOLD);

        for(u32 index = 0; index < total_blocks; index++) {
          /// @todo: Copy-Pasta from editor rendering.
          PhysicsBlock b = blocks[index];
          f32 far_x = b.bottom_right.x - b.top_left.x + 1;
          f32 far_y = b.bottom_right.y - b.top_left.y + 1;
          u32 thickness = 3;
          DrawRectangleLinesEx({
            b.top_left.x * TILE_SIZE * level_tile_scale,
            b.top_left.y * TILE_SIZE * level_tile_scale,
            far_x * TILE_SIZE * level_tile_scale,
            far_y * TILE_SIZE * level_tile_scale
          }, thickness, GOLD);
        }

        Rectangle slider = {10, 10, 1000, 20};
        u32 slider_count = 3;
        u32 gap = 10;
        // DrawRectangleRec({0, 0, screen_width, (slider.height * slider_count) + (gap * (slider_count + 1))}, GRAY);

        // GuiSlider(slider, NULL, TextFormat("jump_height: %.2fs", jump_height), &jump_height, 1, 1000);

        slider.y += slider.height + gap;
        // GuiSlider(slider, NULL, TextFormat("jump_time_to_peak: %.2fs", jump_time_to_peak), &jump_time_to_peak, 0, 1);

        slider.y += slider.height + gap;
        // GuiSlider(slider, NULL, TextFormat("jump_time_to_descent: %.2fs", jump_time_to_descent), &jump_time_to_descent, 0, 1);

        // slider.y += slider.height + gap;
        // slider.width = 200;
        // GuiSlider(slider, NULL, TextFormat("friction: %.2fs", friction), &friction, 0, 2);
      } break;
    }

    EndMode2D();


    u32 font_size = 70;
    u8 spacing = 0;

    // f32 x_space = 0;
    // f32 x_gap = 10;
    // f32 x_gap_index = 1;

    // if(engine_state != EngineState::TILE_SELECTION) {
    //   u32 _font_size = 24;
    //   f32 y_pad = 10;
    //   char* key_one_text = "Press Key '1' to go to tile selection";
    //   Vector2 text_size = MeasureTextEx(font, key_one_text, _font_size, spacing);
    //   Vector2 pos = {x_gap * x_gap_index + x_space, y_pad};
    //   DrawRectangleRec({pos.x, pos.y, text_size.x, text_size.y }, TW_STONE[5]);
    //   // DrawTextEx(font, key_one_text, pos + 3, _font_size, spacing, BLACK);
    //   DrawTextEx(font, key_one_text, pos, _font_size, spacing, WHITE);

    //   x_space += text_size.x;
    //   x_gap_index++;
    // }

    // if(engine_substate != EngineSubState::TILE_PLACEMENT) {
    //   u32 _font_size = 24;
    //   f32 y_pad = 10;
    //   char* key_one_text = "Press Key '2' to go to level editor";
    //   Vector2 text_size = MeasureTextEx(font, key_one_text, _font_size, spacing);
    //   Vector2 pos = {x_gap * x_gap_index + x_space, y_pad};
    //   DrawRectangleRec({pos.x, pos.y, text_size.x, text_size.y }, TW_STONE[5]);
    //   // DrawTextEx(font, key_one_text, pos + 3, _font_size, spacing, BLACK);
    //   DrawTextEx(font, key_one_text, pos, _font_size, spacing, WHITE);

    //   x_space += text_size.x;
    //   x_gap_index++;
    // }

    // if(engine_substate != EngineSubState::PHYSICS_PLACEMENT) {
    //   u32 _font_size = 24;
    //   f32 y_pad = 10;
    //   char* key_one_text = "Press Key '3' to go to physics editor";
    //   Vector2 text_size = MeasureTextEx(font, key_one_text, _font_size, spacing);
    //   Vector2 pos = {x_gap * x_gap_index + x_space, y_pad};
    //   DrawRectangleRec({pos.x, pos.y, text_size.x, text_size.y }, TW_STONE[5]);
    //   // DrawTextEx(font, key_one_text, pos + 3, _font_size, spacing, BLACK);
    //   DrawTextEx(font, key_one_text, pos, _font_size, spacing, WHITE);

    //   x_space += text_size.x;
    //   x_gap_index++;
    // }

    // if(engine_state != EngineState::IN_GAME) {
    //   u32 _font_size = 24;
    //   f32 y_pad = 10;
    //   char* key_one_text = "Press Key '4' to test the game";
    //   Vector2 text_size = MeasureTextEx(font, key_one_text, _font_size, spacing);
    //   Vector2 pos = {x_gap * x_gap_index + x_space, y_pad};
    //   DrawRectangleRec({pos.x, pos.y, text_size.x, text_size.y }, TW_STONE[5]);
    //   // DrawTextEx(font, key_one_text, pos + 3, _font_size, spacing, BLACK);
    //   DrawTextEx(font, key_one_text, pos, _font_size, spacing, WHITE);

    //   x_space += text_size.x;
    //   x_gap_index++;
    // }
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
    
    // {
    //   char* hello_sailor_text = i18n(dictionary_index, "japanese_char_test");
    //   Vector2 pos = {screen_center.x - MeasureTextEx(font, hello_sailor_text, font_size, spacing).x / 2, screen_center.y + font_size};
    //   DrawTextEx(font, hello_sailor_text, pos, font_size, spacing, BLACK);
    //   DrawTextEx(font, hello_sailor_text, pos + 3, font_size, spacing, WHITE);
    // }

    EndDrawing();
  }

  CloseWindow();
  return 0;
}

void load_level() {
  FILE* file = fopen(level_file_path, "r");

  for(u32 col = 0; col < level_height; col++) {
    for(u32 row = 0; row < level_width; row++) {
      f32 x, y;
      fscanf(file, "%f", &x);
      fscanf(file, "%f", &y);
      level[col][row] = {x, y};
    }
  }

  fclose(file);
}

void load_physics_blocks() {
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

void load_player_position() {
  FILE* file = fopen(player_file_path, "r");
  fscanf(file, "%f", &(player_position.x));
  fscanf(file, "%f", &(player_position.y));
  fclose(file);
}