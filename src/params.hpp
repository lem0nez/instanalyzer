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

#include <map>
#include <string>
#include <vector>

class Params {
public:
  static void process_params(const std::vector<std::string>&);
  static void show_help();
  static void show_version();

private:
  enum Parameters {
    PARAM_PROFILE_INFO,
    PARAM_PROFILE_LOCATION,
    PARAM_PROFILE_COMMENTATORS,
    PARAM_COMMENTATOR_INFO,
    PARAM_TOP_POSTS,
    PARAM_TAGGED_PROFILES,
    PARAM_UPDATE_PROFILE,
    PARAM_GEOCODER,
    PARAM_THEME,
    PARAM_UPDATE,
    PARAM_VERSION,
    PARAM_HELP
  };

  struct ParamInfo {
    std::vector<std::string> names;
    std::string info;
    bool is_main_param;
    bool allow_multiple = false;
    // Additional value for parameter.
    std::string val = "";
  };

  static const std::map<Parameters, ParamInfo> m_params;
};
