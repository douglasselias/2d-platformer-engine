void log(const char* text) {
  puts(text);
}

void log(const char* name, const char* value) {
  puts(TextFormat("%s: %s", name, value));
}

void log(const char* name, Vector2 vector) {
  puts(TextFormat("%s: (%.3f, %.3f)", name, (f64)vector.x, (f64)vector.y));
}

void log(const char* name, Vector3 vector) {
  TraceLog(LOG_WARNING, TextFormat("%s: (%.3f, %.3f, %.3f)", name, (f64)vector.x, (f64)vector.y, (f64)vector.z));
}

void log(const char* name, Vector4 vector) {
  TraceLog(LOG_WARNING, TextFormat("%s: (%.3f, %.3f, %.3f, %.3f)", name, (f64)vector.x, (f64)vector.y, (f64)vector.z, (f64)vector.w));
}

#define declare_float_log(type) \
void log(const char* name, type value) { \
  puts(TextFormat("%s: %.3f", name, (f64)value)); \
}

declare_float_log(f32)
declare_float_log(f64)

#define declare_int_log(type) \
void log(const char* name, type value) { \
  puts(TextFormat("%s: %d", name, value)); \
}

declare_int_log(s8)
declare_int_log(s16)
declare_int_log(s32)
declare_int_log(s64)

declare_int_log(u8)
declare_int_log(u16)
declare_int_log(u32)
declare_int_log(u64)