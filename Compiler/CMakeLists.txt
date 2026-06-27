cmake_minimum_required(VERSION 3.20)
project(FDMJCompiler VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
add_compile_options(-w)

include_directories(
  include/frontend
  include/ast
  include/ir
  include/quad
  include/quadflow
  include/opt
  include/instr
  include/reg
  include/util
  include/pipeline
  vendor/tinyxml2
)

add_subdirectory(vendor)
add_subdirectory(lib)
add_subdirectory(tools)
