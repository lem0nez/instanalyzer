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
#include "profile.hpp"
#include "term.hpp"

class Data {
public:
  typedef std::string (*val_parser)(const nlohmann::json&);

  static void show_profile_info(const Profile&);
  static void show_location_info(const Profile& profile,
      const unsigned int& radius = Location::get_default_radius());

  inline static int get_default_posts_count() { return 10; }
  // If "count" is negative number, then will be printed less liked posts.
  static void show_posts_top(const Profile& profile,
      const int count = get_default_posts_count());

  static std::set<Profile::TaggedProfile> get_tagged_profiles(const Profile&);
  static void show_tagged_profiles(const Profile&);

private:
  struct LocationGroup {
    std::string name;
    std::vector<std::string*> address_tree;
  };

  static std::set<Location::Coord> get_coords(const Profile& profile,
      const unsigned int& radius = Location::get_default_radius());

  static const std::vector<std::pair<std::string, val_parser>> m_profile_data;
};
