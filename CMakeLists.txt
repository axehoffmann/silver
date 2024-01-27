cmake_minimum_required (VERSION 3.20)

project ("silver")

 
find_package(LLVM REQUIRED CONFIG PATHS "C:/tools/llvm/build/lib/cmake/llvm")

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})
 
add_executable(silver "silver.cpp" "silver.hpp" "iter.hpp")
add_subdirectory(src)

target_include_directories(silver PUBLIC "../silver")

llvm_map_components_to_libnames(llvm_libs instcombine passes instrumentation scalaropts transformutils analysis support core irreader)
target_link_libraries(silver ${llvm_libs})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET silver PROPERTY CXX_STANDARD 20)
endif()

