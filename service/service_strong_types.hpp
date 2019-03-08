//
// Created by milerius on 05/03/19.
//

#pragma once

#include "st/st.hpp"

namespace raven
{
  using config_id_st = st::type<int, struct config_id_tag, st::arithmetic>;
  using config_key_st = st::type<std::string, struct config_key_tag,
      st::equality_comparable,
      st::addable_with<const char *>,
      st::addable_with<char *>>;
  using db_statement_st = st::type<const char *, struct db_statement_tag>;
  using table_ascii_st = st::type<const char *, struct table_ascii_tag>;
}