find_package(SQLite3)
if(NOT SQLite3_FOUND)
  FetchContent_Declare(
    sqlite3
    URL https://www.sqlite.org/2024/sqlite-amalgamation-3450100.zip
  )
  FetchContent_MakeAvailable(sqlite3)
  add_library(sqlite3_lib ${sqlite3_SOURCE_DIR}/sqlite3.c)
  set(SQLite3_INCLUDE_DIRS ${sqlite3_SOURCE_DIR})
  set(SQLite3_LIBRARIES sqlite3_lib)
  add_library(SQLite::SQLite3 ALIAS ${SQLite3_LIBRARIES})
endif()
