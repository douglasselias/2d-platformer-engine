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
  SetTargetFPS(60);

  InitAudioDevice();
  SetRandomSeed((u32)time(null));

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