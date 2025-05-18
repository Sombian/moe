cmake_minimum_required(VERSION 3.10)

#-------------#
# set project #
#-------------#

project(moe LANGUAGES CXX)

#--------------#
# C++ settings #
#--------------#

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#------------#
# executable #
#------------#

add_executable(${PROJECT_NAME}
	src/main.cpp
)

add_executable(codegen_auto
	scripts/main.cpp
)

target_include_directories(${PROJECT_NAME}
	PUBLIC
		${CMAKE_SOURCE_DIR}/src
	PRIVATE
		${CMAKE_SOURCE_DIR}/3rd-party
)

target_include_directories(codegen_auto
	PUBLIC
		${CMAKE_SOURCE_DIR}/src
	PRIVATE
		${CMAKE_SOURCE_DIR}/3rd-party
)

#----------------#
# code generator #
#----------------#

file(DOWNLOAD
	https://www.unicode.org/Public/UNIDATA/UnicodeData.txt
	${CMAKE_SOURCE_DIR}/scripts/auto/UnicodeData.txt SHOW_PROGRESS
)
file(DOWNLOAD
	https://www.unicode.org/Public/UNIDATA/CaseFolding.txt
	${CMAKE_SOURCE_DIR}/scripts/auto/CaseFolding.txt SHOW_PROGRESS
)
file(DOWNLOAD
	https://www.unicode.org/Public/UNIDATA/CompositionExclusions.txt
	${CMAKE_SOURCE_DIR}/scripts/auto/CompositionExclusions.txt SHOW_PROGRESS
)
file(DOWNLOAD
	https://www.unicode.org/Public/UNIDATA/DerivedCoreProperties.txt
	${CMAKE_SOURCE_DIR}/scripts/auto/DerivedCoreProperties.txt SHOW_PROGRESS
)

add_custom_command(
	OUTPUT
		${CMAKE_SOURCE_DIR}/src/data/stage1.txt
		${CMAKE_SOURCE_DIR}/src/data/stage2.txt
		${CMAKE_SOURCE_DIR}/src/data/stage3.txt
	COMMAND
		codegen_auto
	DEPENDS
		codegen_auto
	WORKING_DIRECTORY
		${CMAKE_SOURCE_DIR}
)

add_custom_target(codegen_files ALL
	DEPENDS
		${CMAKE_SOURCE_DIR}/src/data/stage1.txt
		${CMAKE_SOURCE_DIR}/src/data/stage2.txt
		${CMAKE_SOURCE_DIR}/src/data/stage3.txt
)

#------------------<IMPORTANT>------------------#
add_dependencies(${PROJECT_NAME} codegen_files) #
#-----------------------------------------------#
