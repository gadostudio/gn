# CMakeList.txt : CMake project for gncxx, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.8)

project("gn")

option(GN_STATIC_BUILD "Build static library" ON)
option(GN_ENABLE_VULKAN "Enable Vulkan backend" ON)
option(GN_ALLOW_DOWNLOAD_EXTERN_DEPS "Allow downloading of external dependencies from internet" OFF)
option(GN_BUILD_EXAMPLES "Build examples (must build static library)" ON)
option(GN_BUILD_TESTS "Build tests (must build static library)" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if(GN_ALLOW_DOWNLOAD_EXTERN_DEPS)
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

if(GN_STATIC_BUILD OR GN_BUILD_EXAMPLES OR GN_BUILD_TESTS)
    if(GN_ENABLE_VULKAN)
        if(GN_ALLOW_DOWNLOAD_EXTERN_DEPS)
            # Get Vulkan-Headers from github if allowed
            FetchContent_Declare(
                Vulkan-Headers
                GIT_REPOSITORY          https://github.com/KhronosGroup/Vulkan-Headers.git
                GIT_TAG                 v1.3.216 # Must match Vulkan SDK version
            )

            FetchContent_MakeAvailable(Vulkan-Headers)
            set(GN_STATIC_DEPS "${GN_STATIC_DEPS};Vulkan::Headers")
        else()
            find_package(Vulkan)

            if(Vulkan_FOUND)
                set(GN_STATIC_DEPS "${GN_STATIC_DEPS};Vulkan::Headers")
            endif()
        endif()
    endif()

    add_library(gn-static STATIC src/gn_impl_stub.cpp)
    target_link_libraries(gn-static PRIVATE ${GN_STATIC_DEPS} ${CMAKE_DL_LIBS})
    target_include_directories(gn-static
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
               $<INSTALL_INTERFACE:include>)

    if(GN_BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif()

    if(GN_BUILD_TESTS)
        set(GN_TEST_SOURCES
            test/instance_test.cpp
            test/device_test.cpp
            test/test_driver.cpp)

        add_executable(gn-test-d3d12 ${GN_TEST_SOURCES})
        target_compile_definitions(gn-test-d3d12 PUBLIC GN_TEST_BACKEND_D3D12)
        target_link_libraries(gn-test-d3d12 PRIVATE gn-static)

        add_executable(gn-test-vulkan ${GN_TEST_SOURCES})
        target_compile_definitions(gn-test-vulkan PUBLIC GN_TEST_BACKEND_VULKAN)
        target_link_libraries(gn-test-vulkan PRIVATE gn-static ${GN_STATIC_DEPS})

        add_executable(gn-barrier-test-vulkan test/barrier_conv_test.cpp)
        target_compile_definitions(gn-barrier-test-vulkan PUBLIC GN_TEST_BACKEND_VULKAN)
        target_link_libraries(gn-barrier-test-vulkan PRIVATE gn-static ${GN_STATIC_DEPS})
    endif()
endif()
