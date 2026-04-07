# Dedicated Server Debian 12 (amd64)

## 1) Dependencies

```bash
sudo apt update
sudo apt install -y \
  build-essential \
  cmake \
  ninja-build \
  pkg-config \
  libegl1-mesa-dev \
  libgles2-mesa-dev \
  libglm-dev \
  flex \
  bison
```


## 2) Configure and build (amd64)

```bash
cmake -S . -B build-debian \
  -G Ninja \
  -DSTATIC_LINKING=ON \
  -DFIRST_ENCOUNTER=OFF \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build-debian --target DedicatedServerAndroid -j"$(nproc)"
```

Binary:
- `build-debian/Serious-Engine/Sources/DedicatedServer/DedicatedServer`

## 3) Prepare for start

Needed all (`.gro`, `Levels/`, `Scripts/`, `Data/`).

For TSE:
- `SE1_00.gro`
- `SE1_00_Extra.gro`
- `SE1_00_ExtraTools.gro`
- `SE1_00_Levels.gro`
- `SE1_00_Logo.gro`
- `SE1_00_Music.gro`
- `1_04_patch.gro`
- `1_07_tools.gro`
- `Data/DedicatedServer.gms`
- `Scripts/Dedicated/*`

Easy start:
- unpack `DedicatedServerPrebuilt.zip` (without arm binary),
- copy `DedicatedServer DefaultCoop`


## 4) Run

Run in gamefolder:

```bash
./DedicatedServer DefaultCoop
```

Args:
- `DefaultCoop` — profile in `Scripts/Dedicated/<config-name>/`
