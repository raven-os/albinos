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

    //! Creation config
    CHECK_EQ(Albinos::createConfig("toto", &ptr), Albinos::SUCCESS);

    //! Get key
    Albinos::Key key;
    CHECK_EQ(Albinos::getConfigKey(ptr, &key), Albinos::SUCCESS);
    std::string str("key is: [ " + std::string(key.data) + " ]");
    MESSAGE(str);
    CHECK(not std::string(key.data).empty());
    CHECK_GT(key.size, 0);

    //! Readonly key
    Albinos::Key readonly_key;
    CHECK_EQ(Albinos::getReadOnlyConfigKey(ptr, &readonly_key), Albinos::SUCCESS);
    std::string readonly_key_str("key is: [ " + std::string(readonly_key.data) + " ]");
    MESSAGE(readonly_key_str);
    CHECK(not std::string(readonly_key.data).empty());
    CHECK_GT(readonly_key.size, 0);

    //! Destruction config
    CHECK_EQ(Albinos::destroyConfig(ptr), Albinos::SUCCESS);
}