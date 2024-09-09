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

// struct HashTable {
//   u64 count;
//   u64 max_size;
//   u8 *table[4096];
// };

// HashTable *create_ht(Arena *arena, u64 size) {
//   HashTable *ht = alloc_arena(arena, sizeof(HashTable));
//   ht->count = 0;
//   ht->max_size = size;
//   ht->table = alloc_arena(arena, size);
//   return ht;
// }

#define HASH_TABLE_SIZE 4096

u64 hash_key(const char *key) {
  u64 hash = 0;

  while(*key) {
    hash += *key;
    key++;
  }

  return hash % 4096;
}

// void insert_ht(HashTable *ht, u8 *key, u8 value) {
//   u64 hash = hash_key(key);
//   printf("Hash: %ld\n", hash);
//   ht->table[hash] = value;
//   ht->count++;
// }

// u8 get_ht(HashTable *ht, u8 *key) {
//   u64 hash = hash_key(key);
//   return ht->table[hash];
// }



// char* append_chinese_char(char* str, char* c) {
//   u64 total_len = strlen(str) + strlen(c) + 1;
  
//   strcpy(str);
//   return result;
// }

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
  // s32* codepoints = LoadCodepoints("qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM!@#$%&().;>:<,[]{}/我是猫", &count);
  char* alphabet = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM!@#$%&().;>:<,[]{}/";
  u64 alphabet_length = strlen(alphabet);
  char* all_codepoints = (char*)MemAlloc(alphabet_length * sizeof(char) + sizeof(char) * 3 * total_chars);
  // char* all_codepoints = (char*)malloc(sizeof(char*)*1000);

  strcpy(all_codepoints, alphabet);
   for(s16 i = 0; i < 4096; i++) {
    if(chinese_chars[i]) {
      strcat(all_codepoints, chinese_chars[i]);
    }
  }

  s32 count;
  s32* codepoints = LoadCodepoints(all_codepoints, &count);
  puts(all_codepoints);
  Font font = LoadFontEx("../fonts/noto_serif_chinese_regular.ttf", 700, codepoints, count);
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

s32 main() {
  init_screen();

  i18n_init();
  u64 dictionary_index = 0;

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
  bool is_fullscreen = false;

  Camera camera = {};
  camera.position = {0, 0, -10};
  camera.target = {0, 0, 0};
  camera.up = {0, 1, 0};
  camera.fovy = 45;
  camera.projection = CAMERA_PERSPECTIVE;

  Vector3 cube_position = {0, 0, 0};

  #if 1
  Music music = LoadMusicStream("../musics/battle.wav");
  #elif 0
  Music music = LoadMusicStreamFromMemory(".wav", music_bin, *bin_size);
  #else
  Music music = LoadMusicStreamFromMemory(".wav", MUSIC_DATA, MUSIC_DATA_SIZE);
  #endif
  PlayMusicStream(music);

  while (!WindowShouldClose()) {
    f32 dt = GetFrameTime();
    UpdateMusicStream(music);

    if(IsKeyPressed(KEY_SPACE)) {
      dictionary_index = dictionary_index == 0 ? 1 : 0;
    }
    
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
        // DrawCube(cube_position, 2.0f, 2.0f, 2.0f, RED);
        DrawTriangle3D(cube_position, {0, 10, cube_position.z}, {5, 10, cube_position.z}, GREEN);
      EndMode3D();

      // DrawTextEx(font, "我是猫 Congrats! You created your first window!", {10,screen_center.y}, 250, 4, BLACK);
      DrawTextEx(font, i18n(dictionary_index, "hello_world"), {10,screen_center.y}, 250, 4, BLACK);
      // DrawTextEx(font, i18n("main_menu_play"), {10,screen_center.y}, 250, 4, BLACK);
      // draw_text_centered("Congrats! You created your first window!", 190, BLACK);

      // draw_circle({screen_center, 100}, GRAY);
      // draw_circle_sector({screen_center, 50}, 0, 90, 2, BLUE);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}