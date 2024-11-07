enum Languages {
  EN,
  CN,
  PT_BR,
};

#define HASH_TABLE_SIZE 4096

u64 hash_key(const char *key) {
  u64 hash = 0;

  while(*key) {
    hash = 31 * hash + *key;
    key++;
  }

  return hash % HASH_TABLE_SIZE;
}

char *english_texts[4096] = {};
char *chinese_texts[4096] = {};

char *chinese_chars[4096] = {};

#define bytes_of_chinese_char 3

void init_i18n() {
  char *csv = LoadFileText("../i18n.csv");

  for(char *line = strtok(csv, "\n"); line != null; line = strtok(null, "\n")) {
    char key[200];
    char en[200];
    char cn[200];
    sscanf(line, "%[^,],%[^,],%[^\n]", key, en, cn);

    u64 key_hash = hash_key(key);
    english_texts[key_hash] = strdup(en);
    chinese_texts[key_hash] = strdup(cn);

    u8 index = 0;
    char current_char[bytes_of_chinese_char] = {};
    u64 len = strlen(cn) / bytes_of_chinese_char;
    while(index < len) {
      strncpy(current_char, cn + bytes_of_chinese_char * index, bytes_of_chinese_char);
      chinese_chars[hash_key(current_char)] = strdup(current_char);
      index++;
      memset(current_char, 0, sizeof(current_char));
    }
  }
}

char* i18n(u64 dictionary_index, const char* key) {
  switch(dictionary_index) {
    case 0: return english_texts[hash_key(key)];
    case 1: return chinese_texts[hash_key(key)];
  }
  return "";
}

Font load_font() {
  u64 total_chars = 0;
  for(s16 i = 0; i < 4096; i++) {
    if(chinese_chars[i]) {
      total_chars++;
    }
  }
  u32 chinese_chars_bytes = sizeof(char) * bytes_of_chinese_char * total_chars;

  char* alphabet = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM?!@#$%&().;>:<,[]{}/'";
  u32 alphabet_bytes = (u32)strlen(alphabet) * sizeof(char);
  char* all_codepoints = (char*)MemAlloc(alphabet_bytes + chinese_chars_bytes);

  strcpy(all_codepoints, alphabet);
  for(s16 i = 0; i < 4096; i++) {
    if(chinese_chars[i]) {
      strcat(all_codepoints, chinese_chars[i]);
    }
  }

  u32 font_size = 100;
  s32 count;
  s32* codepoints = LoadCodepoints(all_codepoints, &count);
  Font font = LoadFontEx("../fonts/noto_sans_chinese_regular.ttf", font_size, codepoints, count);
  SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
  return font;
}

void draw_text_centered(const char* text, f32 y, Color color, bool shadow_text = false) {
  static Font font = GetFontDefault();
  static u8 font_size = 32;
  static u8 font_spacing = 4;
  Vector2 text_size = MeasureTextEx(font, text, font_size, font_spacing);
  Vector2 position = {screen_center.x - text_size.x/2, y};
  if(shadow_text) DrawTextEx(font, text, position + 3, font_size, font_spacing, BLACK);
  DrawTextEx(font, text, position, font_size, font_spacing, color);
}