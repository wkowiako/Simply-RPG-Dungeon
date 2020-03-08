cmake_minimum_required (VERSION 3.5.1)
project(SimplyRPGDungeon)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(LIBRARY_OUTPUT_PATH build/)
set(BINARY_OUTPUT_PATH build/)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
set(BUILD_SHARED_LIBS OFF)

include(External_GTest.cmake)
include_directories(${CURSES_INCLUDE_DIR})
include_directories(${GTEST_INCLUDE_DIRS})

if(${GTEST_ONLY})
    return()
endif()

################################################################################
#####                         MODULE DIR SOURCES                           #####
################################################################################
set(MODULE_MessageFramework         ${CMAKE_SOURCE_DIR}/MessageFramework/src)

################################################################################
#####                       MAKE BINARY CMAKE PART                         #####
################################################################################
# LIB_SRC
include_directories(${MODULE_MessageFramework})

# SRC
add_subdirectory(main/src)
add_subdirectory(${MODULE_MessageFramework})

################################################################################
#####                       MAKE UNIT TESTS CMAKE PART                     #####
################################################################################
# LIB_TEST
include_directories(${CMAKE_SOURCE_DIR}/MessageFramework/test)

# TEST
add_subdirectory(main/test)
add_subdirectory(MessageFramework/test)


################################################################################
#####                  LINKING AND BUILD EXECUTALBE PART                   #####
################################################################################
add_library(source_objects_lib OBJECT ${SOURCES})

# bin_RPG
add_executable(bin_RPG $<TARGET_OBJECTS:source_objects_lib>
                                ${MAIN})
target_link_libraries (bin_RPG ${CMAKE_THREAD_LIBS_INIT})

# ut_RPG
set(TEST_BINARY_FILES $<TARGET_OBJECTS:source_objects_lib>
                      ${TEST_FILES}
                      ${MAIN_TEST})
add_executable(ut_RPG ${TEST_BINARY_FILES})
target_link_libraries(ut_RPG ${GTEST_MAIN_LIBRARY}
                                          ${GTEST_LIBRARY}
                                          ${GMOCK_MAIN_LIBRARY}
                                          ${GMOCK_LIBRARY})

################################################################################
#####                      SET COMPILATION ORDER                           #####
##### 1.source_objects_lib                                                 #####
##### 2.messenger_binary                                                   #####
##### 3.googletest                                                         #####
##### 4.messenger_binary_UT                                                #####
################################################################################
add_dependencies(messenger_binary_UT messenger_binary)
add_dependencies(googletest messenger_binary)

################################################################################
#####                              VALGRIND                                #####
################################################################################
find_program(VALGRIND "valgrind")
if(VALGRIND)
    message("VALGRIND found in path:" ${VALGRIND})
    target_link_libraries(messenger_binary_UT LINK_PUBLIC)
    add_custom_target(valgrind COMMAND "${VALGRIND}" --tool=memcheck  --leak-check=yes
        --show-reachable=yes --num-callers=20
        --track-fds=no --log-file="valgrindOutput.txt"
        $<TARGET_FILE:messenger_binary_UT>)
endif()
