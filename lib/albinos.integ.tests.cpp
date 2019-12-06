//
// Created by milerius on 23/11/2019.
//

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include "Albinos.h"
#include "Config.hpp"

//! Create, GetConfigKey, SetSetting, GetSettingSize, GetSettingValue, SetSettingAlias, GetReadOnlyKey, GetConfig,
//! UnsetAlias, RemoveSetting, Destroy config done

//! All the tests need the service to be launched
TEST_CASE("All case")
{
    Albinos::Config cfg_to_retrieve = Albinos::Config("None");
    Albinos::Config albinos_cfg = Albinos::Config("foo");
    Albinos::Config* ptr = &albinos_cfg;
    Albinos::Config* retrieve_ptr = &cfg_to_retrieve;

    //! Creation config
    CHECK_EQ(Albinos::createConfig("toto", &ptr), Albinos::SUCCESS);

    //! Get key
    Albinos::Key key;
    CHECK_EQ(Albinos::getConfigKey(ptr, &key), Albinos::SUCCESS);
    std::string str("key is: [ " + std::string(key.data) + " ]");
    MESSAGE(str);
    CHECK(not std::string(key.data).empty());
    WARN_GT(key.size, 0);


    //! Set setting
    CHECK_EQ(Albinos::setSetting(ptr, "foo", "42bar"), Albinos::SUCCESS);

    //! Get setting size
    std::size_t foo_setting_size;
    CHECK_EQ(Albinos::getSettingSize(ptr, "foo", &foo_setting_size), Albinos::SUCCESS);
    CHECK_EQ(foo_setting_size, 5ull);

    //! Get setting value
    std::string foo_setting_value;
    foo_setting_value.resize(foo_setting_size);
    CHECK_EQ(Albinos::getSettingValue(ptr, "foo", foo_setting_value.data(), foo_setting_size), Albinos::SUCCESS);
    CHECK_EQ(foo_setting_value, "42bar");

    //! Alias
    CHECK_EQ(Albinos::setSettingAlias(ptr, "foo", "42foo"), Albinos::SUCCESS);

    CHECK_EQ(Albinos::unsetAlias(ptr, "42foo"), Albinos::SUCCESS);
    //! Readonly key
    Albinos::Key readonly_key;
    CHECK_EQ(Albinos::getReadOnlyConfigKey(ptr, &readonly_key), Albinos::SUCCESS);
    std::string readonly_key_str("key is: [ " + std::string(readonly_key.data) + " ]");
    MESSAGE(readonly_key_str);
    CHECK(not std::string(readonly_key.data).empty());
    WARN_GT(readonly_key.size, 0);

    CHECK_EQ(Albinos::getConfig(key, &retrieve_ptr), Albinos::SUCCESS);

    //! RemoveSetting
    CHECK_EQ(Albinos::removeSetting(ptr, "foo"), Albinos::SUCCESS);
    //! Config Destruction
    CHECK_EQ(Albinos::destroyConfig(ptr), Albinos::SUCCESS);
}