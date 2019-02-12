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

#include <ctime>
#include <fstream>
#include <iostream>
#include <map>

#include "location.hpp"
#include "modules.hpp"
#include "profile.hpp"
#include "term.hpp"

using namespace std;
using namespace nlohmann;

filesystem::path Instanalyzer::m_work_path;
json Instanalyzer::m_config;

void Instanalyzer::init() {
  using namespace filesystem;

#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
  m_work_path = path(getenv("APPDATA")) / "Instanalyzer";
#else
  m_work_path = path(getenv("HOME")) / ".instanalyzer";
#endif

  try {
    if (!directory_entry(m_work_path).exists()) {
      create_directory(m_work_path);
    }
  } catch (const exception& e) {
    msg(MSG_ERR, Term::process_colors("Error occurred while initializing: "
        "\"#{gray_out}" + string(e.what()) + "#{reset}\"."));
    exit(EXIT_FAILURE);
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

  manage_cache();

  try {
    Location::init();
    Modules::init_interpreter();
    Profile::init();
  } catch (const exception& e) {
    msg(MSG_ERR, e.what());
    exit(EXIT_FAILURE);
  }

  if (!directory_entry(Modules::get_instaloader_path()).exists()) {
    msg(MSG_INFO,
        "Instaloader script doesn't exist, update of modules required.");
    try {
      Modules::update_modules();
    } catch (const exception& e) {
      msg(MSG_ERR, e.what(), true);
      exit(EXIT_FAILURE);
    }
  }
}

string Instanalyzer::get_tmp_prefix() {
#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
  return filesystem::path(getenv("TEMP")) / "instanalyzer_";
#else
  return "/tmp/instanalyzer_";
#endif
}

void Instanalyzer::manage_cache() {
  using namespace filesystem;

  if (!directory_entry(get_cache_path()).exists()) {
    create_directory(get_cache_path());
    set_pref("last_cache_clean", to_string(time(nullptr)));
    return;
  }

  constexpr int INTERVAL_DAYS = 30;
  time_t last_clean = 0;

  try {
    last_clean = stoi(get_pref("last_cache_clean"));
  } catch (const exception&) {}

  if (last_clean + INTERVAL_DAYS * 24 * 3600 < time(nullptr)) {
    cout << "\rCleaning cache..." << flush;
    error_code e;
    remove_all(get_cache_path(), e);
    cout << Term::clear_line() << flush;

    try {
      create_directory(get_cache_path());
    } catch (const exception& e) {
      msg(MSG_ERR, Term::process_colors("Can't create cache directory: "
          "\"#{gray_out}" + string(e.what()) + "#{reset}\"."));
      exit(EXIT_FAILURE);
    }

    set_pref("last_cache_clean", to_string(time(nullptr)));
    msg(MSG_INFO, Term::process_colors("Cache cleaned (interval: #{green_out}" +
        to_string(INTERVAL_DAYS) + "#{reset} days)."));
  }
}

bool Instanalyzer::request_theme(const bool& t_force) {
  if (!t_force && !get_pref("use_dark_theme").empty()) {
    try {
      return stoi(get_pref("use_dark_theme"));
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
  set_pref("use_dark_theme", to_string(use_dark_theme));
  return use_dark_theme;
}

void Instanalyzer::msg(const Massages& t_msg, const string& str,
    const bool& t_new_line) {
  static const map<Massages, string> msg_prefixes = {
    {MSG_INFO, "#{bold}#{blue_out}INFO#{reset} #{bold}|#{reset} "},
    {MSG_ERR, "#{bold}#{red_out}ERROR#{reset} #{bold}|#{reset} "},
    {MSG_WARN, "#{bold}#{orange_out}WARNING#{reset} #{bold}|#{reset} "}
  };
  cout << (t_new_line ? "\n" : "") + Term::reset() +
      Term::process_colors(msg_prefixes.at(t_msg)) + str << endl;
}

string Instanalyzer::get_pref(const string& t_key) {
  return m_config.value(t_key, "");
}

void Instanalyzer::set_pref(const string& t_key, const string& t_val) {
  m_config[t_key] = t_val;
  ofstream ofs(get_work_path() / "config.json");
  ofs << m_config.dump(4) << endl;
}
