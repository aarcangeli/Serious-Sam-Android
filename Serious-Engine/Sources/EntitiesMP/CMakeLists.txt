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

  NavigationMarker
  Reminder
  Spinner
  TacticsChanger
  TacticsHolder
  Watcher
  WatchPlayers
  Bouncer
  DestroyableArchitecture
  MovingBrush
  MovingBrushMarker
  Pendulum
  Ship
  ShipMarker
  Terrain
  WorldBase
  BasicEffects
  BlendController
  BloodSpray
  Debris
  EffectMarker
  Effector
  Fireworks
  Lightning
  MeteorShower
  PhotoAlbum
  PyramidSpaceShip
  PyramidSpaceShipMarker
  RollingStone
  StormController
  TimeController
  WorldSettingsController
  AirElemental
  Beast
  BigHead
  Boneman
  CannonRotating
  CannonStatic
  ChainsawFreak
  CrateBus
  CrateRider
  Demon
  Devil
  DevilMarker
  Elemental
  EnemyBase
  EnemyCounter
  EnemyDive
  EnemyFly
  EnemyMarker
  EnemyRunInto
  EnemySpawner
  ExotechLarva
  ExotechLarvaBattery
  ExotechLarvaCharger
  Eyeman
  Fish
  Gizmo
  Grunt
  GruntSka
  Guffy
  Headman
  Santa
  Scorpman
  Shooter
  Summoner
  SummonerMarker
  Walker
  Werebull
  Woman
  EnvironmentBase
  EnvironmentMarker
  EnvironmentParticlesHolder
  AmmoItem
  AmmoPack
  ArmorItem
  HealthItem
  Item
  KeyItem
  MessageItem
  PowerUpItem
  WeaponItem
  Player
  PlayerActionMarker
  PlayerAnimator
  PlayerMarker
  PlayerView
  PlayerWeapons
  PlayerWeaponsEffects
  AnimationChanger
  AnimationHub
  AreaMarker
  BackgroundViewer
  Camera
  CameraMarker
  Copier
  Counter
  CreditsHolder
  Damager
  DebugEntityStatesDisplay
  DoorController
  Eruptor
  FogMarker
  GradientMarker
  GravityMarker
  GravityRouter
  HazeMarker
  HudPicHolder
  Light
  Marker
  MessageHolder
  MirrorMarker
  ModelDestruction
  ModelHolder
  ModelHolder2
  ModelHolder3
  MusicChanger
  MusicHolder
  ParticlesHolder
  ScrollHolder
  SoundHolder
  Switch
  Teleport
  TextFXHolder
  TouchField
  Trigger
  VoiceHolder
  WorldLink
  AirShockwave
  Bullet
  CannonBall
  DevilProjectile
  Flame
  LarvaOffspring
  Projectile
  SeriousBomb
  SpawnerProjectile
  Twister
  Water
  Global
)

message("EntitiesMP srcs: ${SOURCES}")

add_library(EntitiesMP SHARED ${SOURCES})
target_compile_options(EntitiesMP PUBLIC "-I${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(EntitiesMP PRIVATE "-I${CMAKE_CURRENT_SOURCE_DIR}/StdH")
target_compile_options(EntitiesMP PUBLIC "-I${SE_CURRENT_GENERATED_DIR}")
target_compile_options(EntitiesMP PRIVATE "-I${SE_CURRENT_GENERATED_DIR}/StdH")

target_link_libraries(EntitiesMP Engine)
target_compile_options(EntitiesMP PUBLIC -Wpedantic)
