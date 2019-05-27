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
# include <map>
# include "LibError.hpp"
# include "Albinos.h"
# include "uvw.hpp"
# include "json.hpp"
# include "KeyWrapper.hpp"
# include "Subscription.hpp"

namespace Albinos
{
  class Config
  {

    static constexpr uvw::TimerHandle::Time writeTimeout{std::chrono::duration<uint64_t, std::milli>(200)};

    using json = nlohmann::json;

    std::optional<ReturnedValue> irrecoverable;

    std::optional<std::string> name;
    uint32_t configId;
    std::string lastRequestedValue;
    std::map<std::string, Subscription*> settingsSubscriptions;
    std::vector<SettingUpdatedData> settingsUpdates;
    bool waitingForResponse{false};

    std::optional<KeyWrapper> key;
    std::optional<KeyWrapper> roKey;
    std::unique_ptr<std::vector<std::string>> settingNames;

    std::shared_ptr<uvw::Loop> socketLoop{uvw::Loop::getDefault()};
    std::shared_ptr<uvw::PipeHandle> socket{socketLoop->resource<uvw::PipeHandle>()};
    std::shared_ptr<uvw::TimerHandle> timer{socketLoop->resource<uvw::TimerHandle>()};

    void initSocket();
    void sendJson(json const &data) const;

    void parseResponse(json const &data);

    void loadConfig(KeyWrapper const &givenKey);

  public:

    Config(std::string const &name);
    Config(Key const &key);
    Config(uint32_t configId);
    ~Config();

    ReturnedValue getKey(Key *configKey) const;
    ReturnedValue getReadOnlyKey(Key *configKey) const;
    ReturnedValue getSettingValue(char const *settingName, char *value, size_t valueSize) const;
    ReturnedValue getSettingSize(char const *settingName, size_t *size) const;

    ReturnedValue setSetting(char const *name, char const *value);
    ReturnedValue setSettingAlias(char const *name, char const *aliasName);

    ReturnedValue unsetAlias(char const *aliasName);
    ReturnedValue removeSetting(char const *name);

    ReturnedValue include(Key *inheritFrom, int position);
    ReturnedValue uninclude(Key *otherConfig, int position);

    ReturnedValue getDependencies(Config **deps, size_t *size) const;
    ReturnedValue getLocalSettings(Setting **settings, size_t *size) const;
    ReturnedValue getLocalSettingsNames(char const * const **names) const;
    ReturnedValue getLocalAliases(Alias **aliases, size_t *size) const;

    ReturnedValue deleteConfig() const;

    ReturnedValue subscribeToSetting(char const *settingName, void *data, FCPTR_ON_CHANGE_NOTIFIER onChange, Subscription **subscription);
    ReturnedValue pollSubscriptions();

  };
}
