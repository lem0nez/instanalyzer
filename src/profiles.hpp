/*
 * Copyright © 2019 Nikita Dudko. All rights reserved.
 * Contacts: <nikita.dudko.95@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "boost/regex.hpp"

#include "instanalyzer.hpp"

class Profiles {
public:
  static void init() noexcept(false);
  inline static std::filesystem::path get_profiles_path() {
    return Instanalyzer::get_work_path() / "profiles";
  }

  static void check(const std::string&);
  static void update(const std::string&);
  static void remove_unused_files(const std::string&);

private:
  struct MsgUpd {
    boost::regex msg_regex;
    std::string replacement;
  };

  struct ErrUpd {
    boost::regex err_regex;
    std::string replacement;
    bool is_critical;
  };

  static const std::vector<MsgUpd> m_msgs_upd;
  static const std::vector<ErrUpd> m_errs_upd;
};
