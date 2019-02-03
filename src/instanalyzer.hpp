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

#include "nlohmann/json.hpp"

class Instanalyzer {
public:
  enum Massages {
    MSG_INFO,
    MSG_ERR,
    MSG_WARN
  };

  static void init();
  // force - request user choice anyway.
  // Return true if need use dark theme, otherwise false.
  static bool request_theme(const bool force = false);

  static void msg(const Massages, const std::string&, const bool new_line = false);
  static std::string get_val(const std::string&);
  static void set_val(const std::string&, const std::string&);

  inline static std::filesystem::path get_work_path() { return m_work_path; }
  inline static std::string get_tmp_prefix() { return "/tmp/instanalyzer_"; }

private:
  static std::filesystem::path m_work_path;
  static nlohmann::json m_config;
};
