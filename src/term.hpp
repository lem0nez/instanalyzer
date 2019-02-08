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
#include <set>
#include <string>

class Term {
public:
  enum Color {
    COL_WHITE,
    COL_RED,
    COL_ORANGE,
    COL_YELLOW,
    COL_CREAM,
    COL_GREEN,
    COL_BLUE,
    COL_GRAY,
    COL_BLACK
  };

  struct ColorInfo {
    std::string name;
    // ESC codes for light and dark terminals.
    unsigned int code_dark;
    unsigned int code_light;
  };

  static void init(const bool is_dark);
  static bool is_colored();
  static unsigned int get_columns();

  inline static void set_dark_theme(const bool t_is_dark) {
    m_is_dark = t_is_dark;
  };
  inline static bool is_dark_theme() { return m_is_dark; }

  // Colors have following format: #{col_set}, where:
  // col - color name (one of "m_colors" set);
  // set - "out" (outline) or "fill" (fill background).
  //
  // Also available following marks:
  // #{bold} - set text to bold;
  // #{reset} - reset all colors and font style.
  static std::string process_colors(std::string);
  static std::string get_color(const Color, const bool is_fill = false);
  static std::string reset();
  static std::string bold();

  inline static std::string clear_line() {
    return '\r' + std::string(get_columns(), ' ') + '\r' + reset();
  };

private:
  static const std::set<std::string> m_colored_terms;
  static const std::map<Color, ColorInfo> m_colors;

  static bool m_is_colored;
  static bool m_is_dark;
};
