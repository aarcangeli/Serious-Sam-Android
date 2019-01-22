#include <Engine/StdH.h>
#include <Engine/Base/CTString.h>
#include <Engine/Base/FileName.h>
#include <Engine/Base/Registry.h>
#include <Engine/Base/ErrorReporting.h>

ENGINE_API BOOL REG_GetString(const CTString &strKey, CTString &strString) {
    FatalError("REG_GetString('%s%')", strKey.str_String);
}

ENGINE_API BOOL REG_GetLong(const CTString &strKey, ULONG &ulLong) {
    FatalError("REG_GetLong('%s%')", strKey.str_String);
}

ENGINE_API BOOL REG_SetString(const CTString &strKey, const CTString &strString) {
    FatalError("REG_SetString('%s%', '%s')", strKey.str_String, strString.str_String);
}
