void log(const char* text) {
  TraceLog(LOG_WARNING, text);
}

void log(const char* text, const char* text_value) {
  TraceLog(LOG_WARNING, TextFormat("%s: %s", text, text_value));
}

void log(const char* name, Vector2 vector) {
  TraceLog(LOG_WARNING, TextFormat("%s: (%.3f, %.3f)", name, (f64)vector.x, (f64)vector.y));
}

#define declare_float_log(type) \
void log(const char* name, type value) { \
  TraceLog(LOG_WARNING, TextFormat("%s: %.3f", name, (f64)value)); \
}

declare_float_log(f32)
declare_float_log(f64)

#define declare_int_log(type) \
void log(const char* name, type value) { \
  TraceLog(LOG_WARNING, TextFormat("%s: %d", name, value)); \
}

declare_int_log(s8)
declare_int_log(s16)
declare_int_log(s32)
declare_int_log(s64)

declare_int_log(u8)
declare_int_log(u16)
declare_int_log(u32)
declare_int_log(u64)