#include "background_renderer.h"

#define TAG "BackgroundRenderer"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

BackgroundRenderer::BackgroundRenderer() = default;

static GLfloat triangleCoords[] = {
    0.5f, 0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    -0.5f, 0.5f, 0.0f
};

static GLfloat color[] = {0.63671875f, 0.76953125f, 0.22265625f, 1.0f};

void BackgroundRenderer::init() {
  mDefaultProgram = DefaultShader::compile();
}

void BackgroundRenderer::drawFrame() {
  glUseProgram(mDefaultProgram);

  GLint location = glGetAttribLocation(mDefaultProgram, "vPosition");

  glEnableVertexAttribArray(location);
  glVertexAttribPointer(location, COORDS_PER_VERTEX,
                        GL_FLOAT, false,
                        VERTEX_STRIDE, &triangleCoords);

  GLint colorHandle = glGetUniformLocation(mDefaultProgram, "vColor");

  // Set color for drawing the triangle
  glUniform4fv(colorHandle, 1, color);

  // Draw the triangle
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glDisableVertexAttribArray(location);
}
