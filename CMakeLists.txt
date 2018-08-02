##############################################################################
# P137's CMakeLists.txt
#
# --Instructions-------------------------
#
# 0. If you need to include a new source file, please make sure you have
#       <header_file>.hpp and <source_file>.cpp. Otherwhise CMake command
#       will return a bunch of multiple definitions errors on your methods.
#
# 1. To include a new source file, include the path of <source_file>.cpp
#       inside above set function under # FILE GLOBBING comment. This
#       function will set the sources of project. Make sure you pass
#       the path from the 'src' folder to the <source_file>.cpp.
#       (Eg. src/to/the/<source_file>.cpp).
#
# 2. To include a new external modules, first use pkg_check_modules(<module>)
#       and pass the name of your new module you want to add next to sub modules
#       needed, and then use link_directories(${<name_of_representative_directorie})
#       to create a link path.
#
# 2.1. If the module is a library, use find_library(${<LIB_NAME> <lib_modules>.so}).
#
# 3. Including new sources and new features, increment program version setting
#       CMAKE_MINOR_VERSION, CMAKE_PATCH_VERSION and P137_TWEEK_VERSION.
#
#    a. P137_MAJOR_VERSION represents the RELEASE version or a big number of
#           changes (eg. new UI, conceptual changes, etc).
#    b. P137_MINORR_VERSION represents one feature added or a collection of bug
#           fixes.
#    c. P137_PATCH_VERSION represents a bug fix release.
#    d. P137_TWEEK_VERSION represents the build number.
##############################################################################

include(CMakeToolsHelpers OPTIONAL)
include(cmake/PreventSourceFolderCorruption.cmake)
#include(cmake/P137MinDepVersions.cmake)

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(P137)

#  --DEPENDENCY RESOLUTION---------------
# set(bash_cmd "bash")
# message(STATUS "Verifying installed dependencies")
# execute_process(COMMAND ${bash_cmd}
#     WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#     RESULT_VARIABLE bash_result
#     OUTPUT_VARIABLE bash_resp)
# message(STATUS "All dependencies resolved")
# ---------------------------------------

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# --SETTING VERSION NUMBER---------------
set(P137_MAJOR_VERSION 1)
set(P137_MINOR_VERSION 1)
set(P137_PATCH_VERSION 0)
set(P137_TWEEK_VERSION 0)
set(P137 VERSION ${P137_MAJOR_VERSION}.${P137_MINOR_VERSION}.${P137_PATCH_VERSION}.${P137_TWEEK_VERSION})

# --DETECTING OPERATIONAL SYSTEM---------
message(STATUS "We are on a ${CMAKE_SYSTEM_NAME} system")
message(STATUS "The host processor is ${CMAKE_HOST_SYSTEM_PROCESSOR}")

# --SET C++ COMPILER FLAGS FOR GNU-------
if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wfatal-errors -g -m64 -pipe -pthread -lpthread -lopencv_shape -lopencv_stitching -lopencv_objdetect -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core -lboost_thread -lboost_system -lboost_filesystem")
endif()

# --DEPENDENCIES-------------------------
find_package(PkgConfig)

# Include GTKMM dirs
pkg_check_modules(GTKMM gtkmm-3.0)
link_directories(${GTKMM_LIBRARY_DIRS})

# Include V4L2 dirs
pkg_check_modules(V4L2 libv4l2 libv4lconvert)
link_directories(${V4L2_LIBRARY_DIRS})

# Find needed packages and libraries on system

find_package(Boost REQUIRED COMPONENTS date_time filesystem wserialization system serialization thread regex)

find_package(OpenCV REQUIRED)

find_library(LIBXBEE libxbee.so libxbeep.so)

# --FILE GLOBBING------------------------
set(sources 
        src/main.cpp
        src/camcap.cpp
        src/strategy.cpp
        src/constants.hpp
        src/robots.cpp

        src/aux/jsonSaveManager.cpp
        src/aux/kalmanFilter.cpp
		src/aux/ls.cpp
		src/aux/cpuTimer.cpp

        src/communication/serialCom.cpp
        src/communication/flyingMessenger.cpp

        src/capture/v4lcap.cpp
		src/capture/v4ldev.cpp

        src/vision/tag.cpp
        src/vision/vision.cpp

        src/gui/filechooser.cpp
        src/gui/controlGUI.cpp
        src/gui/testFrame.cpp
        src/gui/strategyGUI.cpp
        src/gui/visionGUI.cpp
        src/gui/robotGUI.cpp
        src/gui/imageView.cpp
        src/gui/v4linterface.cpp
        src/gui/v4linterfaceEvents.cpp
		)

include_directories(src
                    ${GTKMM_INCLUDE_DIRS}
                    ${Boost_INCLUDE_DIRS}
                    ${OpenCV_INCLUDE_DIRS}
                    ${V4L2_INCLUDE_DIRS}
)


# --ADD EXECUTABLE-----------------------
add_executable(P137 ${sources})

# --LINKING LIBRARIES--------------------
target_link_libraries(P137 ${GTKMM_LIBRARIES})
target_link_libraries(P137 ${Boost_LIBRARIES})
target_link_libraries(P137 ${OpenCV_LIBS})
target_link_libraries(P137 ${V4L2_LIBRARIES})
target_link_libraries(P137 ${LIBXBEE})
