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
# include "Libconfig.h"
# include "../vendor/uvw/src/uvw.hpp"

namespace LibConfig
{
  struct Config
  {
  private:

    std::string name;
    Id id;

    std::shared_ptr<uvw::Loop> socketLoop{uvw::Loop::getDefault()};
    std::shared_ptr<uvw::PipeHandle> socket{socketLoop->resource<uvw::PipeHandle>()};

    void initSocket();

  public:

    Config(std::string const &name);
    Config(Id const *id);
    ~Config();

    ReturnedValue getId(Id *configId) const;
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
