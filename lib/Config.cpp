# include "Config.hpp"

///
/// \todo handle event
/// \todo handle status
///
void LibConfig::Config::parseResponse(json const &data)
{
  std::string status;
  try {
    //status = data.at("REQUEST_STATE").get<std::string>();
    status = "STILL_NO_STATUS_SENT";
  } catch (...) {
    // if the data received do not contain 'REQUEST_STATE', it's an event
    std::string setting = data.at("SETTING_NAME").get<std::string>();
    std::string newValue = data.at("UPDATE").get<std::string>();
    bool isDelete = data.at("DELETE").get<bool>();
    return;
  }
  // std::cout << "request status " << status << std::endl;

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
void LibConfig::Config::initSocket()
{
  std::string socketPath = (std::filesystem::temp_directory_path() / "raven-os_service_libconfig.sock").string();

  socket->on<uvw::ErrorEvent>([](const uvw::ErrorEvent&e, uvw::PipeHandle&) {
    std::cout << "Error" << std::endl;
    throw std::exception();
  });
  socket->once<uvw::ConnectEvent>([](const uvw::ConnectEvent&, uvw::PipeHandle&) {
    std::cout << "All succeed" << std::endl;
  });
  socket->on<uvw::DataEvent>([this](const uvw::DataEvent &dataEvent, uvw::PipeHandle &) {
    std::string response = std::string(dataEvent.data.get(), dataEvent.length);
    std::cout << "Data received" << std::endl;
    std::cout << response << std::endl;
    try {
      parseResponse(json::parse(response));
    } catch (...) {
      std::cout << "Error in data received" << std::endl;
      // throw ;
    }
  });
  socket->on<uvw::WriteEvent>([this](const uvw::WriteEvent &, uvw::PipeHandle &sock) {
    std::cout << "Data sent" << std::endl;
    sock.read();
    // if the service doesn't respond after 200ms, stop the loop
    timer->start(writeTimeout, std::chrono::duration<uint64_t, std::milli>(1000));
    socketLoop->run<uvw::Loop::Mode::ONCE>();
  });
  timer->on<uvw::TimerEvent>([this](const uvw::TimerEvent&, uvw::TimerHandle &handle) {
    //std::cout << "Timeout" << std::endl;
    socketLoop->stop();
    handle.stop();
  });
  std::cout << "Trying to connect to " << socketPath << std::endl;
  socket->connect(socketPath);
  socketLoop->run<uvw::Loop::Mode::ONCE>();
}

void LibConfig::Config::sendJson(const json& data) const
{
  std::string requestStr = data.dump();
  char *buff = new char[requestStr.size()];
  requestStr.copy(buff, requestStr.size());
  socket->write(buff, requestStr.size());
  delete buff;
  socketLoop->run<uvw::Loop::Mode::ONCE>();
}

void LibConfig::Config::loadConfig(KeyWrapper givenKey)
{
  json request;
  request["REQUEST_NAME"] = "CONFIG_LOAD";

  switch (givenKey.type) {

  case LibConfig::READ_WRITE:
    key = givenKey;
    request["CONFIG_KEY"] = std::string(key->data.get(), key->size);
    break;

  case LibConfig::READ_ONLY:
    roKey = givenKey;
    request["READONLY_CONFIG_KEY"] = std::string(roKey->data.get(), roKey->size);
    break;

  default:
    assert(false); // unknow key type
  }
  sendJson(request);
}

LibConfig::Config::Config(std::string const &name)
  : name(name)
{
  initSocket();
  json request;
  request["CONFIG_NAME"] = name;
  request["REQUEST_NAME"] = "CONFIG_CREATE";
  sendJson(request);
  loadConfig(*key);
}

LibConfig::Config::Config(Key const &givenKey)
  : name("")
{
  initSocket();
  loadConfig(givenKey);
}

LibConfig::Config::~Config()
{
  if (key || roKey) {
    json request;
    request["REQUEST_NAME"] = "CONFIG_UNLOAD";
    request["CONFIG_ID"] = configId;
    sendJson(request);
    socket->close();
  }
}

LibConfig::ReturnedValue LibConfig::Config::getKey(Key *configKey) const
{
  if (!key)
    return KEY_NOT_INITIALIZED;
  key->dupKey(*configKey);
  return SUCCESS;
}

LibConfig::ReturnedValue LibConfig::Config::getReadOnlyKey(Key *configKey) const
{
  if (!roKey)
    return KEY_NOT_INITIALIZED;
  roKey->dupKey(*configKey);
  return SUCCESS;
}

///
/// \todo handle error
///
LibConfig::ReturnedValue LibConfig::Config::getSettingValue(char const *settingName, char *value, size_t valueSize) const
{
  json request;
  request["REQUEST_NAME"] = "SETTING_GET";
  request["CONFIG_ID"] = configId;
  request["SETTING_NAME"] = settingName;
  sendJson(request);
  std::memcpy(value, lastRequestedValue.c_str(), std::min(valueSize, lastRequestedValue.length()));
  return SUCCESS;
}

///
/// \todo implementation
///
LibConfig::ReturnedValue LibConfig::Config::getSettingSize(char const *settingName, size_t *size) const
{
  (void)settingName;
  (void)size;
  return SUCCESS;
}

///
/// \todo implementation
///
LibConfig::ReturnedValue LibConfig::Config::setSetting(char const *name, char const *value)
{
  (void)name;
  (void)value;
  return SUCCESS;
}

///
/// \todo implementation
///
LibConfig::ReturnedValue LibConfig::Config::setSettingAlias(char const *name, char const *aliasName)
{
  (void)name;
  (void)aliasName;
  return SUCCESS;
}

///
/// \todo implementation
///
LibConfig::ReturnedValue LibConfig::Config::unsetAlias(char const *aliasName)
{
  (void)aliasName;
  return SUCCESS;
}

///
/// \todo implementation
///
LibConfig::ReturnedValue LibConfig::Config::removeSetting(char const *name)
{
  (void)name;
  return SUCCESS;
}

///
/// \todo implementation
///
LibConfig::ReturnedValue LibConfig::Config::include(Config const *inheritFrom)
{
  (void)inheritFrom;
  return SUCCESS;
}

///
/// \todo implementation
///
LibConfig::ReturnedValue LibConfig::Config::subscribeToSetting(char const *settingName, void *data, FCPTR_ON_CHANGE_NOTIFIER onChange, Subscription **subscription)
{
  (void)settingName;
  (void)data;
  (void)onChange;
  (void)subscription;
  return SUCCESS;
}
