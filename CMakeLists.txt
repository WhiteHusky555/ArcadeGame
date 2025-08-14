cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

project(fltk-test-app)


# Specify the source files for the project
set(SOURCES src/main.cpp
    src/controller/ApplicationController.cpp
    src/controller/MenuController.cpp
    src/controller/GameController.cpp
    src/controller/AboutController.cpp

    src/model/Bullet.cpp
    src/model/GameMap.cpp
    src/model/GameModel.cpp
    src/model/GameObject.cpp
    src/model/Tank.cpp
    src/model/MenuModel.cpp
    src/model/AboutModel.cpp

    src/view/MenuView.cpp
    src/view/GameView.cpp
    src/view/AboutView.cpp

    src/common/Direction.h
)

add_executable(${PROJECT_NAME} ${SOURCES})

# Find the FLTK library
find_package(FLTK REQUIRED)

# # Find PNG library
# find_package(PNG REQUIRED)

# # Find ZLIB library
# find_package(ZLIB REQUIRED)

# Add the path to the FLTK headers for compilation
target_include_directories(${PROJECT_NAME} PUBLIC ${FLTK_INCLUDE_DIRS})

# # Add the path to the PNG and ZLIB headers for compilation
# if (PNG_FOUND AND ZLIB_FOUND)
# target_include_directories(${PROJECT_NAME} PUBLIC ${PNG_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
# endif()

# Disable the command window
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")

# Link libraries
target_link_libraries(${PROJECT_NAME}
${FLTK_LIBRARIES}
${PNG_LIBRARIES}
${ZLIB_LIBRARIES}
)

# Also add necessary libraries when building on Windows
if(WIN32)
target_link_libraries(${PROJECT_NAME} comctl32)
endif()
