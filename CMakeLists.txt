cmake_minimum_required(VERSION 3.10)

###############
# set project #
###############

project(moe LANGUAGES CXX)

################
# C++ settings #
################

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

##################
# define project #
##################

add_executable(${PROJECT_NAME}
	src/main.cpp
)

###################
# define includes #
###################

target_include_directories(${PROJECT_NAME}
	PUBLIC
		${CMAKE_SOURCE_DIR}/src
	PRIVATE
		${CMAKE_SOURCE_DIR}/3rd-party
)
