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

#include "instanalyzer.hpp"

#include <fstream>
#include <iostream>
#include <map>

#include "modules.hpp"
#include "term.hpp"

using namespace std;
using namespace nlohmann;

filesystem::path Instanalyzer::m_work_path;
json Instanalyzer::m_config;

void Instanalyzer::init() {
  using namespace filesystem;

  m_work_path = path(getenv("HOME")) / ".instanalyzer";
  if (!directory_entry(m_work_path).exists()) {
    create_directory(m_work_path);
  }

  ifstream ifs(m_work_path / "config.json");
  if (ifs.fail()) {
    m_config = "{}"_json;
  } else {
    ifs >> m_config;
  }

  if (Term::is_colored()) {
    Term::init(request_theme());
  }

  try {
    Modules::init_interpreter();
  } catch (const exception& e) {
    msg(MSG_ERR, e.what());
    exit(EXIT_FAILURE);
  }

  if (!directory_entry(Modules::get_instaloader_path()).exists()) {
    msg(MSG_INFO,
        "Instaloader script doesn't exist, updating of modules required.");
    try {
      Modules::update_modules();
    } catch (const exception& e) {
      msg(MSG_ERR, e.what());
      exit(EXIT_FAILURE);
    }
  }
}

bool Instanalyzer::request_theme() {
  if (!get_val("use_dark_theme").empty()) {
    try {
      return stoi(get_val("use_dark_theme"));
    } catch (const exception& e) {
      msg(MSG_ERR, e.what());
      exit(EXIT_FAILURE);
    }
  }

  Term::init(true);
  cout << Term::process_colors(
      "Choose theme:\n  #{gray_out}1. For dark terminals;#{reset}") << endl;
  Term::set_dark_theme(false);
  cout << Term::process_colors(
      "  #{gray_out}2. For light terminals.#{reset}\nitem> #{bold}") << flush;

  unsigned short item;

  while (!(cin >> item) || item == 0 || item > 2) {
    cin.clear();
    cin.ignore(1000, '\n');
    cout << Term::process_colors(
        "#{reset}Invalid item! Try again.\nitem> #{bold}") << flush;
  }

  cout << Term::reset() << flush;
  bool use_dark_theme = item == 1 ? true : false;
  set_val("use_dark_theme", to_string(use_dark_theme));
  return use_dark_theme;
}

void Instanalyzer::parse_params(const vector<string>& t_params) {}

void Instanalyzer::msg(const Massages t_msg, const string& str) {
  static const map<Massages, string> msg_prefixes = {
    {MSG_INFO, "#{bold}#{blue_out}INFO#{reset} #{bold}|#{reset} "},
    {MSG_ERR, "\n#{bold}#{red_out}ERROR#{reset} #{bold}|#{reset} "},
    {MSG_WARN, "#{bold}#{orange_out}WARNING#{reset} #{bold}|#{reset} "}
  };
  cout << Term::reset() + Term::process_colors(msg_prefixes.at(t_msg)) + str
      << endl;
}

string Instanalyzer::get_val(const string& t_key) {
  return m_config.value(t_key, "");
}

void Instanalyzer::set_val(const string& t_key, const string& t_val) {
  m_config[t_key] = t_val;
  ofstream ofs(get_work_path() / "config.json");
  ofs << m_config.dump(4) << endl;
}
