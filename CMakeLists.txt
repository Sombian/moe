cmake_minimum_required(VERSION 3.10)

include(CheckCXXCompilerFlag)

project(moe LANGUAGES C CXX)

#--------------#
# C++ settings #
#--------------#

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_link_options(-fuse-ld=lld)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

#-----------------#
# setup sanitizer #
#-----------------#

if (MSVC)
	add_link_options(/fsanitize=address /utf-8)
	add_link_options(/fsanitize=address /utf-8)
elseif (NOT MINGW)
	check_cxx_compiler_flag("-fsanitize=address" ASAN_AD)

	if (ASAN_AD)
		add_link_options(-fsanitize=address)
		add_compile_options(-fsanitize=address)
	endif()

	check_cxx_compiler_flag("-fsanitize=undefined" ASAN_UB)

	if (ASAN_UB)
		add_link_options(-fsanitize=undefined)
		add_compile_options(-fsanitize=undefined)
	endif()
endif()

#----------------#
# configure: moe #
#----------------#

add_executable(moe
	src/main.cpp
)

target_include_directories(moe
	PUBLIC
		${CMAKE_SOURCE_DIR}/src
)

#------------------#
# configure: tools #
#------------------#

add_executable(tools
	tools/main.cpp
)

target_include_directories(tools
	PUBLIC
		${CMAKE_SOURCE_DIR}/src
)

#-----------#
# setup CWD #
#-----------#

set_property(TARGET moe PROPERTY
DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

set_property(TARGET tools PROPERTY
DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

#------------#
# setup LLVM #
#------------#

if (NOT MSVC)
	# https://llvm.org/docs/CMake.html
	find_package(LLVM REQUIRED CONFIG)
	add_definitions(${LLVM_DEFINITIONS})
	include_directories(${LLVM_INCLUDE_DIRS})

	target_link_libraries(moe
		LLVMCore LLVMSupport LLVMIRReader
	)

	target_link_libraries(tools
		LLVMCore LLVMSupport LLVMIRReader
	)
endif()

#--------------#
# auto codegen #
#--------------#

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

add_custom_target(utf ALL COMMENT "<-----<BUILT>---->"
	DEPENDS
		${CMAKE_SOURCE_DIR}/src/data/stage1.txt
		${CMAKE_SOURCE_DIR}/src/data/stage2.txt
		${CMAKE_SOURCE_DIR}/src/data/stage3.txt
)

#-------------<IMPORTANT>-------------#
add_dependencies(${PROJECT_NAME} utf) #
#-------------------------------------#
