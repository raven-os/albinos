#include <filesystem>
#include <loguru.hpp>
#include "service.hpp"

int main()
{
  loguru::add_file("/var/log/albinos.log", loguru::Append, loguru::Verbosity_INFO);
  loguru::add_file("/var/log/latest_albinos.log", loguru::Truncate, loguru::Verbosity_MAX);
  raven::service service;
  service.run();
  return 0;
}
