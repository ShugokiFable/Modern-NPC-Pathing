cmake_minimum_required(VERSION 3.21)
project(NPCPathingNG VERSION 2.4.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_PREFIX_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/extern/CommonLibSSE/install"
    "${CMAKE_CURRENT_SOURCE_DIR}/extern/CommonLibSSE/build/vcpkg_installed/x64-windows-static"
)
find_package(CommonLibSSE CONFIG REQUIRED)

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
    src/version.h
    src/settings.cpp
    src/raycast.cpp
    src/pathing.cpp
    src/npc_parkour.cpp
    src/evg_traversal.cpp
)

target_precompile_headers(${PROJECT_NAME} PRIVATE src/PCH.h)

target_link_libraries(${PROJECT_NAME} PRIVATE CommonLibSSE::CommonLibSSE)

# Match CommonLibSSE's static CRT (/MT instead of /MD)
set_property(TARGET ${PROJECT_NAME} PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set(OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/package/Data/SKSE/Plugins")
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "NPCPathingNG"
    SUFFIX ".dll"
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug"
)
