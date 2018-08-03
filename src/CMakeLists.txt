cmake_minimum_required(VERSION 2.8)
project(jojo)
add_executable(jojo jojo.cpp
  libjojo.cpp
  scan.cpp
  sexp.cpp
  compile.cpp
  eval.cpp
  test_all.cpp
  import_all.cpp)
install(TARGETS jojo DESTINATION bin)