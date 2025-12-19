# This project is distributed under the MIT license (see LICENSE)
#

cmake_minimum_required(VERSION 2.0)

project(pong)

file(GLOB PONG_FILE_LIST
	"*.cpp"
	"*.h"
)

add_executable(pong WIN32 ${PONG_FILE_LIST})
target_link_libraries(pong "winmm.lib" "d3d9.lib")