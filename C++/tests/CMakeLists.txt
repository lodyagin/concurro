set(TestsSrc
  RMutexCU.cpp
  tests.cpp
)

include_directories(../../../CUnit/CUnit/Headers)
include_directories(..)
include_directories(${CMAKE_SOURCE_DIR}/PEvents)

add_executable(tests_exec ${TestsSrc} ${TestsInc})

target_link_libraries(tests_exec cunit boost_thread-mt log4cxx concurrent pevents)