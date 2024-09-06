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
  Vector3ClampValue(value, min, max);
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

#if 0
float Normalize(float value, float start, float end);
float Remap(float value, float inputStart, float inputEnd, float outputStart, float outputEnd);
float Wrap(float value, float min, float max);


float Vector2Length(Vector2 v);
float Vector2LengthSqr(Vector2 v);
float Vector2DotProduct(Vector2 v1, Vector2 v2);
float Vector2Distance(Vector2 v1, Vector2 v2);
float Vector2DistanceSqr(Vector2 v1, Vector2 v2);
float Vector2Angle(Vector2 v1, Vector2 v2);                                 // Calculate angle from two vectors
Vector2 Vector2Negate(Vector2 v);                                           // Negate vector
Vector2 Vector2Normalize(Vector2 v);                                        // Normalize provided vector
Vector2 Vector2Transform(Vector2 v, Matrix mat);                            // Transforms a Vector2 by a given Matrix
Vector2 Vector2Reflect(Vector2 v, Vector2 normal);                          // Calculate reflected vector to normal
Vector2 Vector2Rotate(Vector2 v, float angle);                              // Rotate vector by angle
Vector2 Vector2MoveTowards(Vector2 v, Vector2 target, float maxDistance);   // Move Vector towards target
Vector2 Vector2Invert(Vector2 v);                                           // Invert the given vector



    Vector3 Vector3Scale(Vector3 v, float scalar);                              // Multiply vector by scalar
    Vector3 Vector3CrossProduct(Vector3 v1, Vector3 v2);                        // Calculate two vectors cross product
    Vector3 Vector3Perpendicular(Vector3 v);                                    // Calculate one vector perpendicular vector
    float Vector3Length(const Vector3 v);                                       // Calculate vector length
    float Vector3LengthSqr(const Vector3 v);                                    // Calculate vector square length
    float Vector3DotProduct(Vector3 v1, Vector3 v2);                            // Calculate two vectors dot product
    float Vector3Distance(Vector3 v1, Vector3 v2);                              // Calculate distance between two vectors
    float Vector3DistanceSqr(Vector3 v1, Vector3 v2);                           // Calculate square distance between two vectors
    float Vector3Angle(Vector3 v1, Vector3 v2);                                 // Calculate angle between two vectors
    Vector3 Vector3Negate(Vector3 v);                                           // Negate provided vector (invert direction)
    Vector3 Vector3Divide(Vector3 v1, Vector3 v2);                              // Divide vector by vector
    Vector3 Vector3Normalize(Vector3 v);                                        // Normalize provided vector
    void Vector3OrthoNormalize(Vector3 *v1, Vector3 *v2);                       // Orthonormalize provided vectors Makes vectors normalized and orthogonal to each other Gram-Schmidt function implementation
    Vector3 Vector3Transform(Vector3 v, Matrix mat);                            // Transforms a Vector3 by a given Matrix
    Vector3 Vector3RotateByQuaternion(Vector3 v, Quaternion q);                 // Transform a vector by quaternion rotation
    Vector3 Vector3RotateByAxisAngle(Vector3 v, Vector3 axis, float angle);     // Rotates a vector around an axis
    Vector3 Vector3Reflect(Vector3 v, Vector3 normal);                          // Calculate reflected vector to normal
    Vector3 Vector3Min(Vector3 v1, Vector3 v2);                                 // Get min value for each pair of components
    Vector3 Vector3Max(Vector3 v1, Vector3 v2);                                 // Get max value for each pair of components
    Vector3 Vector3Barycenter(Vector3 p, Vector3 a, Vector3 b, Vector3 c);      // Compute barycenter coordinates (u, v, w) for point p with respect to triangle (a, b, c) NOTE: Assumes P is on the plane of the triangle
    Vector3 Vector3Unproject(Vector3 source, Matrix projection, Matrix view);   // Projects a Vector3 from screen space into object space NOTE: We are avoiding calling other raymath functions despite available
    float3 Vector3ToFloatV(Vector3 v);                                          // Get Vector3 as float array
    Vector3 Vector3Invert(Vector3 v);                                           // Invert the given vector

    Vector3 Vector3Refract(Vector3 v, Vector3 n, float r);                      // Compute the direction of a refracted ray where v specifies the normalized direction of the incoming ray, n specifies the normalized normal vector of the interface of two optical media, and r specifies the ratio of the refractive index of the medium from where the ray comes to the refractive index of the medium on the other side of the surface
#endif