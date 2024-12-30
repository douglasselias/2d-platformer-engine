#define vector2_op_value_lhs(type, op) \
Vector2 operator op(const Vector2 lhs, const type value) { \
  return {lhs.x op value, lhs.y op value}; \
}

vector2_op_value_lhs(f32, +)
vector2_op_value_lhs(f32, -)
vector2_op_value_lhs(f32, *)
vector2_op_value_lhs(f32, /)

#define vector2_op_value_rhs(type, op) \
Vector2 operator op(const type value, const Vector2 rhs) { \
  return {rhs.x op value, rhs.y op value}; \
}

vector2_op_value_rhs(f32, +)
vector2_op_value_rhs(f32, -)
vector2_op_value_rhs(f32, *)
vector2_op_value_rhs(f32, /)

#define vector3_op_value_lhs(type, op) \
Vector3 operator op(const Vector3 lhs, const type value) { \
  return {lhs.x op value, lhs.y op value, lhs.z op value}; \
}

vector3_op_value_lhs(f32, +)
vector3_op_value_lhs(f32, -)
vector3_op_value_lhs(f32, *)
vector3_op_value_lhs(f32, /)

#define vector3_op_value_rhs(type, op) \
Vector3 operator op(const type value, const Vector3 rhs) { \
  return {rhs.x op value, rhs.y op value, rhs.z op value}; \
}

vector3_op_value_rhs(f32, +)
vector3_op_value_rhs(f32, -)
vector3_op_value_rhs(f32, *)
vector3_op_value_rhs(f32, /)

#define mutable_vector2_op_value(type, op) \
Vector2 &operator op(Vector2 &lhs, const type value) { \
  lhs.x op value; \
  lhs.y op value; \
  return lhs; \
}

mutable_vector2_op_value(f32, +=)
mutable_vector2_op_value(f32, -=)
mutable_vector2_op_value(f32, *=)
mutable_vector2_op_value(f32, /=)

#define mutable_vector3_op_value(type, op) \
Vector3 &operator op(Vector3 &lhs, const type value) { \
  lhs.x op value; \
  lhs.y op value; \
  lhs.z op value; \
  return lhs; \
}

mutable_vector3_op_value(f32, +=)
mutable_vector3_op_value(f32, -=)
mutable_vector3_op_value(f32, *=)
mutable_vector3_op_value(f32, /=)

#define vector2_op_vector(op) \
Vector2 operator op(const Vector2 &lhs, const Vector2 &rhs) { \
  return {lhs.x op rhs.x, lhs.y op rhs.y}; \
}

vector2_op_vector(+)
vector2_op_vector(-)
vector2_op_vector(*)
vector2_op_vector(/)

#define vector3_op_vector(op) \
Vector3 operator op(const Vector3 &lhs, const Vector3 &rhs) { \
  return {lhs.x op rhs.x, lhs.y op rhs.y, lhs.z op rhs.z}; \
}

vector3_op_vector(+)
vector3_op_vector(-)
vector3_op_vector(*)
vector3_op_vector(/)

#define mutable_vector2_op_vector(op) \
Vector2 &operator op(Vector2 &lhs, const Vector2 &rhs) { \
  lhs.x op rhs.x; \
  lhs.y op rhs.y; \
  return lhs; \
}

mutable_vector2_op_vector(+=)
mutable_vector2_op_vector(-=)
mutable_vector2_op_vector(*=)
mutable_vector2_op_vector(/=)

#define mutable_vector3_op_vector(op) \
Vector3 &operator op(Vector3 &lhs, const Vector3 &rhs) { \
  lhs.x op rhs.x; \
  lhs.y op rhs.y; \
  lhs.z op rhs.z; \
  return lhs; \
}

mutable_vector3_op_vector(+=)
mutable_vector3_op_vector(-=)
mutable_vector3_op_vector(*=)
mutable_vector3_op_vector(/=)

bool operator ==(const Vector2 &lhs, const Vector2 &rhs) {
  return (bool)Vector2Equals(lhs, rhs);
}

bool operator ==(const Vector3 &lhs, const Vector3 &rhs) {
  return (bool)Vector3Equals(lhs, rhs);
}

Vector2 operator -(const Vector2 &v) {
  return {-v.x, -v.y};
}

Vector3 operator -(const Vector3 &v) {
  return {-v.x, -v.y, -v.z};
}
