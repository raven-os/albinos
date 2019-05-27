# include "Config.hpp"

///
/// \todo handle event
///
void Albinos::Config::parseResponse(json const &data)
{
  std::string status;
  try {
    status = data.at("REQUEST_STATE").get<std::string>();
  } catch (...) {
    // if the data received do not contain 'REQUEST_STATE', it's an event
    ModifType modif;
    std::string modifStr = data.at("SUBSCRIPTION_EVENT_TYPE").get<std::string>();
    if (modifStr == "UPDATE")
      modif = UPDATE;
    else if (modifStr == "DELETE")
      modif = DELETE;
    else
      throw LibError(INVALID_REPONSE_FROM_SERVICE);
    settingsUpdates.push_back({data.at("SETTING_NAME").get<std::string>(), modif});
    if (waitingForResponse)
      socketLoop->run<uvw::Loop::Mode::ONCE>();
    return;
  }

  try {
    lastRequestedValue = data.at("SETTING_VALUE").get<std::string>();
    return;
  } catch (...) {
  }

  try {
    name = data.at("CONFIG_NAME").get<std::string>();
    configId = data.at("CONFIG_ID").get<uint32_t>();
    return;
  } catch (...) {
  }

  try {
    std::string keyStr = data.at("CONFIG_KEY").get<std::string>();
    std::string roKeyStr = data.at("READONLY_CONFIG_KEY").get<std::string>();
    key.emplace(keyStr, READ_WRITE);
    roKey.emplace(roKeyStr, READ_ONLY);
    return;
  } catch (...) {
  }
}

///
/// \todo better error management
///
void Albinos::Config::initSocket()
{
  std::string socketPath = (std::filesystem::temp_directory_path() / "raven-os_service_albinos.sock").string();

  socket->on<uvw::ErrorEvent>([this](const uvw::ErrorEvent&e, uvw::PipeHandle&) {
    std::cout << "Error" << std::endl;
    switch (e.code()) {
      /// \todo catch relevant value from libuv http://docs.libuv.org/en/v1.x/errors.html
    default:
      irrecoverable = UNKNOWN;
      throw LibError();
    }
  });
  socket->once<uvw::ConnectEvent>([](const uvw::ConnectEvent&, uvw::PipeHandle&) {
    //std::cout << "All succeed" << std::endl;
  });
  socket->on<uvw::DataEvent>([this](const uvw::DataEvent &dataEvent, uvw::PipeHandle &) {
    std::string response = std::string(dataEvent.data.get(), dataEvent.length);
    //std::cout << "Data received" << std::endl;
    //std::cout << response << std::endl;
    parseResponse(json::parse(response));
  });
  socket->on<uvw::WriteEvent>([this](const uvw::WriteEvent &, uvw::PipeHandle &sock) {
    //std::cout << "Data sent" << std::endl;
    sock.read();
    // if the service doesn't respond after 200ms, stop the loop
    timer->start(writeTimeout, std::chrono::duration<uint64_t, std::milli>(1000));
    waitingForResponse = true;
    socketLoop->run<uvw::Loop::Mode::ONCE>();
    waitingForResponse = false;
  });
  timer->on<uvw::TimerEvent>([this](const uvw::TimerEvent&, uvw::TimerHandle &handle) {
    //std::cout << "Timeout" << std::endl;
    socketLoop->stop();
    handle.stop();
  });
  //std::cout << "Trying to connect to " << socketPath << std::endl;
  socket->connect(socketPath);
  socketLoop->run<uvw::Loop::Mode::ONCE>();
}

void Albinos::Config::sendJson(const json& data) const
{
  std::string requestStr = data.dump();
  char *buff = new char[requestStr.size()];
  requestStr.copy(buff, requestStr.size());
  socket->write(buff, requestStr.size());
  delete buff;
  socketLoop->run<uvw::Loop::Mode::ONCE>();
}

void Albinos::Config::loadConfig(KeyWrapper const &givenKey)
{
  json request;
  request["REQUEST_NAME"] = "CONFIG_LOAD";

  switch (givenKey.type) {

  case Albinos::READ_WRITE:
    key = givenKey;
    request["CONFIG_KEY"] = std::string(key->data.get(), key->size);
    break;

  case Albinos::READ_ONLY:
    roKey = givenKey;
    request["READONLY_CONFIG_KEY"] = std::string(roKey->data.get(), roKey->size);
    break;

  default:
    assert(false); // unknow key type
  }
  sendJson(request);
}

Albinos::Config::Config(std::string const &name)
  : name(name)
{
  initSocket();
  json request;
  request["CONFIG_NAME"] = name;
  request["REQUEST_NAME"] = "CONFIG_CREATE";
  sendJson(request);
  loadConfig(*key);
}

Albinos::Config::Config(Key const &givenKey)
{
  initSocket();
  loadConfig(givenKey);
}

Albinos::Config::Config(uint32_t configId)
  : configId(configId)
{
  initSocket();
}

Albinos::Config::~Config()
{
  json request;
  request["REQUEST_NAME"] = "CONFIG_UNLOAD";
  request["CONFIG_ID"] = configId;
  sendJson(request);
  socket->close();
}

Albinos::ReturnedValue Albinos::Config::getKey(Key *configKey) const
{
  if (irrecoverable.has_value())
    return *irrecoverable;
  if (!key)
    return KEY_NOT_INITIALIZED;
  key->dupKey(*configKey);
  return SUCCESS;
}

Albinos::ReturnedValue Albinos::Config::getReadOnlyKey(Key *configKey) const
{
  if (irrecoverable.has_value())
    return *irrecoverable;
  if (!roKey)
    return KEY_NOT_INITIALIZED;
  roKey->dupKey(*configKey);
  return SUCCESS;
}

