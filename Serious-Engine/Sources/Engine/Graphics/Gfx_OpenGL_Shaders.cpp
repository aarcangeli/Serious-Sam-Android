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
}

SLONG gfxGetAttribLocation(GfxProgram _program, const char *name) {
  GfxProgramPrivate *pgm = (GfxProgramPrivate *) _program;
  GLint result = glGetAttribLocation(pgm->pgmObject, name);
  OGL_CHECKERROR;
  return result;
}

SLONG gfxGetUniformLocation(GfxProgram _program, const char *name) {
  GfxProgramPrivate *pgm = (GfxProgramPrivate *) _program;
  GLint result = glGetUniformLocation(pgm->pgmObject, name);
  OGL_CHECKERROR;
  return result;
}

void gfxSyncProgram() {
  GfxProgramPrivate *pgm = _currentProgram;
  ASSERT(pgm);

  // update buffers
  glUniformMatrix4fv(pgm->projMatIdx, 1, GL_FALSE, gles_adapter::getProjMat());
  glUniformMatrix4fv(pgm->modelViewMatIdx, 1, GL_FALSE, gles_adapter::getModelViewMat());
  glUniform1f(pgm->enableTextureLoc, gles_adapter::isTexture2d() ? 1 : 0);
  glUniform1f(pgm->enableAlphaTestLoc, gles_adapter::isAlphaTest() ? 1 : 0);
  OGL_CHECKERROR;
}
