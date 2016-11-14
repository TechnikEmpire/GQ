# Build script for GQ written by Henri Hyyryläinen
# This is meant to be used within a cmake project
cmake_minimum_required(VERSION 3.0)

if(UNIX)
  
  # C++14
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wl,--no-undefined -Wl,--no-allow-shlib-undefined")
  
endif(UNIX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GUMBO REQUIRED gumbo)

include_directories(${GUMBO_INCLUDE_DIRS})
link_directories(${GUMBO_LIBRARY_DIRS})

add_library(GQ SHARED

  src/AttributeSelector.cpp
  src/AttributeSelector.hpp
  src/BinarySelector.cpp
  src/BinarySelector.hpp
  src/Document.cpp
  src/Document.hpp
  src/Node.cpp
  src/Node.hpp
  src/NodeMutationCollection.cpp
  src/NodeMutationCollection.hpp
  src/Parser.cpp 
  src/Parser.hpp 
  src/Selection.cpp 
  src/Selection.hpp 
  src/Selector.cpp 
  src/Selector.hpp 
  src/Serializer.cpp 
  src/Serializer.hpp 
  src/SpecialTraits.cpp 
  src/SpecialTraits.hpp 
  src/StrRefHash.hpp 
  src/TextSelector.cpp
  src/TextSelector.hpp
  src/TreeMap.cpp
  src/TreeMap.hpp
  src/UnarySelector.cpp
  src/UnarySelector.hpp
  src/Util.cpp
  src/Util.hpp
  )

target_link_libraries(GQ ${GUMBO_LIBRARIES})



  
