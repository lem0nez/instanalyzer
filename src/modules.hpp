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
#include <vector>
#include <string>

#include "instanalyzer.hpp"

class Modules {
public:
  // Callback for parsing output.
  typedef void (*parser_cb)(const std::string&);

  struct ZipModuleInfo {
    std::string name, url;
    std::vector<std::string> paths;
  };

  static void init_interpreter() noexcept(false);
  static void update_modules() noexcept(false);
  static void instaloader(const std::string& params,
      const parser_cb cb_out = nullptr, const parser_cb cb_err = nullptr);

  inline static std::filesystem::path get_modules_path() {
    return Instanalyzer::get_work_path() / "modules";
  }

  inline static std::filesystem::path get_instaloader_path() {
    return get_modules_path() / "instaloader.py";
  }

private:
  static const std::vector<ZipModuleInfo> m_zip_modules;
  static std::filesystem::path m_interpreter_path;
};
