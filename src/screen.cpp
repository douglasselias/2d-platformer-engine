const char *game_title = "Game";
// const u16 screen_width  = 1920;
// const u16 screen_height = 1080;

const u16 screen_width  = 1280;
const u16 screen_height = 720;
const u16 half_screen_width  = screen_width  / 2;
const u16 half_screen_height = screen_height / 2;
const Vector2 screen_center = {half_screen_width, half_screen_height};

void init_screen() {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  SetTraceLogLevel(LOG_WARNING);
  InitWindow(screen_width, screen_height, game_title);
  // SetWindowState(FLAG_WINDOW_UNDECORATED);
  Image icon = LoadImage("../icons/game16.png");
  SetWindowIcon(icon);
  SetTargetFPS(60);

  InitAudioDevice();
  SetRandomSeed((u32)time(null));
}

void draw_texture(Texture2D texture, Vector2 position, f32 scale = 1, Color tint = WHITE) {
  Rectangle source = {0, 0, (f32)texture.width, (f32)texture.height};
  Rectangle dest = {position.x, position.y, (f32)texture.width * scale, (f32)texture.height * scale};
  Vector2 origin = {0,0};
  u8 rotation = 0;
  DrawTexturePro(texture, source, dest, origin, rotation, tint);
}

// s32 load_font_size = 100;
// f32 font_spacing = 0;

// Font font;
// f32 font_size = 70;

// Vector2 measure_text(const char* text) {
//   return MeasureTextEx(font, text, font_size, font_spacing);
// }

// void draw_text(const char* text, Vector2 position, Color color) {
//   DrawTextEx(font, text, position, font_size, font_spacing, color);
// }

// void draw_text_centered(const char* text, f32 y, Color color, bool shadow_text = false) {
//   Vector2 text_size = measure_text(text);
//   Vector2 text_position = {screen_center.x - text_size.x/2, y};
//   if(shadow_text) draw_text(text, text_position + 6, BLACK);
//   draw_text(text, text_position, color);
// }

void draw_text_centered(const char* text, f32 y, Color color) {
  static Font font = GetFontDefault();
  static u8 font_size = 32;
  static u8 font_spacing = 4;
  Vector2 text_size = MeasureTextEx(font, text, font_size, font_spacing);
  Vector2 position = {screen_center.x - text_size.x/2, y};
  // draw_text(text, text_position, color);
  DrawTextEx(font, text, position, font_size, font_spacing, color);
}