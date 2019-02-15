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
#include <set>
#include <string>

#include "comment.hpp"

class Post {
public:
  inline bool operator<(const Post& rhs) { return m_shortcode < rhs.m_shortcode; }

  inline std::string get_shortcode() const { return m_shortcode; }
  inline unsigned int get_likes() const { return m_likes; }
  inline std::time_t get_creation_time() const { return m_creation_time; }
  inline std::set<Comment> get_comments() const { return m_comments; }

  inline void set_shortcode(const std::string& t_shortcode) {
    m_shortcode = t_shortcode;
  }
  inline void set_likes(const unsigned int& t_likes) { m_likes = t_likes; }
  inline void set_creation_time(const std::time_t& t_time) {
    m_creation_time = t_time;
  }
  inline void set_comments(const std::set<Comment>& t_comments) {
    m_comments = t_comments;
  }

private:
  std::string m_shortcode;
  unsigned int m_likes;
  std::time_t m_creation_time;
  std::set<Comment> m_comments;
};
