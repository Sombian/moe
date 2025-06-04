cmake_minimum_required(VERSION 3.10)

#-------------#
# set project #
#-------------#

project(moe LANGUAGES C CXX)

#--------------#
# C++ settings #
#--------------#

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find_package(LLVM REQUIRED CONFIG)
# add_definitions(${LLVM_DEFINITIONS})
# include_directories(${LLVM_INCLUDE_DIRS})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)

if (MSVC)
	add_link_options(/fsanitize=undefined /fsanitize=address /utf-8)
	add_compile_options(/fsanitize=undefined /fsanitize=address /utf-8)
else()
	# add_link_options(-fsanitize=undefined -fsanitize=address -utf-8)
	# add_compile_options(-fsanitize=undefined -fsanitize=address -utf-8)
endif()

#------------#
# executable #
#------------#

add_executable(tools
	scripts/main.cpp
)

add_executable(${PROJECT_NAME}
	src/main.cpp
)

target_include_directories(tools
	PUBLIC
		${CMAKE_SOURCE_DIR}/src
	PRIVATE
		${CMAKE_SOURCE_DIR}/3rd-party
)

target_include_directories(${PROJECT_NAME}
	PUBLIC
		${CMAKE_SOURCE_DIR}/src
	PRIVATE
		${CMAKE_SOURCE_DIR}/3rd-party
)

#----------------#
# code generator #
#----------------#

# file(DOWNLOAD
# 	https://www.unicode.org/Public/UNIDATA/UnicodeData.txt
# 	${CMAKE_SOURCE_DIR}/scripts/auto/UnicodeData.txt SHOW_PROGRESS
# )
# file(DOWNLOAD
# 	https://www.unicode.org/Public/UNIDATA/CaseFolding.txt
# 	${CMAKE_SOURCE_DIR}/scripts/auto/CaseFolding.txt SHOW_PROGRESS
# )
# file(DOWNLOAD
# 	https://www.unicode.org/Public/UNIDATA/CompositionExclusions.txt
# 	${CMAKE_SOURCE_DIR}/scripts/auto/CompositionExclusions.txt SHOW_PROGRESS
# )
# file(DOWNLOAD
# 	https://www.unicode.org/Public/UNIDATA/DerivedCoreProperties.txt
# 	${CMAKE_SOURCE_DIR}/scripts/auto/DerivedCoreProperties.txt SHOW_PROGRESS
# )

add_custom_command(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	DEPENDS
		tools
	COMMAND
		tools
	OUTPUT
		${CMAKE_SOURCE_DIR}/src/data/stage1.txt
		${CMAKE_SOURCE_DIR}/src/data/stage2.txt
		${CMAKE_SOURCE_DIR}/src/data/stage3.txt
)

add_custom_target(autogen ALL COMMENT "<-----<BUILT>---->"
	DEPENDS
		${CMAKE_SOURCE_DIR}/src/data/stage1.txt
		${CMAKE_SOURCE_DIR}/src/data/stage2.txt
		${CMAKE_SOURCE_DIR}/src/data/stage3.txt
)

#---------------<IMPORTANT>---------------#
add_dependencies(${PROJECT_NAME} autogen) #
#-----------------------------------------#
