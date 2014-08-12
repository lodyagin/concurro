set (pevents_SOURCES
	  pevents.cpp
)

set(CMAKE_CXX_FLAGS "-g -O0 -std=c++11 -Wall")

# Enable WaitForMultipleEvents
add_definitions(-DWFMO)

add_library(pevents SHARED ${pevents_SOURCES})
