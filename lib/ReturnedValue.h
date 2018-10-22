///
/// \date 17/10/2018
/// \brief contain all possible returned values for the API
/// \see
/// \todo
///

#pragma once

#ifdef __cplusplus
namespace LibConfig
{
#endif

  enum ReturnedValue
  {

   /// returned if error is not managed
   UNKNOWN,

   /// returned in case of success
   SUCCESS,

   /// returned if one of given parameters isn't good
   BAD_PARAMETERS,

   /// returned if socket file is not found
   SOCKET_NOT_FOUND,

   /// returned if an error occur when connecting
   CONNECTION_ERROR,
  };

#ifdef __cplusplus
}
#endif
