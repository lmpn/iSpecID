FIND_PACKAGE(Boost REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
ADD_LIBRARY(annotatev2
  source/annotatev2.cxx
)
TARGET_INCLUDE_DIRECTORIES(annotatev2 PUBLIC include)
TARGET_LINK_LIBRARIES(annotatev2 utils tsvreader miner)