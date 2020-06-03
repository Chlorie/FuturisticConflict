# Bot Application

set(APP_NAME app)

set(SOURCE_FILES
    "main.cpp"
    "core/app.cpp" "core/config.cpp" "core/component.cpp"
    "components/commands/command_view.cpp" "components/commands/dev_commands.cpp"
    "components/repeat/repeat.cpp" "components/dice_roll.cpp" "components/random_select.cpp"
    "components/anti_recall.cpp" "components/commands/user_commands.cpp"
    "components/othello/othello_graphics.cpp" "components/othello/othello_ai.cpp"
    "components/othello/othello_game.cpp" "components/othello/othello_logic.cpp"
    "utils/random.cpp" "utils/image.cpp")

add_executable(${APP_NAME} ${SOURCE_FILES})

target_compile_features(${APP_NAME} PRIVATE cxx_std_17) # Enforce C++17 or newer
set_target_properties(${APP_NAME} PROPERTIES CXX_EXTENSIONS off)

if (MSVC)
    # Force MSVC to use utf-8 encoding regardless of whether the BOM exists
    target_compile_options(${APP_NAME} PRIVATE "/utf-8")
endif ()

# Warnings and errors settings
# Use highest reasonable warning level, and treat warnings as errors
if (MSVC) # Visual Studio
    if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]") # If default /W3 presents
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}") # Change /W3 to /W4
    else ()
        target_compile_options(${APP_NAME} PRIVATE /W4) # Add /W4 directly
    endif ()
    target_compile_options(${APP_NAME} PRIVATE /WX) # Treat warnings as errors
    # Treat all header files specified by angle brackets to be system headers, and ignore all those warnings
    target_compile_options(${APP_NAME} PRIVATE 
        /experimental:external /external:W0 /external:anglebrackets)
else () # Not Visual Studio, assuming gcc or clang
    target_compile_options(${APP_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif ()

find_path(STB_INCLUDE NAMES "stb.h")
if(STB_INCLUDE STREQUAL "STB_INCLUDE-NOTFOUND")
    message(FATAL_ERROR "Cannot find stb")
else()
    message(STATUS "Found stb at ${STB_INCLUDE}")
endif()

find_package(fmt CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS regex)
target_link_libraries(${APP_NAME} PRIVATE
    miraipp
    fmt::fmt fmt::fmt-header-only
    ${Boost_LIBRARIES})
