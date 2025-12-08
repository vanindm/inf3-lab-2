cmake_minimum_required(VERSION 3.25)
project(lab2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23 -Wall -g")

find_package(OpenSSL REQUIRED)

find_package(wxWidgets CONFIG REQUIRED COMPONENTS core base ribbon)

include(UsewxWidgets)

set(LAB1_SRC_LIST src/*.cpp)

include(CTest)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
endif()

add_executable(DictTestExec     src/DictTest.cpp)
add_executable(FSTestExec     	src/FSTest.cpp)
add_executable(UI 			  	src/UI.cpp)

target_include_directories(DictTestExec			PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(FSTestExec			PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(UI					PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_subdirectory(PATypes)

target_link_libraries(DictTestExec  PATypes)
target_link_libraries(FSTestExec  PATypes OpenSSL::Crypto)
target_link_libraries(UI PATypes OpenSSL::Crypto wx::core wx::ribbon)

enable_testing()

add_test(success_DictTest	DictTestExec)
add_test(success_FSTest		FSTestExec)