# Bot Application

set(SOURCE_FILES
    "main.cpp"
    "core/app.cpp" "core/config.cpp" "core/component.cpp"
    "components/commands/command_view.cpp" "components/commands/dev_commands.cpp"
    "components/repeat/repeat.cpp" "components/dice_roll.cpp" "components/random_select.cpp"
    "components/anti_recall.cpp" "components/commands/user_commands.cpp"
    "components/othello/othello_graphics.cpp" "components/othello/othello_ai.cpp"
    "components/othello/othello_game.cpp" "components/othello/othello_logic.cpp"
    "utils/random.cpp" "utils/image.cpp")

add_executable(app ${SOURCE_FILES})

find_path(STB_INCLUDE NAMES "stb.h")
if(STB_INCLUDE STREQUAL "STB_INCLUDE-NOTFOUND")
    message(FATAL_ERROR "Cannot find stb")
else()
    message(STATUS "Found stb at ${STB_INCLUDE}")
endif()

find_package(fmt CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS regex)
find_package(unofficial-graphicsmagick CONFIG REQUIRED)
target_link_libraries(app PRIVATE
    miraipp
    fmt::fmt fmt::fmt-header-only
    ${Boost_LIBRARIES}
    unofficial::graphicsmagick::graphicsmagick)
