cmake_minimum_required(VERSION 3.16)
project(ELRS_menu VERSION 1.0.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(ELRS_menu STATIC
    src/elrs_exhange.cpp
    src/exchange_handler.cpp
    src/entry_classes/classes/command.cpp
    src/entry_classes/classes/info.cpp
    src/entry_classes/classes/integer.cpp
    src/entry_classes/classes/txt_select.cpp
)
add_library(ELRS_menu::ELRS_menu ALIAS ELRS_menu)
target_compile_features(ELRS_menu PUBLIC cxx_std_17)
target_include_directories(ELRS_menu PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(TARGETS ELRS_menu EXPORT ELRS_menuTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT ELRS_menuTargets NAMESPACE ELRS_menu:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ELRS_menu)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ELRS_menuConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ELRS_menuConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ELRS_menu)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/ELRS_menuConfigVersion.cmake
    VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ELRS_menuConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/ELRS_menuConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ELRS_menu)
