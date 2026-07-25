cmake_minimum_required(VERSION 3.21)
project(StepUpOntoSKSE VERSION 1.5.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build against the same CommonLibSSE-NG the NPCPathingNG plugin uses.
# Override with -DCOMMONLIB_SSE_ROOT=<path to extern/CommonLibSSE> when it lives elsewhere.
set(COMMONLIB_SSE_ROOT "" CACHE PATH "Path to a built CommonLibSSE-NG checkout (contains install/ and build/)")
if (NOT COMMONLIB_SSE_ROOT)
    foreach (candidate IN ITEMS
        "${CMAKE_CURRENT_SOURCE_DIR}/../Modern NPC Pathing/extern/CommonLibSSE"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../NPC Pathing NG 2.4.1/extern/CommonLibSSE"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../NPC Pathing NG 2.4.0/extern/CommonLibSSE"
        "${CMAKE_CURRENT_SOURCE_DIR}/../extern/CommonLibSSE"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../extern/CommonLibSSE"
    )
        if (EXISTS "${candidate}/install/lib/cmake/CommonLibSSE/CommonLibSSEConfig.cmake")
            set(COMMONLIB_SSE_ROOT "${candidate}")
            break()
        endif()
    endforeach()
endif()
if (NOT EXISTS "${COMMONLIB_SSE_ROOT}/install/lib/cmake/CommonLibSSE/CommonLibSSEConfig.cmake")
    message(FATAL_ERROR
        "Built CommonLibSSE-NG not found. Build it first (see NPC Pathing NG/BUILDING.md), "
        "then rerun, or pass -DCOMMONLIB_SSE_ROOT=<path to extern/CommonLibSSE>.")
endif()
message(STATUS "Using CommonLibSSE-NG at: ${COMMONLIB_SSE_ROOT}")
list(APPEND CMAKE_PREFIX_PATH
    "${COMMONLIB_SSE_ROOT}/install"
    "${COMMONLIB_SSE_ROOT}/build/vcpkg_installed/x64-windows-static"
)
find_package(CommonLibSSE CONFIG REQUIRED)

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
    src/Settings.cpp
    src/Menu.cpp
    src/Hooks.cpp
    src/Raycast.cpp
    src/StepUpManager.cpp
)

target_precompile_headers(${PROJECT_NAME} PRIVATE src/PCH.h)
target_link_libraries(${PROJECT_NAME} PRIVATE CommonLibSSE::CommonLibSSE)

set_property(TARGET ${PROJECT_NAME} PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_definitions(${PROJECT_NAME} PRIVATE _UNICODE UNICODE)

set(OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/package/SKSE/Plugins")
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "StepUpOntoSKSE"
    SUFFIX ".dll"
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIR}"
)
