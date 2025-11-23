cmake_minimum_required(VERSION 3.10)

project(lab1)
set(LAB1_SRC_LIST src/*.cpp)

include(CTest)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23 -Wall -g")

add_executable(lazySequenceTestExec     src/lazySequenceTest.cpp)
add_executable(streamTestExec    src/streamTest.cpp)
add_executable(performanceTestExec	src/performanceTest.cpp)
add_executable(turingMachineTestExec	src/turingMachineTest.cpp)
add_executable(turingMachineVigenereExec	src/turingMachineVigenere.cpp)

target_include_directories(lazySequenceTestExec			PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(streamTestExec		PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(performanceTestExec			PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(turingMachineTestExec	PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(turingMachineVigenereExec	PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_subdirectory(PATypes)

target_link_libraries(lazySequenceTestExec  PATypes)
target_link_libraries(streamTestExec    PATypes)
target_link_libraries(performanceTestExec	PATypes)
target_link_libraries(turingMachineTestExec	PATypes)
target_link_libraries(turingMachineVigenereExec	PATypes)

enable_testing()

add_test(success_lazySequenceTest	lazySequenceTestExec)
add_test(success_streamTest	streamTestExec)
add_test(success_turingMachineTest	turingMachineTestExec)
add_test(success_performanceTest		performanceTestExec 10)
