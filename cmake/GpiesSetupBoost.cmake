set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_STATIC_RUNTIME     OFF)
set(Boost_USE_MULTITHREADED      OFF)
set(BOOST_IOSTREAMS_ENABLE_BZIP2 ON)
find_package(Boost COMPONENTS program_options iostreams)
if(NOT Boost_FOUND)
  FetchContent_Declare(
    Boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.7z # downloading a zip release speeds up the download
  )
  set(BOOST_ENABLE_CMAKE ON)
  set(BOOST_INCLUDE_LIBRARIES program_options iostreams)
  FetchContent_MakeAvailable(Boost)
endif()
