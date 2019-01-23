# include "Config.hpp"

///
/// \todo handle event
/// \todo handle status
///
void LibConfig::Config::parseResponse(json const &data)
{
  std::string status;
  try {
    status = data.at("REQUEST_STATE").get<std::string>();
  } catch (...) {
    // if the data received do not contain 'REQUEST_STATE', it's an event
    std::string setting = data.at("SETTING_NAME").get<std::string>();
    std::string newValue = data.at("UPDATE").get<std::string>();
    bool isDelete = data.at("DELETE").get<bool>();
    return;
  }
  std::cout << "request status " << status << std::endl;
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
  socket->on<uvw::DataEvent>([this](const uvw::DataEvent &dataEvent, uvw::PipeHandle &){
    std::cout << "Data received" << std::endl;
    std::cout << std::string(dataEvent.data.get(), dataEvent.length) << std::endl;
    try {
      parseResponse(json::parse(std::string(dataEvent.data.get(), dataEvent.length)));
    } catch (...) {
      std::cout << "Error in data received" << std::endl;
      // throw ;
    }
  });
  socket->on<uvw::WriteEvent>([this](const uvw::WriteEvent &, uvw::PipeHandle &sock){
    std::cout << "Data sent" << std::endl;
    sock.read();
    socketLoop->run<uvw::Loop::Mode::ONCE>();
  });
  std::cout << "Trying to connect to " << socketPath << std::endl;
  socket->connect(socketPath);
  socketLoop->run<uvw::Loop::Mode::ONCE>();
}

void LibConfig::Config::sendJson(const json& data)
{
  std::string requestStr = data.dump();
  char *buff = new char[requestStr.size()];
  requestStr.copy(buff, requestStr.size());
  socket->write(buff, requestStr.size());
  delete buff;
  socketLoop->run<uvw::Loop::Mode::ONCE>();
}

///
/// \todo get response and set-up name and id
///
void LibConfig::Config::loadConfig(Key const &givenKey)
{
  json request;
  request["REQUEST_NAME"] = "CONFIG_LOAD";

  switch (givenKey.type) {

  case LibConfig::READ_WRITE:
    key = {new char[givenKey.size], givenKey.size, LibConfig::READ_WRITE};
    std::memcpy(key->data, givenKey.data, givenKey.size);
    request["CONFIG_KEY"] = std::string(static_cast<char*>(key->data), key->size);
    break;

  case LibConfig::READ_ONLY:
    roKey = {new char[givenKey.size], givenKey.size, LibConfig::READ_ONLY};
    std::memcpy(key->data, givenKey.data, givenKey.size);
    request["READONLY_CONFIG_KEY"] = std::string(static_cast<char*>(roKey->data), roKey->size);
    break;

  default:
    assert(false); // unknow key type
  }
  sendJson(request);
}

///
/// \todo get response and set-up keys
///
LibConfig::Config::Config(std::string const &name)
  : name(name)
{
  initSocket();
  json request;
  request["CONFIG_NAME"] = name;
  request["REQUEST_NAME"] = "CONFIG_CREATE";
  sendJson(request);
  // key must be setted up there
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
    socketLoop->run<uvw::Loop::Mode::ONCE>();
    if (key)
      delete key->data;
    if (roKey)
      delete roKey->data;
  }
}

LibConfig::ReturnedValue LibConfig::Config::getKey(Key *configKey) const
{
  if (!key)
    return KEY_NOT_INITIALIZED;
  configKey->data = new char[key->size];
  std::memcpy(configKey->data, key->data, key->size);
  configKey->size = key->size;
  configKey->type = READ_WRITE;
  return SUCCESS;
}

LibConfig::ReturnedValue LibConfig::Config::getReadOnlyKey(Key *configKey) const
{
  if (!roKey)
    return KEY_NOT_INITIALIZED;
  configKey->data = new char[roKey->size];
  std::memcpy(configKey->data, roKey->data, roKey->size);
  configKey->size = roKey->size;
  configKey->type = READ_ONLY;
  return SUCCESS;
}

///
/// \todo implementation
///
LibConfig::ReturnedValue LibConfig::Config::getSettingValue(char const *settingName, char *value, size_t valueSize) const
{
  (void)settingName;
  (void)value;
  (void)valueSize;
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
