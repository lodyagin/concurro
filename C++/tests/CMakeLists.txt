set(TestsSrc
  RMutexCU.cpp
  tests.cpp
)

include_directories(../../../CUnit/CUnit/Headers)
include_directories(..)

add_executable(tests_exec ${TestsSrc} ${TestsInc})

target_link_libraries(tests_exec cunit)

