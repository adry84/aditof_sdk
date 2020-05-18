cmake_minimum_required(VERSION 3.4)
project(write_eeprom VERSION 0.0.1 LANGUAGES C CXX)
set (CMAKE_CXX_STANDARD 11)

# add the executable
add_executable(${PROJECT_NAME} 
	main.cpp
	write_eeprom.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 11)

target_link_libraries(${PROJECT_NAME} PRIVATE aditof)



