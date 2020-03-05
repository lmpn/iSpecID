FIND_PACKAGE(Boost REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
ADD_LIBRARY(annotatev4
  source/annotatev4.cxx
)
TARGET_INCLUDE_DIRECTORIES(annotatev4 PUBLIC include)
TARGET_LINK_LIBRARIES(annotatev4 utils tsvreader miner)