cmake_minimum_required(VERSION 3.10)

project("ClapAdditiveSynth" VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(PLUGIN_ID org.ilsoft.${PROJECT_NAME})
set(PLUGIN_NAME ${PROJECT_NAME})
set(PLUGIN_VENDOR org.ilsoft)
set(PLUGIN_URL https://github.com/skuyuy)
set(PLUGIN_DESCRIPTION "Additive Synth in CLAP")
set(CLAP_BUILD_TESTS OFF)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/constants.h.in 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/constants.h
)

add_subdirectory(deps/clap EXCLUDE_FROM_ALL)
add_subdirectory(deps/clap-helpers EXCLUDE_FROM_ALL)

# scan sources
file(GLOB_RECURSE SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE INCL ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)


add_library(
    ${PROJECT_NAME} MODULE
    ${SRC}
    ${INCL}
)
target_link_libraries(${PROJECT_NAME} clap-core clap-helpers)

target_include_directories(
    ${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# platform stuff
if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        BUNDLE True
        BUNDLE_EXTENSION clap
        MACOSX_BUNDLE_GUI_IDENTIFIER org.ilsoft.${VENDOR}
        MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
        MACOSX_BUNDLE_BUNDLE_VERSION ${CMAKE_PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${CMAKE_PROJECT_VERSION}
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/cmake/${PROJECT_NAME}.plist.in
    )
    target_link_libraries(${PROJECT_NAME} "-framework CoreFoundation" "-framework AppKit" "-framework CoreGraphics")
    target_compile_definitions(${PROJECT_NAME} PRIVATE IS_MAC=1)
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Werror
        $<$<BOOL:${USE_SANITIZER}>:-fsanitize=address>
        $<$<BOOL:${USE_SANITIZER}>:-fsanitize=undefined>
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<BOOL:${USE_SANITIZER}>:-fsanitize=address>
        $<$<BOOL:${USE_SANITIZER}>:-fsanitize=undefined>
    )
elseif(UNIX)
    target_compile_definitions(${PROJECT_NAME} PRIVATE IS_LINUX=1)
    # linux adapter
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".clap" PREFIX "")
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE IS_WIN=1)
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".clap" PREFIX "")
endif()

# run clap-validator