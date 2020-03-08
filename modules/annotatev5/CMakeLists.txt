FIND_PACKAGE(Boost REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
ADD_LIBRARY(annotatev5
  source/annotatev5.cxx
)
TARGET_INCLUDE_DIRECTORIES(annotatev5 PUBLIC include)
TARGET_LINK_LIBRARIES(annotatev5 utils tsvreader miner )