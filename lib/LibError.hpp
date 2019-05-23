#pragma once


class LibError
{

public:

  LibError(int errorCode = -1);
  ~LibError(void) throw() {}

  int getCode(void) const throw();

private:

  int code;

};
