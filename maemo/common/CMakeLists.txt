set(SRC
  logging.cpp
  )

add_definitions(-DQT_SHARED)
add_library(common STATIC ${SRC})
