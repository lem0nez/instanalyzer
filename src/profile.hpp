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
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "boost/regex.hpp"
#include "nlohmann/json.hpp"

#include "instanalyzer.hpp"

class Profile {
public:
  struct TaggedProfile {
    TaggedProfile() = default;
    TaggedProfile(const Profile& t_profile):
        profile(std::make_shared<Profile>(t_profile)) {}

    inline bool operator <(const TaggedProfile& rhs) const {
      return *profile < *rhs.profile || post_shortcode < rhs.post_shortcode;
    }

    std::shared_ptr<Profile> profile;
    std::string post_shortcode;
    double x, y;
  };

  Profile() = default;
  Profile(const std::string& t_name): m_name(t_name) {}

  inline bool operator<(const Profile& rhs) const { return m_id < rhs.m_id; }

  inline std::string get_id() const { return m_id; }
  inline std::string get_name() const { return m_name; }
  inline std::string get_full_name() const { return m_full_name; }
  inline bool is_verified() const { return m_is_verified; }

  inline void set_id(const std::string& t_id) { m_id = t_id; }
  inline void set_name(const std::string& t_name) { m_name = t_name; }
  inline void set_full_name(const std::string& t_name) { m_full_name = t_name; }
  inline void set_verified(const bool& t_verified) { m_is_verified = t_verified; }

  void check() const;
  void update() const;
  void remove_unused_files() const;
  std::set<nlohmann::json> get_posts(const bool& use_cache = true) const;

  static void init() noexcept(false);
  inline static std::filesystem::path get_profiles_path() {
    return Instanalyzer::get_work_path() / "profiles";
  }

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

  std::string m_id, m_name, m_full_name;
  bool m_is_verified;

  static const unsigned int MAX_CACHED_POSTS;

  static const std::vector<MsgUpd> m_msgs_upd;
  static const std::vector<ErrUpd> m_errs_upd;
  static std::map<Profile, std::set<nlohmann::json>> m_cached_posts;
};
