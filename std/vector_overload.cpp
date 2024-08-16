#define vector_op_value(type, op) \
Vector2 operator op(const Vector2 lhs, const type value) { \
  return {lhs.x op value, lhs.y op value}; \
}

vector_op_value(f32, +)
vector_op_value(f32, -)
vector_op_value(f32, *)
vector_op_value(f32, /)

#define mutable_vector_op_value(type, op) \
Vector2 &operator op(Vector2 &lhs, const type value) { \
  lhs.x op value; \
  lhs.y op value; \
  return lhs; \
}

mutable_vector_op_value(f32, +=)
mutable_vector_op_value(f32, -=)
mutable_vector_op_value(f32, *=)
mutable_vector_op_value(f32, /=)

#define vector_op_vector(op) \
Vector2 operator op(const Vector2 &lhs, const Vector2 &rhs) { \
  return {lhs.x op rhs.x, lhs.y op rhs.y}; \
}

vector_op_vector(+)
vector_op_vector(-)
vector_op_vector(*)
vector_op_vector(/)

#define mutable_vector_op_vector(op) \
Vector2 &operator op(Vector2 &lhs, const Vector2 &rhs) { \
  lhs.x op rhs.x; \
  lhs.y op rhs.y; \
  return lhs; \
}

mutable_vector_op_vector(+=)
mutable_vector_op_vector(-=)
mutable_vector_op_vector(*=)
mutable_vector_op_vector(/=)