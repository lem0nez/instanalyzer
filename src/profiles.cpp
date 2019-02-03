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

#include "profiles.hpp"

#include <fstream>
#include <iostream>
#include <set>

#include "instanalyzer.hpp"
#include "modules.hpp"
#include "term.hpp"

using namespace std;

const vector<Profiles::MsgUpd> Profiles::m_msgs_upd = {
  {boost::regex("^\\s*\\[\\s*([^\\s\\/]+)\\s*\\/\\s*([^]\\s]+)\\s*].*$",
      boost::regex::extended), Term::clear_line() +
      "Downloaded #{blue_out}$1#{reset} of #{blue_out}$2#{reset} posts..."}
};

const vector<Profiles::ErrUpd> Profiles::m_errs_upd = {
  {boost::regex("^.+ ([^\\s]+) does not exist\\.$", boost::regex::extended),
      "Profile #{red_out}@$1#{reset} doesn't exist!", true},
  {boost::regex(".+ Max retries exceeded with url:.+\\[retrying; skip with \\^C]$",
      boost::regex::extended), "Max connection retries exceeded.", false},
  {boost::regex("^([^:]+): --login=USERNAME required.$", boost::regex::extended),
      "Profile #{red_out}@$1#{reset} is private!", true}
};

void Profiles::init() {
  using namespace filesystem;

  if (!directory_entry(get_profiles_path()).exists()) {
    create_directory(get_profiles_path());
  }
}

void Profiles::update(const string& t_profile) {
  using namespace filesystem;

  cout << Term::process_colors(
      "Updating profile #{blue_out}@" + t_profile + "#{reset}...") << endl;

  try {
    remove_all(get_profiles_path() / t_profile);
  } catch (const exception&) {}

  const auto& fout = [] (const string& str) {
    for (const auto& m : m_msgs_upd) {
      const string& fmt_str = boost::regex_replace(
          str, m.msg_regex, Term::process_colors(m.replacement));
      if (fmt_str != str) {
        cout << fmt_str << flush;
      }
    }
  };

  static bool err_started = false;
  const auto& ferr = [] (const string& str) {
    if (!err_started) {
      for (const auto& e : m_errs_upd) {
        const string& fmt_str = boost::regex_replace(
            str, e.err_regex, Term::process_colors(e.replacement));
        if (!e.replacement.empty() && fmt_str != str) {
          cout << Term::clear_line() << flush;

          if (e.is_critical) {
            Instanalyzer::msg(Instanalyzer::MSG_ERR, fmt_str);
            exit(EXIT_FAILURE);
          } else {
            Instanalyzer::msg(Instanalyzer::MSG_WARN, fmt_str);
            return;
          }
        }
      }

      err_started = true;
      cout << Term::clear_line() << flush;
      Instanalyzer::msg(Instanalyzer::MSG_ERR, Term::process_colors(
          "Error occurred while updating profile:\n"
          " #{red_out}--- --- ---#{gray_out}\n") + str);
    } else {
      cout << str << endl;
    }
  };

  static const unsigned int MAX_CONNECTION_ATTEMPTS = 10;
  Modules::instaloader(
      "-V -C -G --no-pictures --no-profile-pic --no-captions --no-compress-json "
      "--max-connection-attempts=" + to_string(MAX_CONNECTION_ATTEMPTS) +
      " --filename-pattern={shortcode} --dirname-pattern=" +
      string(get_profiles_path()) + "/{target} " + t_profile, fout, ferr);

  if (err_started) {
    cout << Term::clear_line() +
        Term::process_colors(" #{red_out}--- --- ---#{reset}") << endl;
    exit(EXIT_FAILURE);
  } else {
    cout << Term::clear_line() + "All posts updated." << endl;
  }

  remove_unused_files(t_profile);
  Instanalyzer::msg(Instanalyzer::MSG_INFO, "Updating finished.");
}

void Profiles::remove_unused_files(const string& t_profile) {
  using namespace filesystem;
  cout << "\rRemoving unused files..." << flush;

  const path& profile_path = get_profiles_path() / t_profile;
  const set<string> unused_postfixes = {"_comments.json", "_location.txt"};

  for (const auto& f : directory_iterator(profile_path)) {
    const string& path(f.path());
    for (const auto& p : unused_postfixes) {
      if (path.substr(path.size() - p.size()) == p) {
        try {
          remove(f);
        } catch (const exception& e) {
          cout << Term::clear_line() << flush;
          Instanalyzer::msg(Instanalyzer::MSG_ERR, e.what());
          exit(EXIT_FAILURE);
        }
      }
    }
  }

  ifstream ifs(profile_path / "id");
  if (ifs.fail()) {
    cout << Term::clear_line() << flush;
    Instanalyzer::msg(Instanalyzer::MSG_ERR,
        "File with ID of profile didn't open!");
    exit(EXIT_FAILURE);
  }

  string id; ifs >> id;
  ifs.close();

  try {
    rename(string(profile_path / t_profile) + '_' + id + ".json",
        profile_path / "profile.json");
  } catch (const exception& e) {
    cout << Term::clear_line() << flush;
    Instanalyzer::msg(Instanalyzer::MSG_ERR, e.what());
    exit(EXIT_FAILURE);
  }
  remove(profile_path / "id");

  cout << Term::clear_line() + "Unused files removed." << endl;
}
