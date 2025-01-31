# cmake_minimum_required(VERSION 3.10)

set(dir_name test_dir)

set(lecture_name PV112_${dir_name})

visitlab_generate_lecture(PV112 ${dir_name})

target_include_directories(${lecture_name} PUBLIC libraries/my_structures
                                            PUBLIC libraries/json
                                            PUBLIC libraries/SimplexNoise)

target_sources(${lecture_name} PUBLIC libraries/json/json.hpp)
target_sources(${lecture_name} PUBLIC libraries/SimplexNoise/SimplexNoise.cpp)
target_sources(${lecture_name} PUBLIC libraries/my_structures/GameController.cpp)
target_sources(${lecture_name} PUBLIC libraries/my_structures/SceneCameras.cpp)
target_sources(${lecture_name} PUBLIC libraries/my_structures/ObjectRenderStorage.cpp)
target_sources(${lecture_name} PUBLIC libraries/my_structures/MapManager.cpp)
target_sources(${lecture_name} PUBLIC libraries/my_structures/QOL.cpp)
target_sources(${lecture_name} PUBLIC libraries/my_structures/Timer.hpp)
target_sources(${lecture_name} PUBLIC libraries/my_structures/GameTileData.cpp)
target_link_libraries(${lecture_name} PUBLIC winmm)

set(copy_file_name "data.json")
set(copy_file_name "ambient_sound.wav")

add_custom_command(
    TARGET ${lecture_name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${course_path}
)
add_custom_command(
    TARGET ${lecture_name} POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/${copy_file_name}" "${CMAKE_BINARY_DIR}/courses/PV112/${dir_name}/"
    COMMENT "Copying ${copy_file_name} to ${CMAKE_BINARY_DIR}/courses/PV112/${dir_name} directory"
)