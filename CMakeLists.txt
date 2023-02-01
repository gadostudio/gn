# CMakeList.txt : CMake project for gncxx, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.12)

project("gn")

option(GN_STATIC_BUILD "Build static library" ON)
option(GN_ENABLE_VULKAN "Enable Vulkan backend" ON)
option(GN_DOWNLOAD_EXTERN_DEPS "Download external dependencies from internet" OFF)
option(GN_DONT_USE_DEPS "Disable external dependencies" OFF)
option(GN_BUILD_EXAMPLES "Build examples (must build static library)" ON)
option(GN_BUILD_TESTS "Build tests (must build static library)" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_EXC TRUE)

if(GN_DOWNLOAD_EXTERN_DEPS)
    include(FetchContent)
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")
endif()

add_library(gn INTERFACE)
set_property(TARGET gn PROPERTY CXX_STANDARD 17)
target_include_directories(gn
    INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
              $<INSTALL_INTERFACE:include>)

add_library(gnsl INTERFACE)
set_property(TARGET gnsl PROPERTY CXX_STANDARD 17)
target_include_directories(gnsl
    INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
              $<INSTALL_INTERFACE:include>)

if(GN_STATIC_BUILD OR GN_BUILD_EXAMPLES OR GN_BUILD_TESTS)
    if(GN_ENABLE_VULKAN)
        if(NOT GN_DONT_USE_DEPS AND GN_DOWNLOAD_EXTERN_DEPS)
            FetchContent_Declare(
                Vulkan-Headers
                GIT_REPOSITORY          https://github.com/KhronosGroup/Vulkan-Headers.git
                GIT_TAG                 v1.3.216 # Must match with the latest Vulkan SDK version
            )

            FetchContent_MakeAvailable(Vulkan-Headers)
            set(GN_BUILD_EXAMPLES OFF)
        elseif(NOT GN_DONT_USE_DEPS)
            find_package(Vulkan)
        endif()

        set(GN_STATIC_DEPS "${GN_STATIC_DEPS};Vulkan::Headers")
    endif()

    add_library(gn-static STATIC src/gn_impl_stub.cpp)
    target_link_libraries(gn-static PRIVATE ${GN_STATIC_DEPS} ${CMAKE_DL_LIBS})
    target_include_directories(gn-static
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
               $<INSTALL_INTERFACE:include>)

    add_library(gnsl-static STATIC src/gnsl_impl_stub.cpp)
    target_link_libraries(gn-static PRIVATE gnsl)
    target_include_directories(gnsl-static
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
               $<INSTALL_INTERFACE:include>)

    if(GN_BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif()

    if(GN_BUILD_TESTS)
        add_subdirectory(test)
    endif()
endif()
