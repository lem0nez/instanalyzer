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

#include <ctime>
#include <map>
#include <set>
#include <string>

#include "nlohmann/json.hpp"

#include "profile.hpp"

class Comment {
public:
  Comment() = default;

  inline bool operator<(const Comment& rhs) const {
    return m_creation_time < rhs.m_creation_time;
  }

  inline std::string get_id() const { return m_id; }
  inline std::string get_text() const { return m_text; }
  inline std::string get_post_shortcode() const { return m_post_shortcode; }
  inline Profile get_profile() const { return m_profile; }
  inline unsigned int get_likes() const { return m_likes; }
  inline std::time_t get_creation_time() const { return m_creation_time; }
  inline bool is_spam() const { return m_is_spam; }

  inline void set_id(const std::string& t_id) { m_id = t_id; }
  inline void set_text(const std::string& t_text) { m_text = t_text; }
  inline void set_post_shortcode(const std::string& t_shortcode) {
    m_post_shortcode = t_shortcode;
  }
  inline void set_profile(const Profile& t_profile) {
    m_profile = t_profile;
  }
  inline void set_likes(const unsigned int& t_likes) { m_likes = t_likes; }
  inline void set_creation_time(const std::size_t& t_creation_time) {
    m_creation_time = t_creation_time;
  }
  inline void set_spam(const bool& t_is_spam) { m_is_spam = t_is_spam; }

  static std::set<Comment> get_comments(const std::set<nlohmann::json>&);

  static void show_commentators(const Profile&);
  static void show_commentator_info(
      const Profile& owner, const std::string& commentator);
  static void print_comments(const std::set<Comment>& comments,
      const std::string& label = "");
  static void show_references(const Profile& owner,
      const std::string& commentator, std::set<Comment> comments = {});

private:
  std::string m_id, m_text, m_post_shortcode;
  Profile m_profile;

  unsigned int m_likes;
  std::time_t m_creation_time;
  bool m_is_spam;
};
