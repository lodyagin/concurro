set (pevents_SOURCES
	  pevents.cpp
)

# Enable WaitForMultipleEvents
add_definitions(-DWFMO)

add_library(pevents ${pevents_SOURCES})
