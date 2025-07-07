cmake_minimum_required(VERSION 3.26)
# Configurando o padrão c++
set(CMAKE_CXX_STANDARD 17)

# Configurando o nome e executável do projeto
set(PROJECT_NAME "Ichirei-Shikon")
project(${PROJECT_NAME})

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)

add_executable(${PROJECT_NAME}
        Source/Main.cpp
        Source/GameMath.cpp
        Source/Random.cpp
        Source/Actors/Actor.cpp
        Source/Components/Component.cpp
        Source/Game.cpp
        Source/Components/DrawComponents/DrawComponent.cpp
        Source/Components/RigidBodyComponent.cpp
        Source/Components/ColliderComponents/CircleColliderComponent.cpp
        Source/Components/DrawComponents/DrawSpriteComponent.cpp
        Source/Components/DrawComponents/DrawPolygonComponent.cpp
        Source/Actors/Player.cpp
        Source/Components/ColliderComponents/AABBColliderComponent.cpp
        Source/Components/DrawComponents/DrawAnimatedComponent.cpp
        Source/Actors/Block.cpp
        Source/Actors/Ghost.cpp
        Source/Actors/Spawner.cpp
        Source/Actors/FlyingDemon.cpp
        Source/Actors/Particle.cpp
        Source/AudioSystem.cpp
        Source/UIElements/UIScreen.cpp
        Source/UIElements/UIFont.cpp
        Source/UIElements/UIButton.cpp
        Source/UIElements/UIText.cpp
        Source/UIElements/UIElement.cpp
        Source/UIElements/UIHud.cpp
        Source/UIElements/UIImage.cpp
        Source/SpatialHashing.cpp
        Source/Actors/DemonBoss.cpp
        Source/Actors/Slash.cpp
        Source/Actors/Trigger.cpp
        Source/Actors/Trigger.h
        Source/UIElements/UIRect.cpp
        Source/UIElements/UIRect.h
        Source/UIElements/UITimerBar.cpp
        Source/UIElements/UITimerBar.h
        Source/Actors/BlockSpriteSheet.cpp
        Source/Actors/BlockSpriteSheet.h
        Source/Actors/InvisibleBlock.cpp
        Source/Actors/InvisibleBlock.h
        Source/PerlinNoise.h
        Source/PerlinNoise.cpp
)

# Inclui os diretórios de cabeçalho para que o compilador os encontre
target_include_directories(${PROJECT_NAME} PRIVATE
        Source
        Source/Actors
        Source/Components
        Source/Components/ColliderComponents
        Source/Components/DrawComponents
        Source/UIElements
)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2_image::SDL2_image SDL2_ttf::SDL2_ttf SDL2_mixer::SDL2_mixer)