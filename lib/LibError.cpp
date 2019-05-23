# include "LibError.hpp"

LibError::LibError(int errorCode)
  : code(errorCode)
{}

int LibError::getCode(void) const throw()
{
  return errorCode;
}
