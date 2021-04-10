cmake_minimum_required(VERSION 3.0)
project(DvfSim)

add_executable(DvfSim src/main.cpp
                      src/bot.cpp
                      include/DvfSimulator.h
                      include/Bot.h)
                      
target_include_directories(DvfSim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)


add_subdirectory(test)
enable_testing()


if(MSVC)
    target_compile_features(DvfSim PRIVATE cxx_std_17)
endif()

