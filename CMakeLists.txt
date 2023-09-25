cmake_minimum_required(VERSION 3.15)

# Enable vcpkg
set(CMAKE_TOOLCHAIN_FILE
    "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

# Set the triplet
if(WIN32)
    set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "VCPKG Triplet")
elseif(APPLE)
    execute_process(
        COMMAND uname -m
        COMMAND tr -d '\n'
        OUTPUT_VARIABLE VCPKG_ARCHITECTURE)
    message(STATUS "Architecture: ${VCPKG_ARCHITECTURE}")

    if(${VCPKG_ARCHITECTURE} STREQUAL "arm64") # Apple Silicon
        set(VCPKG_TARGET_TRIPLET "arm64-osx" CACHE STRING "VCPKG Triplet")
    else()
        set(VCPKG_TARGET_TRIPLET "x64-osx" CACHE STRING "VCPKG Triplet")
    endif()
else()
    execute_process(
        COMMAND uname -m
        COMMAND tr -d '\n'
        OUTPUT_VARIABLE VCPKG_ARCHITECTURE)
    message(STATUS "Architecture: ${VCPKG_ARCHITECTURE}")

    if(${VCPKG_ARCHITECTURE} STREQUAL "x86_64")
        set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "VCPKG Triplet")
    elseif(${VCPKG_ARCHITECTURE} STREQUAL "x86_86")
        set(VCPKG_TARGET_TRIPLET "x86-linux" CACHE STRING "VCPKG Triplet")
    elseif(${VCPKG_ARCHITECTURE} STREQUAL "aarch64")
        set(VCPKG_TARGET_TRIPLET "arm64-linux" CACHE STRING "VCPKG Triplet")
    elseif(${VCPKG_ARCHITECTURE} STREQUAL "arm64")
        set(VCPKG_TARGET_TRIPLET "arm64-linux" CACHE STRING "VCPKG Triplet")
    elseif(${VCPKG_ARCHITECTURE} MATCHES "arm.*")
        set(VCPKG_TARGET_TRIPLET "arm-linux" CACHE STRING "VCPKG Triplet")
    else()
        set(VCPKG_TARGET_TRIPLET "${VCPKG_ARCHITECTURE}-linux" CACHE STRING "VCPKG Triplet")
    endif()
endif()

# Some hacks to fix vcpkg
if(WIN32)
    set(ZLIB_LIBRARY_RELEASE
        "${CMAKE_CURRENT_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/lib/zlib.lib"
    )
    set(ZLIB_LIBRARY_DEBUG
        "${CMAKE_CURRENT_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/debug/lib/zlibd.lib"
    )
else()
    # Make BoostConfig.cmake able to check its inputs
    cmake_policy(SET CMP0057 NEW)

    # Honor the visibility properties for all target types, including
    # object libraries and static libraries.
    cmake_policy(SET CMP0063 NEW)

    # Don't ignore <PackageName>_ROOT variables
    cmake_policy(SET CMP0074 NEW)
endif()

project(emulationstation-all)

# Enable C++-17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------------------------------------------------------------
# add local find scripts to CMAKE path
LIST(APPEND CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Utils
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Packages
)

# -------------------------------------------------------------------------------
# set up OpenGL system variable
set(GLSystem "Desktop OpenGL" CACHE STRING "The OpenGL system to be used")
set_property(CACHE GLSystem PROPERTY STRINGS "Desktop OpenGL" "OpenGL ES")

# -------------------------------------------------------------------------------
# check if we're running on Raspberry Pi
MESSAGE("Looking for bcm_host.h")

if(EXISTS "/opt/vc/include/bcm_host.h")
    MESSAGE("bcm_host.h found")
    set(BCMHOST found)
    set(GLSystem "OpenGL ES")
else()
    MESSAGE("bcm_host.h not found")
endif()

# -------------------------------------------------------------------------------
# check if we're running on olinuxino
MESSAGE("Looking for libMali.so")

if(EXISTS "/usr/lib/libMali.so")
    MESSAGE("libMali.so found")
    set(GLSystem "OpenGL ES")
else()
    MESSAGE("libMali.so not found")
endif()

# finding necessary packages
# -------------------------------------------------------------------------------
if(${GLSystem} MATCHES "Desktop OpenGL")
    find_package(OpenGL REQUIRED)
else()
    find_package(OpenGLES REQUIRED)
endif()

