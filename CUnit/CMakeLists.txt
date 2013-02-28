#add_subdirectory(Sources/Automated)
#add_subdirectory(Sources/Basic)
#add_subdirectory(Sources/Console)
#add_subdirectory(Sources/Framework)

set(CUnitAutomatedSrc
  ${CMAKE_CURRENT_SOURCE_DIR}/Sources/Automated/Automated.c
)

set(CUnitAutomatedInc
  ${CMAKE_CURRENT_SOURCE_DIR}/Headers/Automated.h
)

set(CUnitBasicSrc
  ${CMAKE_CURRENT_SOURCE_DIR}/Sources/Basic/Basic.c
)

set(CUnitBasicInc
  ${CMAKE_CURRENT_SOURCE_DIR}/Headers/Basic.h
)

set(CUnitConsoleSrc
  ${CMAKE_CURRENT_SOURCE_DIR}/Sources/Console/Console.c
)

set(CUnitConsoleInc
  ${CMAKE_CURRENT_SOURCE_DIR}/Headers/Console.h
)

set (CUnitFrameworkSrc
  ${CMAKE_CURRENT_SOURCE_DIR}/Sources/Framework/CUError.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Sources/Framework/MyMem.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Sources/Framework/TestDB.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Sources/Framework/TestRun.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Sources/Framework/Util.c
)

set (CUnitFrameworkInc
  ${CMAKE_CURRENT_SOURCE_DIR}/Headers/CUError.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Headers/CUnit.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Headers/CUnit_intl.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Headers/MyMem.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Headers/TestDB.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Headers/TestRun.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Headers/Util.h
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/Headers
)

add_library(cunit 
  ${CUnitAutomatedSrc} 
  ${CUnitBasicSrc}
  ${CUnitConsoleSrc}
  ${CUnitFrameworkSrc}

  # For VS
  ${CUnitAutomatedInc} 
  ${CUnitBasicInc}
  ${CUnitConsoleInc}
  ${CUnitFrameworkInc}
)

