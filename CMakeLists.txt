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
add_compile_options(-Wno-switch)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

#-----------------#
# setup sanitizer #
#-----------------#

if (NOT MINGW)
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
# configure LLVM #
#----------------#

find_package(LLVM REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

#------------------#
# configure: tools #
#------------------#

add_executable(tools
	codegen/main.cpp
)

target_link_libraries(tools
	LLVMCore
	LLVMSupport
	LLVMIRReader
)

target_include_directories(tools
	PUBLIC
		${CMAKE_SOURCE_DIR}/src
	PRIVATE
		${CMAKE_SOURCE_DIR}/externs
)

#--------------------#
# configure: project #
#--------------------#

add_executable(${PROJECT_NAME}
	src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
	LLVMCore
	LLVMSupport
	LLVMIRReader
)

target_include_directories(${PROJECT_NAME}
	PUBLIC
		${CMAKE_SOURCE_DIR}/src
	PRIVATE
		${CMAKE_SOURCE_DIR}/externs
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

add_custom_target(utf ALL COMMENT "<-----<BUILT>---->"
	DEPENDS
		${CMAKE_SOURCE_DIR}/src/data/stage1.txt
		${CMAKE_SOURCE_DIR}/src/data/stage2.txt
		${CMAKE_SOURCE_DIR}/src/data/stage3.txt
)

#-------------<IMPORTANT>-------------#
add_dependencies(${PROJECT_NAME} utf) #
#-------------------------------------#
