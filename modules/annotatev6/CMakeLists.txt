FIND_PACKAGE(Boost REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
ADD_LIBRARY(annotatev6
  source/annotatev6.cxx
)
TARGET_INCLUDE_DIRECTORIES(annotatev6 PUBLIC include)
TARGET_LINK_LIBRARIES(annotatev6 utils tsvreader miner )