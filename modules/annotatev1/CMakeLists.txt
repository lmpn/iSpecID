FIND_PACKAGE(Boost REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
ADD_LIBRARY(annotatev1
  source/annotatev1.cxx
)
TARGET_INCLUDE_DIRECTORIES(annotatev1 PUBLIC include)
TARGET_LINK_LIBRARIES(annotatev1 utils tsvreader miner)