///
/// \todo handle error
///
Albinos::ReturnedValue Albinos::Config::getSettingValue(char const *settingName, char *value, size_t valueSize) const
{
  if (irrecoverable.has_value())
    return *irrecoverable;
  json request;
  request["REQUEST_NAME"] = "SETTING_GET";
  request["CONFIG_ID"] = configId;
  request["SETTING_NAME"] = settingName;
  sendJson(request);
  std::memcpy(value, lastRequestedValue.c_str(), std::min(valueSize, lastRequestedValue.length()));
  return SUCCESS;
}

///
/// \todo handle error
///
Albinos::ReturnedValue Albinos::Config::getSettingSize(char const *settingName, size_t *size) const
{
  if (irrecoverable.has_value())
    return *irrecoverable;
  json request;
  request["REQUEST_NAME"] = "SETTING_GET";
  request["CONFIG_ID"] = configId;
  request["SETTING_NAME"] = settingName;
  sendJson(request);
  *size = lastRequestedValue.length();
  return SUCCESS;
}

///
/// \todo handle error
///
Albinos::ReturnedValue Albinos::Config::setSetting(char const *name, char const *value)
{
  if (irrecoverable.has_value())
    return *irrecoverable;
  json request;
  request["REQUEST_NAME"] = "SETTING_UPDATE";
  request["CONFIG_ID"] = configId;
  request["SETTINGS_TO_UPDATE"][name] = value;
  sendJson(request);
  return SUCCESS;
}

///
/// \todo handle error
///
Albinos::ReturnedValue Albinos::Config::setSettingAlias(char const *name, char const *aliasName)
{
  if (irrecoverable.has_value())
    return *irrecoverable;
  json request;
  request["REQUEST_NAME"] = "ALIAS_SET";
  request["CONFIG_ID"] = configId;
  request["SETTING_NAME"] = name;
  request["ALIAS_NAME"] = aliasName;
  sendJson(request);
  return SUCCESS;
}

///
/// \todo handle error
///
Albinos::ReturnedValue Albinos::Config::unsetAlias(char const *aliasName)
{
  if (irrecoverable.has_value())
    return *irrecoverable;
  json request;
  request["REQUEST_NAME"] = "ALIAS_UNSET";
  request["ALIAS_NAME"] = aliasName;
  sendJson(request);
  return SUCCESS;
}

///
/// \todo implementation
///
Albinos::ReturnedValue Albinos::Config::removeSetting(char const *name)
{
  if (irrecoverable.has_value())
    return *irrecoverable;
  (void)name;
  return SUCCESS;
}

///
/// \todo implementation
///
Albinos::ReturnedValue Albinos::Config::include(Key *inheritFrom, int position)
{
  if (irrecoverable.has_value())
    return *irrecoverable;
  (void)inheritFrom;
  (void)position;
  return SUCCESS;
}

///
/// \todo implementation
///
Albinos::ReturnedValue Albinos::Config::uninclude(Key *inheritFrom, int position)
{
  if (irrecoverable.has_value())
    return *irrecoverable;
  (void)inheritFrom;
  (void)position;
  return SUCCESS;
}

///
/// \todo get errors
///
Albinos::ReturnedValue Albinos::Config::subscribeToSetting(char const *settingName, void *data, FCPTR_ON_CHANGE_NOTIFIER onChange, Subscription **subscription)
{
  if (irrecoverable.has_value())
    return *irrecoverable;
  json request;
  *subscription = new Subscription(settingName, onChange, data);
  settingsSubscriptions[settingName] = *subscription;
  request["REQUEST_NAME"] = "SUBSCRIBE_SETTING";
  request["CONFIG_ID"] = configId;
  request["SETTING_NAME"] = settingName;
  sendJson(request);
  return SUCCESS;
}

///
/// \todo implementation
///
Albinos::ReturnedValue Albinos::Config::getDependencies(Config **deps, size_t *size) const
{
  if (irrecoverable.has_value())
    return *irrecoverable;
  (void)deps;
  (void)size;
  return SUCCESS;
}

///
/// \todo implementation
///
Albinos::ReturnedValue Albinos::Config::getLocalSettings(Setting **settings, size_t *size) const
{
  if (irrecoverable.has_value())
    return *irrecoverable;
  (void)settings;
  (void)size;
  return SUCCESS;
}

///
/// \todo implementation
///
Albinos::ReturnedValue Albinos::Config::getLocalSettingsNames(char ***names) const
{
  if (irrecoverable.has_value())
    return *irrecoverable;
  (void)names;
  return SUCCESS;
}

///
/// \todo implementation
///
Albinos::ReturnedValue Albinos::Config::getLocalAliases(Alias **aliases, size_t *size) const
{
  if (irrecoverable.has_value())
    return *irrecoverable;
  (void)aliases;
  (void)size;
  return SUCCESS;
}

///
/// \todo implementation
///
Albinos::ReturnedValue Albinos::Config::deleteConfig() const
{
  if (irrecoverable.has_value())
    return *irrecoverable;
  return SUCCESS;
}

///
/// \todo error management
///
Albinos::ReturnedValue Albinos::Config::pollSubscriptions()
{
  if (irrecoverable.has_value())
    return *irrecoverable;
  while (socketLoop->run<uvw::Loop::Mode::NOWAIT>());
  while (!settingsUpdates.empty()) {
    settingsSubscriptions.at(settingsUpdates.back().name)->executeCallBack(settingsUpdates.back().modif);
    settingsUpdates.pop_back();
  }
  return SUCCESS;
}
