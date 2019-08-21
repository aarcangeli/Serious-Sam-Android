#include <Engine/StdH.h>
#include <Engine/Base/ErrorReporting.h>

#include <GLES2/gl2.h>
#include <AndroidAdapters/gles_adapter.h>
#include <EGL/egl.h>

#define GL_DEBUG_TYPE_ERROR 0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR 0x824E
#define GL_DEBUG_TYPE_PORTABILITY 0x824F
#define GL_DEBUG_TYPE_PERFORMANCE 0x8250
#define GL_DEBUG_TYPE_OTHER 0x8251
#define GL_DEBUG_TYPE_MARKER 0x8268
#define GL_DEBUG_SEVERITY_HIGH 0x9146
#define GL_DEBUG_SEVERITY_MEDIUM 0x9147
#define GL_DEBUG_SEVERITY_LOW 0x9148
#define GL_DEBUG_OUTPUT 0x92E0
#define GL_DEBUG_SEVERITY_NOTIFICATION 0x826B

namespace gles_adapter {
  void glMessageCallback(GLenum source,
                         GLenum type,
                         GLuint id,
                         GLenum severity,
                         GLsizei length,
                         const GLchar *message,
                         const void *userParam) {
    const char *typeDesc, *severityDesc;
    switch (type) {
      case GL_DEBUG_TYPE_ERROR:
        typeDesc = "ERROR";
        break;
      case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        typeDesc = "DEPRECATED_BEHAVIOR";
        break;
      case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        typeDesc = "UNDEFINED_BEHAVIOR";
        break;
      case GL_DEBUG_TYPE_PORTABILITY:
        typeDesc = "PORTABILITY";
        break;
      case GL_DEBUG_TYPE_PERFORMANCE:
        typeDesc = "PERFORMANCE";
        break;
      case GL_DEBUG_TYPE_OTHER:
        typeDesc = "OTHER";
        break;
      case GL_DEBUG_TYPE_MARKER:
        typeDesc = "MARKER";
        break;
      default:
        typeDesc = "LOG";
        break;
    }
    switch (severity) {
      case GL_DEBUG_SEVERITY_HIGH:
        severityDesc = "HIGH";
        break;
      case GL_DEBUG_SEVERITY_MEDIUM:
        severityDesc = "MEDIUM";
        break;
      case GL_DEBUG_SEVERITY_LOW:
        severityDesc = "LOW";
        break;
      case GL_DEBUG_SEVERITY_NOTIFICATION:
        severityDesc = "NOTIFICATION";
        break;
      default:
        severityDesc = "LOG";
    }

    if (severity == GL_DEBUG_SEVERITY_HIGH) {
      WarningMessage("OPENGL %s (%s): %s\n", typeDesc, severityDesc, message);
    } else {
      InfoMessage("OPENGL %s (%s): %s\n", typeDesc, severityDesc, message);
    }
  }

  typedef void (*GLDEBUGPROC)(GLenum source, GLenum type, GLuint id, GLenum severity,
                              GLsizei length, const GLchar *message, const void *userParam);

  typedef void (*PFNGLDEBUGMESSAGECALLBACKPROC)(GLDEBUGPROC callback, const void *userParam);

  void glMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                         const GLchar *message, const void *userParam);

  void installGlLogger() {
    PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback =
      (PFNGLDEBUGMESSAGECALLBACKPROC) eglGetProcAddress("glDebugMessageCallback");
    if (glDebugMessageCallback) {
      glDebugMessageCallback(&glMessageCallback, nullptr);
    }
  }
}