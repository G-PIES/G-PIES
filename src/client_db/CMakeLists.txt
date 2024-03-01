find_package(SQLite3)
if(SQLite3_FOUND)
  set(SQLite3_LIBRARY_TARGET SQLite::SQLite3)
else()
  include(FetchContent)
  FetchContent_Declare(
    sqlite3
    URL https://www.sqlite.org/2024/sqlite-amalgamation-3450100.zip
  )
  FetchContent_MakeAvailable(sqlite3)
  add_library(sqlite3 ${sqlite3_SOURCE_DIR}/sqlite3.c)
  set(SQLite3_INCLUDE_DIRS ${sqlite3_SOURCE_DIR})
  set(SQLite3_LIBRARY_TARGET sqlite3)
endif()

file(GLOB SRC_FILES ./*.cpp)
add_library(clientdb STATIC ${SRC_FILES})
target_include_directories(clientdb PRIVATE ${SQLite3_INCLUDE_DIRS})
target_link_libraries(clientdb ${SQLite3_LIBRARY_TARGET})
