# Bot Application

set(SOURCE_FILES
    "main.cpp"
 "core/app.cpp")

add_executable(app ${SOURCE_FILES})

find_package(fmt CONFIG REQUIRED)
target_link_libraries(app PRIVATE
    miraipp
    fmt::fmt fmt::fmt-header-only)
