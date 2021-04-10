# Build output setup
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test/bin)

################################
# GTest
################################
project(googletest-git NONE)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.10.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(BUILD_GTEST ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

################################
# Tests
################################
# Add test cpp file
add_executable(BotTest BotTest.cpp ../src/Bot.cpp)
target_include_directories(BotTest PRIVATE ../include)
# Link test executable against gtest & gtest_main
target_link_libraries(BotTest gtest gtest_main)
if(MSVC)
    target_compile_features(BotTest PRIVATE cxx_std_17)
endif()

add_test(
    NAME BotTest
    COMMAND BotTest
)