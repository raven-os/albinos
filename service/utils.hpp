//
// Created by milerius on 05/03/19.
//

#pragma once

#include <string>
#include <fstream>
#include "service_strong_types.hpp"

namespace raven
{
  inline constexpr const table_ascii_st table_ascii{
      R"(!"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~')"};
  inline constexpr const std::size_t table_ascii_len = 94;

  inline std::string random_string(std::size_t random_string_len = 32) noexcept
  {
      std::random_device rd;
      std::mt19937 engine(rd());
      std::uniform_int_distribution<> dist(0, table_ascii_len);
      std::string ret;
      ret.resize(random_string_len);
      std::generate(begin(ret), end(ret), [&engine, &dist]() { return table_ascii.value()[dist(engine)]; });
      return ret;
  }
}

#ifdef DOCTEST_LIBRARY_INCLUDED
#include <algorithm>
#include <vector>

TEST_CASE ("random string")
{
    auto test_case = [](auto &&values) {
        std::generate(begin(values), end(values), []() { return raven::random_string(); });
        std::sort(begin(values), end(values));
        auto result = std::adjacent_find(begin(values), end(values));
        WARN(result == values.end());
    };
        SUBCASE("10 elements") {
        std::vector<std::string> values(10);
        test_case(values);
    }
        SUBCASE("100 elements") {
        std::vector<std::string> values(100);
        test_case(values);
    }
        SUBCASE("1000 elements") {
        std::vector<std::string> values(1000);
        test_case(values);
    }

        SUBCASE("10000 elements") {
        std::vector<std::string> values(10000);
        test_case(values);
    }
        SUBCASE("100000 elements") {
        std::vector<std::string> values(100000);
        test_case(values);
    }
}

#endif