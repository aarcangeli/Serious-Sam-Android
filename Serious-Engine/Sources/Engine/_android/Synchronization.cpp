#include <Engine/StdH.h>
#include <Engine/Base/Synchronization.h>

// disable temporally mutex

CTCriticalSection::CTCriticalSection(void) {
}

CTCriticalSection::~CTCriticalSection(void) {
}

INDEX CTCriticalSection::Lock(void) {
    return 1;
}

INDEX CTCriticalSection::TryToLock(void) {
    return true;
}

INDEX CTCriticalSection::Unlock(void) {
    return true;
}

CTSingleLock::CTSingleLock(CTCriticalSection *pcs, BOOL bLock) : sl_cs(*pcs) {
}

CTSingleLock::~CTSingleLock(void) {
}

void CTSingleLock::Lock(void) {
}

BOOL CTSingleLock::TryToLock(void) {
    return true;
}

BOOL CTSingleLock::IsLocked(void) {
    return true;
}

void CTSingleLock::Unlock(void) {
}
