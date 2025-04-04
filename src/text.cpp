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
  const char* i18n_path = "../i18n.csv";
  #if DEV == 1
  char *csv = LoadFileText(i18n_path);
  #elif EXPORT_I18N == 1
  char *csv = LoadFileText(i18n_path);

  s32 *bin_size = (s32*)MemAlloc(sizeof(s32));
  u8 *i18n_bin = LoadFileData(i18n_path, bin_size);
  s32 *compressed_size = (s32*)MemAlloc(sizeof(s32));
  u8* compressed_i18n = CompressData(i18n_bin, *bin_size, compressed_size);
  if(ExportDataAsCode(compressed_i18n, *compressed_size, "../bundle/i18n.cpp")) { puts("Success! (i18n Exported)"); }
  #else
  s32 *decompressed_i18n_size = (s32*)MemAlloc(sizeof(s32));
  char* csv = (char*)DecompressData(I18N_DATA, I18N_DATA_SIZE, decompressed_i18n_size);
  #endif

  for(char *line = strtok(csv, "\n"); line != null; line = strtok(null, "\n")) {
    char key[200] = {};
    char en[200] = {};
    char cn[200] = {};
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

  char* alphabet = "1234567890qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM?!@#$%&().;>:<,[]{}/'-";
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
