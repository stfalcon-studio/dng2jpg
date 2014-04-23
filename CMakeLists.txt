project(dng2jpg)

add_definitions(-Wall -O2 -Wextra -std=c++11 -pedantic)

set( SOURCES main.cpp )

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} WindowsCodecs Ole32)
