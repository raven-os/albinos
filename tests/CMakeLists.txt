###### CTest ######
include(CTest)
###################

###### GTest ######
find_package(GTest REQUIRED)

set(SOURCES lib-test.cpp)
CREATE_UNIT_TEST(lib-test libconfig: ${SOURCES})
