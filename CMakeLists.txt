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
	script/unicode.cpp
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

##################
# download files #
##################

file(DOWNLOAD
	https://www.unicode.org/Public/UNIDATA/UnicodeData.txt
	${CMAKE_SOURCE_DIR}/script/generate/UnicodeData.txt SHOW_PROGRESS
)
file(DOWNLOAD
	https://www.unicode.org/Public/UNIDATA/CaseFolding.txt
	${CMAKE_SOURCE_DIR}/script/generate/CaseFolding.txt SHOW_PROGRESS
)
file(DOWNLOAD
	https://www.unicode.org/Public/UNIDATA/CompositionExclusions.txt
	${CMAKE_SOURCE_DIR}/script/generate/CompositionExclusions.txt SHOW_PROGRESS
)
file(DOWNLOAD
	https://www.unicode.org/Public/UNIDATA/DerivedCoreProperties.txt
	${CMAKE_SOURCE_DIR}/script/generate/DerivedCoreProperties.txt SHOW_PROGRESS
)
