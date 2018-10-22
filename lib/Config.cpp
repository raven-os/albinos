# include "Config.hpp"

///
/// \todo better error management
///
void LibConfig::Config::initSocket()
{
  std::string socketPath = (std::filesystem::temp_directory_path() / "raven-os_service_libconfig.sock").string();
  ReturnedValue rv = UNKNOWN;

  socket->on<uvw::ErrorEvent>([&rv](const uvw::ErrorEvent&e, uvw::PipeHandle&) {
				if (e.code() == UV_ENOENT)
				  rv = SOCKET_NOT_FOUND;
				else
				  rv = CONNECTION_ERROR;
				std::cout << "ERROR" << std::endl;
			      });
  socket->once<uvw::ConnectEvent>([&rv](const uvw::ConnectEvent&, uvw::PipeHandle&) {
				    rv = SUCCESS;
				    std::cout << "All succeed" << std::endl;
				 });
  std::cout << "Try to connect to " << socketPath << std::endl;
  socket->connect(socketPath);
  socketLoop->run();
  if (rv != SUCCESS)
    throw std::exception();
}

///
/// \todo implementation
///
LibConfig::Config::Config(std::string const &name)
  : name(name)
{
  initSocket();
}

LibConfig::Config::Config(Id const *id)
  : id(*id)
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
LibConfig::ReturnedValue LibConfig::Config::getId(Id *configId) const
{
  (void)configId;
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
