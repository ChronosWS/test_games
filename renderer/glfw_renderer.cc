#include "glfw_renderer.h"

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shape_verts.h"

namespace renderer {

bool GLFWRenderer::Start(int window_width, int window_height,
                     const std::string& window_title) {
  if (!glfwInit()) {
    std::cerr << "Cound not start GLFW3" << std::endl;
    return false;
  }
  // Only for mac I need this?
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow* window = glfwCreateWindow(
      640, 480, "Hello Triangle", NULL, NULL);
  if (!window) {
    std::cout << "Failed to open window with GLFW3" << std::endl;
    return false;
  }
  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
    std::cout << "Failed to initialize OpenGL context" << std::endl;
    return false;
  }
  const GLubyte* renderer = glGetString(GL_RENDERER);
  const GLubyte* version = glGetString(GL_VERSION);
  std::cout << renderer << std::endl;
  std::cout << version << std::endl;
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  return true;
}

bool GLFWRenderer::Setup() {
  // Create VBO.
  GLuint vbo = 0;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), kTriangleVerts,
               GL_STATIC_DRAW);
  // Create VAO bind vbo to it.
  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  return true;
}

void GLFWRenderer::Draw(const component::LineComponent& component) {
  std::cout << "Draw Line." << std::endl;
}

void GLFWRenderer::Draw(const component::TriangleComponent& component) {
  std::cout << "Draw Triangle." << std::endl;
}

}  // renderer