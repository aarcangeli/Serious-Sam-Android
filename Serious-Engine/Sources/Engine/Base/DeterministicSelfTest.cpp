#include <Engine/StdH.h>
#include <Engine/Base/Console.h>
#include <Engine/Base/CRC.h>
#include <Engine/Base/DeterministicFP.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

struct XorShift128Plus {
    uint64_t s0=0x243F6A8885A308D3ull, s1=0x13198A2E03707344ull;
    inline uint64_t next(){ uint64_t x=s0,y=s1; s0=y; x^=x<<23; y^=y>>26; x^=x>>17; s1=x^y; return s1 + y; }
    inline float nextF(){ return (float)((next()>>40) * (1.0/16777216.0)); }
};

static inline void CRC_AddF(ULONG &crc, float f) { CRC_AddFLOAT(crc, f); }

void Deterministic_RunSelfTest() {
    SetDeterministicFP();

    ULONG crc_libm; CRC_Start(crc_libm);
    float a=1.0f, b=0.5f, r=0.0f;
    for (int i=0;i<100000;++i) {
        a = sinf(a + 0.1234567f) + 0.5f * cosf(b + 0.7654321f);
        b = sqrtf(fabsf(a) + 1e-6f);
        r = a*0.75f + b*0.25f;
        CRC_AddF(crc_libm,a); CRC_AddF(crc_libm,b); CRC_AddF(crc_libm,r);
    }
    CRC_Finish(crc_libm);

    ULONG crc_rng; CRC_Start(crc_rng);
    XorShift128Plus rng;
    for (int i=0;i<200000;++i) { float v = rng.nextF(); CRC_AddF(crc_rng, v); }
    CRC_Finish(crc_rng);

    auto snap12 = [](float v)->float { return ldexpf(roundf(v*4096.0f), -12); };
    ULONG crc_sim; CRC_Start(crc_sim);
    float px=0.1f, py=0.2f, pz=-0.3f;
    float vx=0.01f, vy=0.02f, vz=-0.04f;
    for (int t=0;t<60000;++t){
        float ax = sinf(px*0.7f)*0.001f;
        float ay = cosf(py*0.5f)*0.001f;
        float az = sinf(pz*0.3f)*0.001f;
        vx = snap12(vx + ax); vy = snap12(vy + ay); vz = snap12(vz + az);
        px = snap12(px + vx); py = snap12(py + vy); pz = snap12(pz + vz);
        float nx=px, ny=py, nz=pz;
        float invLen = 1.0f / fmaxf(1e-6f, sqrtf(nx*nx + ny*ny + nz*nz));
        nx = snap12(nx*invLen); ny = snap12(ny*invLen); nz = snap12(nz*invLen);
        CRC_AddF(crc_sim,px); CRC_AddF(crc_sim,py); CRC_AddF(crc_sim,pz);
        CRC_AddF(crc_sim,nx); CRC_AddF(crc_sim,ny); CRC_AddF(crc_sim,nz);
    }
    CRC_Finish(crc_sim);

    CPrintF("[DeterministicFP] SelfTest (Engine CRC): LIBM=0x%08X RNG=0x%08X SIM=0x%08X\n",
            (unsigned)crc_libm, (unsigned)crc_rng, (unsigned)crc_sim);
}