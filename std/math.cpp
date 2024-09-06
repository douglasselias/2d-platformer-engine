#define max fmax
#define min fmin
#define abs fabs
#define mod fmod

f32 clamp(f32 value, f32 min, f32 max) {
  return Clamp(value, min, max);
}

Vector2 clamp(Vector2 value, Vector2 min, Vector2 max) {
  return Vector2Clamp(value, min, max);
}

Vector2 clamp(Vector2 value, f32 min, f32 max) {
  return Vector2ClampValue(value, min, max);
}

Vector3 clamp(Vector3 value, Vector3 min, Vector3 max) {
  return Vector3Clamp(value, min, max);
}

Vector3 clamp(Vector3 value, f32 min, f32 max) {
  return Vector3ClampValue(value, min, max);
}

f32 lerp(f32 start, f32 end, f32 amount) {
  return Lerp(start, end, amount);
}

Vector2 lerp(Vector2 a, Vector2 b, f32 amount) {
  return Vector2Lerp(a, b, amount);
}

Vector3 lerp(Vector3 a, Vector3 b, f32 amount) {
  return Vector3Lerp(a, b, amount);
}

f32 wrap(f32 value, f32 min, f32 max) {
  return Wrap(value, min, max);
}

Vector2 wrap(Vector2 value, Vector2 min, Vector2 max) {
  return {
    Wrap(value.x, min.x, max.x),
    Wrap(value.y, min.y, max.y),
  };
}

Vector3 wrap(Vector3 value, Vector3 min, Vector3 max) {
  return {
    Wrap(value.x, min.x, max.x),
    Wrap(value.y, min.y, max.y),
    Wrap(value.z, min.z, max.z),
  };
}

f32 vector_length(Vector2 v) {
  return Vector2Length(v);
}

f32 vector_length(Vector3 v) {
  return Vector3Length(v);
}

f32 vector_length_sqr(Vector2 v) {
  return Vector2LengthSqr(v);
}

f32 vector_length_sqr(Vector3 v) {
  return Vector3LengthSqr(v);
}

f32 vector_dot_product(Vector2 a, Vector2 b) {
  return Vector2DotProduct(a, b);
}

f32 vector_dot_product(Vector3 a, Vector3 b) {
  return Vector3DotProduct(a, b);
}

f32 vector_distance(Vector2 a, Vector2 b) {
  return Vector2Distance(a, b);
}

f32 vector_distance(Vector3 a, Vector3 b) {
  return Vector3Distance(a, b);
}

f32 vector_distance_sqr(Vector2 a, Vector2 b) {
  return Vector2DistanceSqr(a, b);
}

f32 vector_distance_sqr(Vector3 a, Vector3 b) {
  return Vector3DistanceSqr(a, b);
}

f32 vector_angle(Vector2 a, Vector2 b) {
  return Vector2Angle(a, b);
}

f32 vector_angle(Vector3 a, Vector3 b) {
  return Vector3Angle(a, b);
}

Vector2 vector_negate(Vector2 v) {
  return Vector2Negate(v);
}

Vector3 vector_negate(Vector3 v) {
  return Vector3Negate(v);
}

Vector2 vector_normalize(Vector2 v) {
  return Vector2Normalize(v);
}

Vector3 vector_normalize(Vector3 v) {
  return Vector3Normalize(v);
}

Vector2 vector_transform(Vector2 v, Matrix mat) {
  return Vector2Transform(v, mat);
}

Vector3 vector_transform(Vector3 v, Matrix mat) {
  return Vector3Transform(v, mat);
}

Vector2 vector_reflect(Vector2 v, Vector2 normal) {
  return Vector2Reflect(v, normal);
}

Vector3 vector_reflect(Vector3 v, Vector3 normal) {
  return Vector3Reflect(v, normal);
}

Vector2 vector_invert(Vector2 v) {
  return Vector2Invert(v);
}

Vector3 vector_invert(Vector3 v) {
  return Vector3Invert(v);
}

Vector2 vector_min(Vector2 a, Vector2 b) {
  return {
    (f32)min(a.x, b.x),
    (f32)min(a.y, b.y),
  };
}

Vector3 vector_min(Vector3 a, Vector3 b) {
  return Vector3Min(a, b);
}

Vector2 vector_max(Vector2 a, Vector2 b) {
  return {
    (f32)max(a.x, b.x),
    (f32)max(a.y, b.y),
  };
}

Vector3 vector_max(Vector3 a, Vector3 b) {
  return Vector3Max(a, b);
}

struct float2 {
  f32 v[2];
};

float2 vector_to_float_array(Vector2 v) {
  float2 buffer = {};

  buffer.v[0] = v.x;
  buffer.v[1] = v.y;

  return buffer;
}

float3 vector_to_float_array(Vector3 v) {
  return Vector3ToFloatV(v);
}