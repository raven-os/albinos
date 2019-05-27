#pragma once

# include "Albinos.h"

class LibError
{

public:

  LibError(Albinos::ReturnedValue errorCode = Albinos::UNKNOWN);
  ~LibError(void) throw() {}

  Albinos::ReturnedValue getCode(void) const throw();

private:

  Albinos::ReturnedValue code;

};
