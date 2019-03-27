#include "stdh.h"
#include <Engine/Graphics/GfxLibrary.h>
#include <Engine/Graphics/Gfx_Shaders.h>
#include <Engine/Math/Matrix.h>
#include <AndroidAdapters/gles_adapter.h>

class GfxProgramPrivate {
public:
  GfxProgramPrivate(const char *vertexShader, const char *fragmentShader)
    : vertexShader(vertexShader),
      fragmentShader(fragmentShader) {};

  GLuint pgmObject = 0;
  const char *vertexShader, *fragmentShader;
  // uniforms
  GLint projMat;
  GLint modelViewMat;
  GLint mainTexture;
  GLint enableTexture;
  GLint enableAlphaTest;
  GLint withReflectionMapping;
  GLint withSpecularMapping;
  GLint isUnlite;
  GLint stretch;
  GLint offset;
  GLint lerpRatio;
  GLint texCorr;
  GLint color;
  GLint lightObj;
  GLint colorAmbient;
  GLint colorLight;
  GLint viewer;
  GLint objectRotation;
  GLint objectToView;
};

GfxShadersUniforms gfxCurrentUniforms;

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
    pgm->projMat = glGetUniformLocation(pgm->pgmObject, "projMat");
    pgm->modelViewMat = glGetUniformLocation(pgm->pgmObject, "modelViewMat");
    pgm->mainTexture = glGetUniformLocation(pgm->pgmObject, "mainTexture");
    if (pgm->mainTexture >= 0) glUniform1i(pgm->mainTexture, 0);
    pgm->enableTexture = glGetUniformLocation(pgm->pgmObject, "enableTexture");
    pgm->enableAlphaTest = glGetUniformLocation(pgm->pgmObject, "enableAlphaTest");

    pgm->withReflectionMapping = glGetUniformLocation(pgm->pgmObject, "withReflectionMapping");
    pgm->withSpecularMapping = glGetUniformLocation(pgm->pgmObject, "withSpecularMapping");

    pgm->stretch = glGetUniformLocation(pgm->pgmObject, "stretch");
    pgm->offset = glGetUniformLocation(pgm->pgmObject, "offset");
    pgm->lerpRatio = glGetUniformLocation(pgm->pgmObject, "lerpRatio");
    pgm->texCorr = glGetUniformLocation(pgm->pgmObject, "texCorr");
    pgm->color = glGetUniformLocation(pgm->pgmObject, "color");
    pgm->lightObj = glGetUniformLocation(pgm->pgmObject, "lightObj");
    pgm->colorAmbient = glGetUniformLocation(pgm->pgmObject, "colorAmbient");
    pgm->colorLight = glGetUniformLocation(pgm->pgmObject, "colorLight");
    pgm->viewer = glGetUniformLocation(pgm->pgmObject, "viewer");
    pgm->isUnlite = glGetUniformLocation(pgm->pgmObject, "isUnlite");
    pgm->objectRotation = glGetUniformLocation(pgm->pgmObject, "objectRotation");
    pgm->objectToView = glGetUniformLocation(pgm->pgmObject, "objectToView");

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

void gfxUniform(const char *uniformName, const FLOATmatrix3D &matrix) {
  GfxProgramPrivate *pgm = _currentProgram;
  ASSERT(pgm);
  GLint uniformLocation = glGetUniformLocation(pgm->pgmObject, uniformName);
  if (uniformLocation >= 0) {
    glUniformMatrix3fv(uniformLocation, 1, GL_FALSE, (const float *) &matrix);
  }
  gles_adapter::syncError();
}

inline void gfxSetUniformValue(GLint uniformLocation, bool value) {
  glUniform1f(uniformLocation, value ? 1 : 0);
}

inline void gfxSetUniformValue(GLint uniformLocation, float value) {
  glUniform1f(uniformLocation, value);
}

inline void gfxSetUniformValue(GLint uniformLocation, FLOAT2D value) {
  glUniform2f(uniformLocation, value(1), value(2));
}

inline void gfxSetUniformValue(GLint uniformLocation, const FLOAT3D &value) {
  glUniform3f(uniformLocation, value(1), value(2), value(3));
}

inline void gfxSetUniformValue(GLint uniformLocation, GFXColor value) {
  glUniform4f(uniformLocation, value.r / 255.0f, value.g / 255.0f, value.b / 255.0f, value.a / 255.0f);
}

inline void gfxSetUniformValue(GLint uniformLocation, const FLOATmatrix3D &value) {
  glUniformMatrix3fv(uniformLocation, 1, GL_FALSE, (const float *) &value);
}

inline BOOL operator!=(const GFXColor &v1, const GFXColor &v2) {
  return v1.abgr != v2.abgr;
};

inline BOOL gfxEquals(const FLOATmatrix3D &v1, const FLOATmatrix3D &v2) {
  if (v1.matrix[0][0] != v1.matrix[0][0]) return false;
  if (v1.matrix[0][1] != v1.matrix[0][1]) return false;
  if (v1.matrix[0][2] != v1.matrix[0][2]) return false;
  if (v1.matrix[1][0] != v1.matrix[1][0]) return false;
  if (v1.matrix[1][1] != v1.matrix[1][1]) return false;
  if (v1.matrix[1][2] != v1.matrix[1][2]) return false;
  if (v1.matrix[2][0] != v1.matrix[2][0]) return false;
  if (v1.matrix[2][1] != v1.matrix[2][1]) return false;
  if (v1.matrix[2][2] != v1.matrix[2][2]) return false;
  return true;
};

void gfxSyncProgram(struct GfxShadersUniforms &params) {
  GfxProgramPrivate *pgm = _currentProgram;
  ASSERT(pgm);

  // update buffers
  glUniformMatrix4fv(pgm->projMat, 1, GL_FALSE, gles_adapter::getProjMat());
  glUniformMatrix4fv(pgm->modelViewMat, 1, GL_FALSE, gles_adapter::getModelViewMat());
  params.enableTexture = gles_adapter::isTexture2d();
  params.enableAlphaTest = gles_adapter::isAlphaTest();

#define SYNC_UNIFORM(P) \
  if (pgm->P >= 0 && gfxCurrentUniforms.P != params.P) { \
    gfxSetUniformValue(pgm->P, params.P); \
    gfxCurrentUniforms.P = params.P; \
  }

#define SYNC_UNIFORM_MAT(P) \
  if (pgm->P >= 0 && !gfxEquals(gfxCurrentUniforms.P, params.P)) { \
    gfxSetUniformValue(pgm->P, params.P); \
    gfxCurrentUniforms.P = params.P; \
  }

  SYNC_UNIFORM(enableTexture)
  SYNC_UNIFORM(enableAlphaTest)
  SYNC_UNIFORM(withReflectionMapping)
  SYNC_UNIFORM(withSpecularMapping)
  SYNC_UNIFORM(stretch)
  SYNC_UNIFORM(offset)
  SYNC_UNIFORM(lerpRatio)
  SYNC_UNIFORM(texCorr)
  SYNC_UNIFORM(color)
  SYNC_UNIFORM(lightObj)
  SYNC_UNIFORM(colorAmbient)
  SYNC_UNIFORM(colorLight)
  SYNC_UNIFORM(viewer)
  SYNC_UNIFORM(isUnlite)
  SYNC_UNIFORM_MAT(objectRotation)
  SYNC_UNIFORM_MAT(objectToView)

#undef SYNC_UNIFORM
#undef SYNC_UNIFORM_MAT

  gles_adapter::syncError();
}
