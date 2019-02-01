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

#include "term.hpp"

#include <algorithm>
#include <unistd.h>
#include <sys/ioctl.h>

#include "boost/algorithm/string/replace.hpp"
#include "boost/regex.hpp"

using namespace std;

bool Term::m_is_colored;
bool Term::m_is_dark;

const set<string> Term::m_colored_terms = {
  "ansi", "color", "console", "cygwin", "gnome", "konsole", "kterm",
  "linux", "msys", "putty", "rxvt", "screen", "vt100", "xterm"
};

const map<Term::Color, Term::ColorInfo> Term::m_colors = {
  {COL_WHITE, {"white", 255, 231}}, {COL_RED, {"red", 197, 197}},
  {COL_ORANGE, {"orange", 208, 202}}, {COL_YELLOW, {"yellow", 220, 214}},
  {COL_CREAM, {"cream", 223, 218}}, {COL_GREEN, {"green", 42, 40}},
  {COL_BLUE, {"blue", 33, 27}}, {COL_GRAY, {"gray", 250, 245}},
  {COL_BLACK, {"black", 235, 232}}
};

void Term::init(const bool t_is_dark) {
  m_is_colored = is_colored();
  set_dark_theme(t_is_dark);
}

bool Term::is_colored() {
  const string env = getenv("TERM");
  return any_of(m_colored_terms.cbegin(), m_colored_terms.cend(),
      [&] (const string& term) { return env.find(term) != string::npos; });
}

unsigned int Term::get_columns() {
  struct winsize ws;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
  return ws.ws_col;
}

string Term::process_colors(string str) {
  using namespace boost;

  if (!m_is_colored) {
    str = regex_replace(str, regex("#\\{[[:alpha:]_]+\\}"), "");
  } else {
    const map<string, string> marks = {
      {"bold", bold()}, {"reset", reset()}
    };
    const map<string, string> color_types = {
      {"_out", "\x1b[38;5;"}, {"_fill", "\x1b[48;5;"}
    };

    // Replace colors.
    for (const auto& c : m_colors) {
      for (const auto& t : color_types) {
        const string& mark = "#{" + c.second.name + t.first + "}";
        const string& code = t.second + to_string(m_is_dark ?
            c.second.code_dark : c.second.code_light) + "m";
        ireplace_all(str, mark, code);
      }
    }

    // Process additional marks.
    for (const auto& m : marks) {
      ireplace_all(str, "#{" + m.first + "}", m.second);
    }
  }
  return str;
}

string Term::get_color(const Color col, const bool is_fill) {
  if (!m_is_colored) {
    return "";
  }

  const string& code = is_fill ? "\x1b[48;5;" : "\x1b[38;5;";
  const ColorInfo& data = m_colors.at(col);
  return code +
      to_string(m_is_dark ? data.code_dark : data.code_light) + 'm';
}

string Term::reset() {
  return m_is_colored ? "\x1b[0m" : "";
}

string Term::bold() {
  return m_is_colored ? "\x1b[1m" : "";
}
