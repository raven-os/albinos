# include "Config.hpp"

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
  socket->on<uvw::DataEvent>([this](const uvw::DataEvent &data, uvw::PipeHandle &sock){
			       std::cout << "Data received" << std::endl;
			       try {
				 response = json::parse(std::string(data.data.get(), data.length));
			       } catch (std::exception) {
				 std::cout << "Error in response" << std::endl;
				 throw std::exception();
			       }
			     });
  std::cout << "Trying to connect to " << socketPath << std::endl;
  socket->connect(socketPath);
  socketLoop->run();
}

void LibConfig::Config::sendJson(const json& data)
{
  std::string requestStr = data.dump();
  char *buff = new char[requestStr.size()];
  requestStr.copy(buff, requestStr.size());
  socket->write(buff, requestStr.size());
  delete buff;
}

///
/// \todo implementation
///
LibConfig::Config::Config(std::string const &name)
  : name(name), key({nullptr, 0})
{
  initSocket();
  json request;
  request["config"] = name;
  request["order"] = "CONFIG_CREATE";
  request["provider"] = "";
  sendJson(request);
  socketLoop->run();
}

LibConfig::Config::Config(Key const *key)
  : key(*key)
{
  initSocket();
}

///
/// \todo implementation
///
LibConfig::Config::~Config()
{
}

///
/// \todo implementation
///
LibConfig::ReturnedValue LibConfig::Config::getKey(Key *configKey) const
{
  (void)configKey;
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
