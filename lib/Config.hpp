///
/// \date 14/10/2018
/// \brief main struct, represent a configuration
/// \bug
/// \see
/// \todo implement the struct
///

#pragma once

# include <string>
# include <filesystem>
# include <iostream>
# include <optional>
# include "Libconfig.h"
# include "uvw.hpp"
# include "json.hpp"

namespace LibConfig
{
  class Config
  {
  private:

    using json = nlohmann::json;

    json response;

    std::string name;
    std::string configId;
    std::optional<Key> key;
    std::optional<Key> roKey;

    std::shared_ptr<uvw::Loop> socketLoop{uvw::Loop::getDefault()};
    std::shared_ptr<uvw::PipeHandle> socket{socketLoop->resource<uvw::PipeHandle>()};

    void initSocket();
    void sendJson(const json& data);

  public:

    Config(std::string const &name);
    Config(Key const *key);
    ~Config();

    ReturnedValue getKey(Key *configKey) const;
    ReturnedValue getSettingValue(char const *settingName, char *value, size_t valueSize) const;
    ReturnedValue getSettingSize(char const *settingName, size_t *size) const;

    ReturnedValue setSetting(char const *name, char const *value);
    ReturnedValue setSettingAlias(char const *name, char const *aliasName);

    ReturnedValue unsetAlias(char const *aliasName);
    ReturnedValue removeSetting(char const *name);

    ReturnedValue include(Config const *inheritFrom);

    ReturnedValue subscribeToSetting(char const *settingName, void *data, FCPTR_ON_CHANGE_NOTIFIER onChange, Subscription **subscription);

  };
}
