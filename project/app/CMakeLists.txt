# Bot Application

set(SOURCE_FILES
    "main.cpp"
    "core/app.cpp" "core/config.cpp" "core/component.cpp"
    "components/commands/command_view.cpp" "components/commands/dev_commands.cpp"
    "components/repeat/repeat.cpp" "components/dice_roll.cpp" "utils/random.cpp")

add_executable(app ${SOURCE_FILES})

find_package(fmt CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS regex)
target_link_libraries(app PRIVATE
    miraipp
    fmt::fmt fmt::fmt-header-only
    ${Boost_LIBRARIES})
