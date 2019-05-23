///
/// \date 14/10/2018
/// \brief allow the user to manage his callbacks
/// \bug
/// \see
/// \todo
///

#pragma once

# include "Albinos.h"
# include <string>

namespace Albinos
{
  struct SettingUpdatedData
  {
    std::string name;
    ModifType modif;
  };

  class Subscription
  {
  private:

    std::string associatedSetting;
    FCPTR_ON_CHANGE_NOTIFIER callBack;
    void *associatedData;

  public:

    Subscription(std::string const &associatedSetting, FCPTR_ON_CHANGE_NOTIFIER callBack, void *associatedData);
    ~Subscription();

    void executeCallBack(ModifType) const;
    std::string const &getAssociatedSetting() const;

  };
}
