cmake_minimum_required(VERSION 3.16)
project(TowerDefense C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4)
FetchContent_Declare(glm  GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 1.0.1)
FetchContent_MakeAvailable(glfw glm)

file(GLOB SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_executable(game ${SOURCES} libs/glad/src/glad.c)
target_include_directories(game PRIVATE libs/glad/include src)
target_link_libraries(game PRIVATE glfw glm::glm)
