#include "renderer.h"

#include "platform/platform.cc"
#include "shader.h"
#include "ui.cc"
#include "gl/gl.cc"
#include "gl/shader.cc"
#include "math/mat_ops.h"

namespace rgg {

struct GeometryProgram {
  GLuint reference;
  GLuint matrix_uniform;
  GLuint color_uniform;
};

struct CircleProgram {
  GLuint reference;
  GLuint model_uniform;
  GLuint view_projection_uniform;
  GLuint color_uniform;
  GLuint radius_uniform;
};

struct RGG {
  math::Mat4f projection;
  math::Mat4f view;
  math::Mat4f camera_transform;

  GeometryProgram geometry_program;
  CircleProgram circle_program;

  // References to vertex data on GPU.
  GLuint triangle_vao_reference;
  GLuint rectangle_vao_reference;
  GLuint line_vao_reference;

  int meter_size = 50;
};

static RGG kRGG;

bool
SetupGeometryProgram()
{
  GLuint vert_shader, frag_shader;
  if (!gl::CompileShader(GL_VERTEX_SHADER, &rgg::kVertexShader,
                         &vert_shader)) {
    return false;
  }

  if (!gl::CompileShader(GL_FRAGMENT_SHADER, &rgg::kFragmentShader,
                         &frag_shader)) {
    return false;
  }

  if (!gl::LinkShaders(&kRGG.geometry_program.reference, 2, vert_shader,
                       frag_shader)) {
    return false;
  }

  // No use for the basic shaders after the program is linked.
  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  kRGG.geometry_program.matrix_uniform
      = glGetUniformLocation(kRGG.geometry_program.reference, "matrix");
  assert(kRGG.geometry_program.matrix_uniform != uint32_t(-1));
  kRGG.geometry_program.color_uniform
      = glGetUniformLocation(kRGG.geometry_program.reference, "color");
  assert(kRGG.geometry_program.color_uniform != uint32_t(-1));
  return true;
}

bool
SetupCircleProgram()
{
  GLuint vert_shader, frag_shader;
  if (!gl::CompileShader(GL_VERTEX_SHADER, &rgg::kCircleVertexShader,
                         &vert_shader)) {
    return false;
  }

  if (!gl::CompileShader(GL_FRAGMENT_SHADER, &rgg::kCircleFragmentShader,
                         &frag_shader)) {
    return false;
  }

  if (!gl::LinkShaders(&kRGG.circle_program.reference, 2, vert_shader,
                       frag_shader)) {
    return false;
  }

  // No use for the basic shaders after the program is linked.
  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  kRGG.circle_program.model_uniform
      = glGetUniformLocation(kRGG.circle_program.reference, "model");
  assert(kRGG.circle_program.model_uniform != uint32_t(-1));
  kRGG.circle_program.view_projection_uniform
      = glGetUniformLocation(kRGG.circle_program.reference, "view_projection");
  assert(kRGG.circle_program.model_uniform != uint32_t(-1));

  kRGG.circle_program.color_uniform
      = glGetUniformLocation(kRGG.circle_program.reference, "color");
  assert(kRGG.circle_program.color_uniform != uint32_t(-1));
  return true;
}

bool
Initialize()
{
  const GLubyte* renderer = glGetString(GL_RENDERER);
  const GLubyte* version = glGetString(GL_VERSION);
  printf("Renderer: %s Version: %s\n", renderer, version);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glEnable(GL_LINE_SMOOTH);

  // Compile and link shaders.
  if (!SetupGeometryProgram()) return false;
  if (!SetupCircleProgram()) return false;

  // Create the geometry for basic shapes.
  float m = kRGG.meter_size;
  GLfloat tri[9] = {0.0f, m / 2.f, 0.f, m / 2.f, -m / 2.f, 0.f, -m / 2.f,
                    -m / 2.f, 0.f};
  kRGG.triangle_vao_reference = gl::CreateGeometryVAO(9, tri);

  // Rectangle. Notice it's a square. Scale to make rectangly.
  GLfloat square[18] = {
      -m / 2.f, m / 2.f, 0.f,
      m / 2.f, m / 2.f, 0.f,
      m / 2.f, -m / 2.f, 0.f,

      -m / 2.f, -m / 2.f, 0.f,
      -m / 2.f, m / 2.f, 0.f,
      m / 2.f, -m / 2.f, 0.f
  };
  kRGG.rectangle_vao_reference = gl::CreateGeometryVAO(18, square);

  // Line is flat on the x-axis with distance m.
  GLfloat line[6] = {-1.f, 0.f, 0.f, 1.f, 0.f, 0.f};
  kRGG.line_vao_reference = gl::CreateGeometryVAO(6, line);

  if (!SetupUI()) {
    printf("Failed to setup UI.\n");
    return false;
  }

  return true;
}

void
SetProjectionMatrix(const math::Mat4f& projection)
{
  kRGG.projection  = projection;
}

void
SetViewMatrix(const math::Mat4f& view)
{
  kRGG.view = view;
}

void
SetCameraTransformMatrix(const math::Mat4f& camera_transform)
{
  kRGG.camera_transform = camera_transform;
}

void
RenderTriangle(const math::Vec3f& position, const math::Vec3f& scale,
               const math::Quatf& orientation, const math::Vec4f& color)
{
  glUseProgram(kRGG.geometry_program.reference);
  glBindVertexArray(kRGG.triangle_vao_reference);
   // Translate and rotate the triangle appropriately.
  math::Mat4f model = math::CreateTranslationMatrix(position) *
                      math::CreateScaleMatrix(scale) *
                      math::CreateRotationMatrix(orientation);
  math::Mat4f matrix = kRGG.projection * kRGG.view * model;
  glUniform4f(kRGG.geometry_program.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.geometry_program.matrix_uniform, 1, GL_FALSE,
                     &matrix[0]);
  glDrawArrays(GL_LINE_LOOP, 0, 3);
}

void
RenderRectangle(const math::Vec3f& position, const math::Vec3f& scale,
                const math::Quatf& orientation, const math::Vec4f& color)
{
  glUseProgram(kRGG.geometry_program.reference);
  glBindVertexArray(kRGG.rectangle_vao_reference);
  // Translate and rotate the rectangle appropriately.
  math::Mat4f model = math::CreateTranslationMatrix(position) *
                      math::CreateScaleMatrix(scale) *
                      math::CreateRotationMatrix(orientation);
  math::Mat4f matrix = kRGG.projection * kRGG.view * model;
  glUniform4f(kRGG.geometry_program.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.geometry_program.matrix_uniform, 1, GL_FALSE,
                     &matrix[0]);
  // This is using the first 4 verts given by square[18].
  glDrawArrays(GL_LINE_LOOP, 0, 4);
}


void
RenderCircle(const math::Vec3f& position, const math::Vec3f& scale,
             const math::Quatf& orientation, const math::Vec4f& color)
{
  glUseProgram(kRGG.circle_program.reference);
  glBindVertexArray(kRGG.rectangle_vao_reference);
  // Translate and rotate the circle appropriately.
  math::Mat4f model = math::CreateTranslationMatrix(position) *
                      math::CreateScaleMatrix(scale) *
                      math::CreateRotationMatrix(orientation);
  math::Mat4f view_pojection = kRGG.projection * kRGG.view;
  glUniform4f(kRGG.circle_program.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.circle_program.model_uniform, 1, GL_FALSE,
                     &model[0]);
  glUniformMatrix4fv(kRGG.circle_program.view_projection_uniform, 1, GL_FALSE,
                     &view_pojection[0]);
  glDrawArrays(GL_TRIANGLES, 0, 6);

}

math::Mat4f
CreateLineTransform(const math::Vec3f& start, const math::Vec3f& end)
{
  // Line is the length of a meter on the horizontal axis.
  //
  // It must be translated / rotated / scaled to be properly moved
  // to meet the start / end nature of the line component.

  // Position is the midpoint of the start and end.
  math::Vec3f translation = (start + end) / 2.f;
  // Angle between the two points in 2d.
  math::Vec3f diff = end - start;
  float angle = atan2(diff.y, diff.x) * (180.f / PI);
  float distance = math::Length(diff);
  return math::CreateTranslationMatrix(translation) *
         math::CreateScaleMatrix(
             math::Vec3f(distance / 2.f, distance / 2.f, 1.f)) *
         math::CreateRotationMatrix(
             math::Quatf(angle, math::Vec3f(0.f, 0.f, -1.f)));
}

void
RenderLine(const math::Vec3f& start, const math::Vec3f& end,
           const math::Vec4f& color)
{
  glUseProgram(kRGG.geometry_program.reference);
  glBindVertexArray(kRGG.line_vao_reference);
  math::Mat4f matrix = kRGG.projection * kRGG.view *
                       CreateLineTransform(start, end);
  glUniform4f(kRGG.geometry_program.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.geometry_program.matrix_uniform, 1, GL_FALSE,
                     &matrix[0]);
  glDrawArrays(GL_LINES, 0, 2);
}

void
RenderGrid(float width, float height, const math::Vec4f& color)
{
  glUseProgram(kRGG.geometry_program.reference);
  glBindVertexArray(kRGG.line_vao_reference);
  // The bottom left and top right of the screen with regards to the camera.
  auto dims = window::GetWindowSize();
  math::Vec3f top_right =
      kRGG.camera_transform * math::Vec3f(dims.x, dims.y, 0.f);
  math::Vec3f bottom_left =
      kRGG.camera_transform * math::Vec3f(-dims.x, -dims.y, 0.f);
  float right_most_grid_x =
        width - (fmod(top_right.x + width, width));
  float left_most_grid_x = -fmod(bottom_left.x + width, width);
  float top_most_grid_y =
      height - (fmod(top_right.y + height, height));
  float bottom_most_grid_y = -fmod(bottom_left.y + height, height);

  math::Vec3f top_right_transformed =
      top_right + math::Vec3f(right_most_grid_x, top_most_grid_y, 0.f);
  math::Vec3f bottom_left_transformed =
      bottom_left + math::Vec3f(left_most_grid_x, bottom_most_grid_y, 0.f);

  // Draw horizontal lines.
  for (float y = bottom_left_transformed.y; y < top_right.y;
       y += height) {
    auto start = math::Vec3f(bottom_left_transformed.x, y, 0.f);
    auto end = math::Vec3f(top_right_transformed.x, y, 0.f);
    RenderLine(start, end, color);
  }

  // Draw vertical lines.
  for (float x = bottom_left_transformed.x; x < top_right.x;
       x += width) {
    auto start = math::Vec3f(x, bottom_left_transformed.y, 0.f);
    auto end = math::Vec3f(x, top_right_transformed.y, 0.f);
    RenderLine(start, end, color);
  }
}

}
