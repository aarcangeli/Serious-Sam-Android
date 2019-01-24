#include <jni.h>
#include <string>
#include <GLES2/gl2.h>
#include <android/log.h>

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aarcangeli_serioussamandroid_MainActivity_init(JNIEnv *env, jobject instance) {
  glClearColor(1, 0, 0, 0);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aarcangeli_serioussamandroid_MainActivity_resize(JNIEnv *env, jobject instance,
                                                                 jint width, jint height) {
  glViewport(0, 0, width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aarcangeli_serioussamandroid_MainActivity_draw(JNIEnv *env, jobject instance) {
  glClear(GL_COLOR_BUFFER_BIT);
}
