#include "stdh.h"
#include <Engine/Graphics/GfxLibrary.h>
#include <AndroidAdapters/gles_adapter.h>

class GfxProgramPrivate {
public:
  GfxProgramPrivate(const char *vertexShader, const char *fragmentShader)
    : vertexShader(vertexShader),
      fragmentShader(fragmentShader) {};

  GLuint pgmObject = 0;
  const char *vertexShader, *fragmentShader;
  GLint projMatIdx, modelViewMatIdx, mainTextureLoc, enableTextureLoc, enableAlphaTestLoc;
};

GfxProgramPrivate *_currentProgram = 0;

GfxProgram gfxMakeShaderProgram(const char *vertexShader, const char *fragmentShader) {
  return (GfxProgram) new GfxProgramPrivate(vertexShader, fragmentShader);
}

GLuint compileShader(GLenum type, const char *name, const char *source) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);

  // check status
  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char buffer[2001] = "";
    glGetShaderInfoLog(shader, 2000, nullptr, buffer);
    static char error[2500];
    sprintf(error, "Cannot compile %s: %s", name, buffer);
    throw error;
  }

  return shader;
}

void gfxUseProgram(GfxProgram _program) {
  GfxProgramPrivate *pgm = (GfxProgramPrivate *) _program;
  _currentProgram = pgm;
  if (!pgm) {
    glUseProgram(0);
    return;
  }

  if (!pgm->pgmObject) {
    pgm->pgmObject = glCreateProgram();
    glAttachShader(pgm->pgmObject, compileShader(GL_VERTEX_SHADER, "vertex shader", pgm->vertexShader));
    glAttachShader(pgm->pgmObject, compileShader(GL_FRAGMENT_SHADER, "fragment shader", pgm->fragmentShader));
    glLinkProgram(pgm->pgmObject);
    glUseProgram(pgm->pgmObject);

    // get uniforms
    pgm->projMatIdx = glGetUniformLocation(pgm->pgmObject, "projMat");
    pgm->modelViewMatIdx = glGetUniformLocation(pgm->pgmObject, "modelViewMat");
    pgm->mainTextureLoc = glGetUniformLocation(pgm->pgmObject, "mainTexture");
    if (pgm->mainTextureLoc >= 0) glUniform1i(pgm->mainTextureLoc, 0);
    pgm->enableTextureLoc = glGetUniformLocation(pgm->pgmObject, "enableTexture");
    pgm->enableAlphaTestLoc = glGetUniformLocation(pgm->pgmObject, "enableAlphaTest");
  } else {
    glUseProgram(pgm->pgmObject);
  }

  gles_adapter::syncError();
}

SLONG gfxGetAttribLocation(GfxProgram _program, const char *name) {
  GfxProgramPrivate *pgm = (GfxProgramPrivate *) _program;
  GLint result = glGetAttribLocation(pgm->pgmObject, name);
  gles_adapter::syncError();
  return result;
}

void gfxUniform(const char *uniformName, float f0) {
  GfxProgramPrivate *pgm = _currentProgram;
  ASSERT(pgm);
  GLint uniformLocation = glGetUniformLocation(pgm->pgmObject, uniformName);
  if (uniformLocation >= 0) {
    glUniform1f(uniformLocation, f0);
  }
  gles_adapter::syncError();
}

void gfxUniform(const char *uniformName, float f0, float f1) {
  GfxProgramPrivate *pgm = _currentProgram;
  ASSERT(pgm);
  GLint uniformLocation = glGetUniformLocation(pgm->pgmObject, uniformName);
  if (uniformLocation >= 0) {
    glUniform2f(uniformLocation, f0, f1);
  }
  gles_adapter::syncError();
}

void gfxUniform(const char *uniformName, float f0, float f1, float f2) {
  GfxProgramPrivate *pgm = _currentProgram;
  ASSERT(pgm);
  GLint uniformLocation = glGetUniformLocation(pgm->pgmObject, uniformName);
  if (uniformLocation >= 0) {
    glUniform3f(uniformLocation, f0, f1, f2);
  }
  gles_adapter::syncError();
}

void gfxUniform(const char *uniformName, float f0, float f1, float f2, float f3) {
  GfxProgramPrivate *pgm = _currentProgram;
  ASSERT(pgm);
  GLint uniformLocation = glGetUniformLocation(pgm->pgmObject, uniformName);
  if (uniformLocation >= 0) {
    glUniform4f(uniformLocation, f0, f1, f2, f3);
  }
  gles_adapter::syncError();
}

void gfxUniform(const char *uniformName, GFXColor &color) {
  gfxUniform(uniformName, color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
}

void gfxSyncProgram() {
  GfxProgramPrivate *pgm = _currentProgram;
  ASSERT(pgm);

  // update buffers
  glUniformMatrix4fv(pgm->projMatIdx, 1, GL_FALSE, gles_adapter::getProjMat());
  glUniformMatrix4fv(pgm->modelViewMatIdx, 1, GL_FALSE, gles_adapter::getModelViewMat());
  glUniform1f(pgm->enableTextureLoc, gles_adapter::isTexture2d() ? 1 : 0);
  glUniform1f(pgm->enableAlphaTestLoc, gles_adapter::isAlphaTest() ? 1 : 0);
  gles_adapter::syncError();
}
