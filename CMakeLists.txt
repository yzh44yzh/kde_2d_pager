cmake_minimum_required(VERSION 3.24)
project(kde_2d_pager VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

find_package(ECM 6.0 REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(KDEInstallDirs6)
include(ECMUninstallTarget)
find_package(Qt6 6.8 REQUIRED COMPONENTS Core DBus Gui Qml Quick)
find_package(KF6Config REQUIRED)
find_package(Plasma 6.6 REQUIRED)
find_package(PlasmaActivities 6.6 REQUIRED)
find_package(KF6CoreAddons REQUIRED)

add_library(pagercore STATIC
    src/workspace.h
    src/kdeworkspace.h src/kdeworkspace.cpp
    src/switchcontroller.h src/switchcontroller.cpp
)
target_include_directories(pagercore PUBLIC src)
target_link_libraries(pagercore PUBLIC Qt6::Core Qt6::DBus Plasma::Activities)
target_compile_options(pagercore PRIVATE -Wall -Wextra -Wpedantic)
qt6_extract_metatypes(pagercore)

add_subdirectory(applet)

include(CTest)
if(BUILD_TESTING)
    find_package(Qt6 REQUIRED COMPONENTS Test)
    add_subdirectory(tests)
endif()
