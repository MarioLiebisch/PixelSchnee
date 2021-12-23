cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)

project(PixelSchnee)

if(WIN32)
	find_package(SFML COMPONENTS graphics main REQUIRED)
else()
	find_package(SFML COMPONENTS graphics REQUIRED)
	find_package(X11 REQUIRED)
endif()

add_executable(PixelSchnee WIN32 main.cpp)
if(WIN32)
target_link_libraries(PixelSchnee sfml-graphics sfml-main)
else()
target_link_libraries(PixelSchnee sfml-graphics ${X11_LIBRARIES})
endif()

