
#include "StdH.h"
#include <Engine/Base/Synchronization_win32_fallback.h>
#include <pthread.h>
#include <stdio.h>

struct HandlePrivate {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    BOOL state;
};

LONG InterlockedIncrement(LONG *Addend) {
  return __sync_add_and_fetch(Addend, 1);
}

LONG InterlockedDecrement(LONG volatile *Addend) {
  return __sync_sub_and_fetch(Addend, 1);
}

DWORD GetCurrentThreadId() {
  static_assert(sizeof(pthread_t) == sizeof(DWORD), "");
  return (DWORD) pthread_self();
}

HANDLE CreateEvent(void *attr, BOOL bManualReset, BOOL initial, LPCSTR lpName) {
  ASSERT(!bManualReset);
  HandlePrivate *handle = new HandlePrivate;

  pthread_mutex_init(&handle->mutex, nullptr);
  pthread_cond_init(&handle->cond, nullptr);
  handle->state = initial;

  return (HANDLE) handle;
}

BOOL CloseHandle(HANDLE hObject) {
  delete (HandlePrivate *) hObject;
  return true;
}

DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds) {
  ASSERT(dwMilliseconds == INFINITE);
  HandlePrivate *handle = (HandlePrivate *) hHandle;

  pthread_mutex_lock(&handle->mutex);
  while (!handle->state) {
    pthread_cond_wait(&handle->cond, &handle->mutex);
  }
  handle->state = false;
  pthread_mutex_unlock(&handle->mutex);
  return WAIT_OBJECT_0;
}

BOOL SetEvent(HANDLE hEvent) {
  HandlePrivate *handle = (HandlePrivate *) hEvent;

  pthread_mutex_lock(&handle->mutex);
  handle->state = true;
  pthread_cond_signal(&handle->cond);
  pthread_mutex_unlock(&handle->mutex);

  return true;
}

BOOL ResetEvent(HANDLE hEvent) {
  HandlePrivate *handle = (HandlePrivate *) hEvent;

  pthread_mutex_lock(&handle->mutex);
  handle->state = false;
  pthread_mutex_unlock(&handle->mutex);

  return true;
}
