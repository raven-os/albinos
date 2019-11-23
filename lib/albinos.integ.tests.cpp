//
// Created by milerius on 23/11/2019.
//

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include "Albinos.h"
#include "Config.hpp"

//! All the tests need the service to be launched
TEST_CASE("All case")
{
    Albinos::Config albinos_cfg = Albinos::Config("foo");
    Albinos::Config* ptr = &albinos_cfg;

    //! Creation
    CHECK_EQ(Albinos::createConfig("toto", &ptr), Albinos::SUCCESS);

    CHECK_EQ(Albinos::destroyConfig(ptr), Albinos::SUCCESS);
}