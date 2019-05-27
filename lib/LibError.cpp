# include "LibError.hpp"

LibError::LibError(Albinos::ReturnedValue errorCode)
  : code(errorCode)
{}

Albinos::ReturnedValue LibError::getCode(void) const throw()
{
  return code;
}
