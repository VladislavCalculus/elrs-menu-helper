cmake_minimum_required(VERSION 3.16)

add_library(ELRS_menu STATIC)

file(GLOB_RECURSE ELRS_MENU_SOURCES
     CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_LIST_DIR}/src/*.c"
     "${CMAKE_CURRENT_LIST_DIR}/src/*.cc"
     "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
)

target_sources(ELRS_menu PRIVATE ${ELRS_MENU_SOURCES})

target_include_directories(ELRS_menu
  PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/src"
)

target_compile_features(ELRS_menu PUBLIC cxx_std_17)
