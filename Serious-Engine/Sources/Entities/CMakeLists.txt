include(generatedFiles)

set(SE_BASE "${CMAKE_CURRENT_SOURCE_DIR}")
set(SE_CURRENT_GENERATED_DIR "${SE_GENERATED_DIR}/Entities")

file(MAKE_DIRECTORY "${SE_CURRENT_GENERATED_DIR}/StdH")

set(
  SOURCES
  Common/Common.cpp
  Common/HUD.cpp
  Common/Particles.cpp
  Common/PathFinding.cpp
  StdH/StdH.cpp
)

run_ecc(
  SOURCES

  Acid  
  AirWave  
  AmmoItem  
  AmmoPack  
  AnimationChanger  
  AnimationHub  
  ArmorItem  
  BackgroundViewer  
  BasicEffects  
  Beast  
  BigHead  
  BlendController  
  BloodSpray  
  Boneman  
  Bouncer  
  Bullet  
  Camera  
  CameraMarker  
  CannonBall  
  Catman  
  Copier  
  Counter  
  CrateRider  
  CyborgBike  
  Cyborg  
  Damager  
  Debris  
  DestroyableArchitecture  
  Devil  
  DevilMarker  
  DevilProjectile  
  DoorController  
  Dragonman  
  EffectMarker  
  Effector  
  Elemental  
  EnemyBase  
  EnemyCounter  
  EnemyDive  
  EnemyFly  
  EnemyMarker  
  EnemyRunInto  
  EnemySpawner  
  EnvironmentBase  
  EnvironmentMarker  
  Eruptor  
  Eyeman  
  Fish  
  Fishman  
  Flame  
  FogMarker  
  Gizmo  
  Global  
  GradientMarker  
  GravityMarker  
  GravityRouter  
  HazeMarker  
  Headman  
  HealthItem  
  Huanman  
  Item  
  KeyItem  
  Light  
  Lightning  
  LightStyle  
  Mamut  
  Mamutman  
  Mantaman  
  Marker  
  MessageHolder  
  MessageItem  
  MirrorMarker  
  ModelDestruction  
  ModelHolder2  
  ModelHolder  
  MovingBrush  
  MovingBrushMarker  
  MusicChanger  
  MusicHolder  
  NavigationMarker  
  ParticlesHolder  
  Pendulum  
  Pipebomb  
  PlayerActionMarker  
  PlayerAnimator  
  Player  
  PlayerMarker  
  PlayerView  
  PlayerWeaponsEffects  
  PlayerWeapons  
  Projectile  
  PyramidSpaceShip  
  PyramidSpaceShipMarker  
  Reminder  
  RobotDriving  
  RobotFixed  
  RobotFlying  
  RollingStone  
  Scorpman  
  Ship  
  ShipMarker  
  SoundHolder  
  StormController  
  Switch  
  Teleport  
  TouchField  
  Trigger  
  Twister  
  VoiceHolder  
  Walker  
  Watcher  
  WatchPlayers  
  Water  
  WeaponItem  
  Werebull  
  Woman  
  WorldBase  
  WorldLink  
  WorldSettingsController  
)

if (STATIC_LINKING)
  add_library(Entities STATIC ${SOURCES})
else ()
  add_library(Entities SHARED ${SOURCES})
endif ()
target_compile_options(Entities PUBLIC "-I${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(Entities PRIVATE "-I${CMAKE_CURRENT_SOURCE_DIR}/StdH")
target_compile_options(Entities PUBLIC "-I${SE_CURRENT_GENERATED_DIR}")
target_compile_options(Entities PRIVATE "-I${SE_CURRENT_GENERATED_DIR}/StdH")

target_link_libraries(Entities Engine)
target_compile_options(Entities PUBLIC -Wpedantic)