find_package(ZLIB REQUIRED)
find_package(Freetype MODULE REQUIRED)
find_package(FreeImage CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(Boost 1.82.0 REQUIRED COMPONENTS system filesystem date_time regex locale thread)
find_package(Eigen3 CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)

# add ALSA for Linux
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_package(ALSA REQUIRED)
endif()

# -------------------------------------------------------------------------------
# set up compiler flags and excutable names
if(DEFINED BCMHOST)
    add_definitions(-D_RPI_)
endif()

# -------------------------------------------------------------------------------
if(MSVC)
    set(CMAKE_DEBUG_POSTFIX "d")

    # add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    # add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP") # multi-processor compilation
    # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP") # multi-processor compilation
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    # check for G++ 4.7+
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE G++_VERSION)

    if(G++_VERSION VERSION_LESS 4.7)
        message(SEND_ERROR "You need at least G++ 4.7 to compile EmulationStation!")
    endif()

    # set up compiler flags for GCC
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-attributes -O3") # support C++11 for std::, optimize
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O3") # -s = strip binary
endif()

if(APPLE AND NOT CMAKE_COMPILER_IS_GNUCXX)
    # set up compiler flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-attributes -O3") # support C++11 for std::, optimize
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O3") # -s = strip binary

    # set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
endif()

if(${GLSystem} MATCHES "Desktop OpenGL")
    add_definitions(-DUSE_OPENGL_DESKTOP)
else()
    add_definitions(-DUSE_OPENGL_ES)
endif()

add_definitions(-DEIGEN_DONT_ALIGN)

# -------------------------------------------------------------------------------
# add include directories
set(COMMON_INCLUDE_DIRS

    # OSX seems to have a conflicting platform.h somewhere and fails to find
    # getHomePath, so let use our includes paths first.
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/es-app/src
    ${CMAKE_CURRENT_SOURCE_DIR}/es-core/src
    ${FREETYPE_INCLUDE_DIRS}
    ${FreeImage_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIR}
    ${CURL_INCLUDE_DIR}
)

# add ALSA for Linux
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    LIST(APPEND COMMON_INCLUDE_DIRS
        ${ALSA_INCLUDE_DIRS}
    )
endif()

if(DEFINED BCMHOST)
    LIST(APPEND COMMON_INCLUDE_DIRS
        "/opt/vc/include"
        "/opt/vc/include/interface/vcos"
        "/opt/vc/include/interface/vmcs_host/linux"
        "/opt/vc/include/interface/vcos/pthreads"
    )
else()
    if(${GLSystem} MATCHES "Desktop OpenGL")
        LIST(APPEND COMMON_INCLUDE_DIRS
            ${OPENGL_INCLUDE_DIR}
        )
    else()
        LIST(APPEND COMMON_INCLUDE_DIRS
            ${OPENGLES_INCLUDE_DIR}
        )
    endif()
endif()

# -------------------------------------------------------------------------------
# define libraries and directories
if(DEFINED BCMHOST)
    link_directories(
        ${Boost_LIBRARY_DIRS}
        "/opt/vc/lib"
    )
else()
    link_directories(
        ${Boost_LIBRARY_DIRS}
    )
endif()

set(COMMON_LIBRARIES
    ${Boost_LIBRARIES}
    freeimage::FreeImage
    freeimage::FreeImagePlus
    Freetype::Freetype
    SDL2_mixer::SDL2_mixer-static
    SDL2::SDL2main
    SDL2::SDL2-static
    ${CURL_LIBRARIES}
    pugixml
    nanosvg
    sqlite3
)

# add ALSA for Linux
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    LIST(APPEND COMMON_LIBRARIES
        ${ALSA_LIBRARY}
    )
endif()

if(DEFINED BCMHOST)
    LIST(APPEND COMMON_LIBRARIES
        bcm_host
        EGL
        ${OPENGLES_LIBRARIES}
    )
else()
    if(MSVC)
        LIST(APPEND COMMON_LIBRARIES
            winmm
        )
    endif()

    if(${GLSystem} MATCHES "Desktop OpenGL")
        LIST(APPEND COMMON_LIBRARIES
            ${OPENGL_LIBRARIES}
        )
    else()
        LIST(APPEND COMMON_LIBRARIES
            EGL
            ${OPENGLES_LIBRARIES}
        )
    endif()
endif()

# -------------------------------------------------------------------------------
# set up build directories
set(dir ${CMAKE_CURRENT_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${dir} CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${dir} CACHE PATH "Build directory" FORCE)

# -------------------------------------------------------------------------------
# add each component
add_subdirectory("external")
add_subdirectory("es-core")
add_subdirectory("es-app")

# i18n
find_program(MSGFMT_EXECUTABLE msgfmt)

if(MSGFMT_EXECUTABLE)
    message(STATUS "Native language support enabled.")
    add_subdirectory(locale)
endif(MSGFMT_EXECUTABLE)
