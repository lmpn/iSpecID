FIND_PACKAGE(Boost REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
ADD_LIBRARY(annotatev3
  source/annotatev3.cxx
)
TARGET_INCLUDE_DIRECTORIES(annotatev3 PUBLIC include)
TARGET_LINK_LIBRARIES(annotatev3 utils tsvreader miner)