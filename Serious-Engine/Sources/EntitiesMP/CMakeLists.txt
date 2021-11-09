include(generatedFiles)

set(SE_BASE "${CMAKE_CURRENT_SOURCE_DIR}")
set(SE_CURRENT_GENERATED_DIR "${SE_GENERATED_DIR}/EntitiesMP")

file(MAKE_DIRECTORY "${SE_CURRENT_GENERATED_DIR}/StdH")

set(
  SOURCES
  Common/Common.cpp
  Common/EmanatingParticles.cpp
  Common/HUD.cpp
  Common/Particles.cpp
  Common/PathFinding.cpp
  StdH/StdH.cpp
)

run_ecc(
  SOURCES

  AirElemental
  AirShockwave
  AmmoItem
  AmmoPack
  AnimationChanger
  AnimationHub
  AreaMarker
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
  CannonRotating
  CannonStatic
  ChainsawFreak
  Copier
  Counter
  CrateBus
  CrateRider
  CreditsHolder
  Damager
  Debris
  DebugEntityStatesDisplay
  Demon
  DestroyableArchitecture
  Devil
  DevilMarker
  DevilProjectile
  DoorController
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
  EnvironmentParticlesHolder
  Eruptor
  ExotechLarva
  ExotechLarvaBattery
  ExotechLarvaCharger
  Eyeman
  Fireworks
  Fish
  Flame
  FogMarker
  Gizmo
  Global
  GradientMarker
  GravityMarker
  GravityRouter
  Grunt
  GruntSka
  Guffy
  HazeMarker
  Headman
  HealthItem
  HudPicHolder
  Item
  KeyItem
  LarvaOffspring
  Light
  Lightning
  Marker
  MessageHolder
  MessageItem
  MeteorShower
  MirrorMarker
  ModelDestruction
  ModelHolder
  ModelHolder2
  ModelHolder3
  MovingBrush
  MovingBrushMarker
  MusicChanger
  MusicHolder
  NavigationMarker
  ParticlesHolder
  Pendulum
  PhotoAlbum
  Player
  PlayerActionMarker
  PlayerAnimator
  PlayerMarker
  PlayerView
  PlayerWeapons
  PlayerWeaponsEffects
  PowerUpItem
  Projectile
  PyramidSpaceShip
  PyramidSpaceShipMarker
  Reminder
  RollingStone
  Santa
  Scorpman
  ScrollHolder
  SeriousBomb
  Ship
  ShipMarker
  Shooter
  SoundHolder
  SpawnerProjectile
  Spinner
  StormController
  Summoner
  SummonerMarker
  Switch
  TacticsChanger
  TacticsHolder
  Teleport
  Terrain
  TextFXHolder
  TimeController
  TouchField
  Trigger
  Twister
  VoiceHolder
  Walker
  WatchPlayers
  Watcher
  Water
  WeaponItem
  Werebull
  Woman
  WorldBase
  WorldLink
  WorldSettingsController
)

if (STATIC_LINKING)
  add_library(EntitiesMP STATIC ${SOURCES})
else ()
  add_library(EntitiesMP SHARED ${SOURCES})
endif ()
target_compile_options(EntitiesMP PUBLIC "-I${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(EntitiesMP PRIVATE "-I${CMAKE_CURRENT_SOURCE_DIR}/StdH")
target_compile_options(EntitiesMP PUBLIC "-I${SE_CURRENT_GENERATED_DIR}")
target_compile_options(EntitiesMP PRIVATE "-I${SE_CURRENT_GENERATED_DIR}/StdH")

target_link_libraries(EntitiesMP Engine)
#target_compile_options(EntitiesMP PUBLIC -Wpedantic)
