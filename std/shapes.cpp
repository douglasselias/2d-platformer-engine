struct Line {
  Vector2 start;
  Vector2 end;
};

void draw_line(Line line, Color color, f32 thickness = 0) {
  if(FloatEquals(thickness, 0)) DrawLineV(line.start, line.end, color);
  else DrawLineEx(line.start, line.end, thickness, color);
}

void draw_bezier_line(Line line, Color color, f32 thickness = 0) {
  DrawLineBezier(line.start, line.end, thickness, color);
}

bool both_lines_are_colliding(Line a, Line b, Vector2 *collision_point) {
  return CheckCollisionLines(a.start, a.end, b.start, b.end, collision_point);
}

bool point_and_line_are_colliding(Vector2 point, Line line, s32 threshold) {
  return CheckCollisionPointLine(point, line.start, line.end, threshold);
}

struct Circle {
  Vector2 center;
  f32 radius;
};

void draw_circle(Circle circle, Color color) {
  DrawCircleV(circle.center, circle.radius, color);
}

/// @todo: Can segments be negative???
void draw_circle_sector(Circle circle, f32 start_angle, f32 end_angle, u32 segments, Color color) {
  DrawCircleSector(circle.center, circle.radius, start_angle, end_angle, (s32)segments, color);
}

void draw_circle_sector_lines(Circle circle, f32 start_angle, f32 end_angle, u32 segments, Color color) {
  DrawCircleSectorLines(circle.center, circle.radius, start_angle, end_angle, (s32)segments, color);
}

void draw_circle_gradient(Circle circle, Color a, Color b) {
  DrawCircleGradient((s32)circle.center.x, (s32)circle.center.y, circle.radius, a, b);
}

void draw_circle_lines(Circle circle, Color color) {
  DrawCircleLinesV(circle.center, circle.radius, color);
}

bool both_circles_are_colliding(Circle a, Circle b) {
  return CheckCollisionCircles(a.center, a.radius, b.center, b.radius);
}

bool circle_and_rect_are_colliding(Circle circle, Rectangle rec) {
  return CheckCollisionCircleRec(circle.center, circle.radius, rec);
}

bool point_and_circle_are_colliding(Vector2 point, Circle circle) {
  return CheckCollisionPointCircle(point, circle.center, circle.radius);
}

struct Triangle {
  Vector2 a;
  Vector2 b;
  Vector2 c;
};

void draw_triangle(Triangle triangle, Color color) {
  DrawTriangle(triangle.a, triangle.b, triangle.c, color);
}

void draw_triangle_lines(Triangle triangle, Color color) {
  DrawTriangleLines(triangle.a, triangle.b, triangle.c, color);
}

bool point_and_triangle_are_colliding(Vector2 point, Triangle triangle) {
  return CheckCollisionPointTriangle(point, triangle.a, triangle.b, triangle.c);
}
