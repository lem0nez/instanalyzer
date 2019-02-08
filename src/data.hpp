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

#include <set>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

#include "location.hpp"
#include "term.hpp"

class Data {
public:
  typedef std::string (*val_parser)(const nlohmann::json&);

  static void show_profile_info(const std::string&);
  static void show_location_info(const std::string& profile,
      const unsigned int radius = Location::get_default_radius());

private:
  struct LocationGroup {
    std::string name;
    std::vector<std::string*> address_tree;
  };

  static std::set<Location::Coord> get_coords(const std::string& profile,
      const unsigned int radius = Location::get_default_radius());

  static const std::vector<std::pair<std::string, val_parser>> m_profile_data;
};
