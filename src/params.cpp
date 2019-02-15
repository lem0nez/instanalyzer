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

#include "params.hpp"

#include <algorithm>
#include <deque>
#include <iostream>

#include "comment.hpp"
#include "data.hpp"
#include "instanalyzer.hpp"
#include "location.hpp"
#include "modules.hpp"
#include "profile.hpp"
#include "term.hpp"

using namespace std;

const map<Params::Parameters, Params::ParamInfo> Params::m_params = {
  {PARAM_PROFILE_LOCATION, {{"-l", "--location"},
      "Show info of most visited places.", true}},
  {PARAM_PROFILE_COMMENTATORS, {{"-c", "--commentators"},
      "Show most active commentators.", true}},
  {PARAM_COMMENTATOR_INFO, {{"-o", "--commentator"},
      "Show commentator info.", true, true, "name"}},
  {PARAM_TOP_POSTS, {{"-p", "--top-posts"},
      "Show top of most (or less with prefix \"r\") liked posts.",
      true, false, "count"}},
  {PARAM_TAGGED_PROFILES, {{"-t", "--tagged"},
      "Show often tagged profiles.", true}},
  {PARAM_PROFILE_INFO, {{"-i", "--info"}, "Show profile info.", true}},
  {PARAM_UPDATE_PROFILE,
      {{"-u", "--update"}, "Force update local copy of profile.", true}},
  {PARAM_GEOCODER, {{"--geocoder", "-g"},
      "Change geocoder (if available).", false}},
  {PARAM_THEME, {{"--theme"}, "Change theme.", false}},
  {PARAM_UPDATE, {{"--update-modules"}, "Force update modules.", false}},
  {PARAM_VERSION, {{"--version"}, "Show version of tools and exit.", false}},
  {PARAM_HELP, {{"--help", "-h"}, "Show help and exit.", false}}
};

void Params::process_params(const vector<string>& t_params) {
  if (t_params.empty()) {
    show_help();
    exit(EXIT_SUCCESS);
  }

  // Get value which associated with parameter.
  // Return empty string if value doesn't exist
  const auto& get_val = [&t_params]
      (const vector<string>::const_iterator& t_param_it) -> string {
    const auto& val = t_param_it + 1;

    if (val != t_params.cend() && val->substr(0, 1) != "-") {
      return *val;
    } else {
      return "";
    }
  };

  bool request_profile = false;
  string profile;
  set<Parameters> used_params;
  deque<function<void()>> funcs;

  for (auto p = t_params.cbegin(); p != t_params.cend(); ++p) {
    bool is_found = false;

    for (const auto& i : m_params) {
      if (find(i.second.names.cbegin(), i.second.names.cend(), *p) ==
          i.second.names.cend()) {
        continue;
      } else {
        is_found = true;
      }

      if (!i.second.allow_multiple) {
        if (used_params.count(i.first) != 0) {
          continue;
        } else {
          used_params.insert(i.first);
        }
      }

      switch (i.first) {
        case PARAM_PROFILE_INFO:
          request_profile = true;
          funcs.push_back([&profile] {
            Data::show_profile_info(Profile(profile));
          });
          continue;
        case PARAM_PROFILE_LOCATION:
          request_profile = true;
          funcs.push_back([&profile] {
            Data::show_location_info(Profile(profile));
          });
          continue;
        case PARAM_PROFILE_COMMENTATORS:
          request_profile = true;
          funcs.push_back([&profile] { Comment::show_commentators(profile); });
          continue;
        case PARAM_COMMENTATOR_INFO: {
          request_profile = true;
          const string& val = get_val(p);

          if (val.empty()) {
            Instanalyzer::msg(Instanalyzer::MSG_ERR, Term::process_colors(
                "Need specify commentator name with parameter \"#{red_out}" +
                *p + "#{reset}\"!"));
            exit(EXIT_FAILURE);
          }

          funcs.push_back([&profile, val] {
            Comment::show_commentator_info(profile, val);
          });
          ++p;
          continue;
        }
        case PARAM_TOP_POSTS: {
          request_profile = true;
          const string& val = get_val(p);
          int count = Data::get_default_posts_count();

          if (!val.empty() && (p + 2) != t_params.cend()) {
            try {
              if (val.substr(0, 1) == "r") {
                count = -stoi(val.substr(1));
              } else {
                count = stoi(val);
              }
            } catch (const exception&) {
              Instanalyzer::msg(Instanalyzer::MSG_ERR, Term::process_colors(
                  "Parameter \"" + *p + "\" receive the integer value "
                  "(including prefix \"r\")!"));
              exit(EXIT_FAILURE);
            }
            ++p;
          }

          funcs.push_back([&profile, count] {
            Data::show_posts_top(profile, count);
          });
          continue;
        }
        case PARAM_TAGGED_PROFILES:
          request_profile = true;
          funcs.push_back([&profile] { Data::show_tagged_profiles(profile); });
          continue;
        case PARAM_UPDATE_PROFILE:
          request_profile = true;
          funcs.push_front([&profile] { Profile(profile).update(); });
          continue;
        case PARAM_GEOCODER:
          Location::set_geocoder(Location::request_geocoder());
          Instanalyzer::set_pref("geocoder", to_string(Location::get_geocoder()));
          continue;
        case PARAM_THEME:
          Term::set_dark_theme(Instanalyzer::request_theme(true));
          continue;
        case PARAM_UPDATE:
          Modules::update_modules();
          continue;
        case PARAM_VERSION:
          show_version();
          exit(EXIT_SUCCESS);
        case PARAM_HELP:
          show_help();
          exit(EXIT_SUCCESS);
      }
    }

    if (!is_found) {
      if (p->substr(0, 1) == "-") {
        Instanalyzer::msg(Instanalyzer::MSG_ERR, Term::process_colors(
            "Invalid parameter: #{bold}" + *p + "#{reset}!"));
        exit(EXIT_FAILURE);
      } else {
        profile = *p;
      }
    }
  }

  if (request_profile && profile.empty()) {
    Instanalyzer::msg(Instanalyzer::MSG_ERR, "Need to specify profile!");
    exit(EXIT_FAILURE);
  } else if (!profile.empty() && funcs.empty()) {
    show_help();
    exit(EXIT_SUCCESS);
  }

  for (const auto& f : funcs) {
    f();
  }
}

void Params::show_help() {
  string main_params = "Main parameters:\n", other_params = "Other parameters:\n";

  for (const auto& p : m_params) {
    auto& params = p.second.is_main_param ? main_params : other_params;
    params += "  " + Term::bold();

    for (auto n = p.second.names.cbegin(); n != p.second.names.cend(); ++n) {
      params += (n == p.second.names.cbegin()) ? *n : (", " + *n);
    }
    if (!p.second.val.empty()) {
      params += " <" + p.second.val + '>';
    }

    params += Term::reset() + "    " + p.second.info + '\n';
  }

  cout << Term::process_colors(
      "#{gray_out}The #{bold}Instanalyzer#{reset}#{gray_out} tool, based on the "
      "#{bold}Instaloader#{reset}#{gray_out} script, for collecting\n"
      "and structuring information about posts of Instagram profile.#{reset}\n"
      "Usage: #{bold}instanalyzer#{reset} [parameters] {profile}.\n\n") +
      main_params + '\n' + other_params << flush;
}

void Params::show_version() {
  cout << Term::process_colors(
      "Instanalyzer: #{cream_out}" + string(VERSION) + "#{reset}") << endl;

  const auto& python_fver = [] (const string& str) {
    cout << Term::process_colors("Python: #{cream_out}" +
        str.substr(str.find(' ') + 1, str.size()) + "#{reset}") << endl;
  };
  const auto& python_ferr = [] ([[maybe_unused]] const string& str) {
    Instanalyzer::msg(Instanalyzer::MSG_ERR, Term::process_colors(
        "Version of #{bold}Python#{reset} didn't get! "
        "Maybe, it didn't install?"));
    exit(EXIT_FAILURE);
  };
  Modules::interpreter("--version", python_fver, python_ferr);

  const auto& instaloader_fver = [] (const string& str) {
    cout << Term::clear_line() + Term::process_colors(
        "Instaloader: #{cream_out}" + str + "#{reset}") << endl;
  };
  const auto& instaloader_ferr = [] ([[maybe_unused]] const string& str) {
    cout << Term::clear_line() << flush;
    Instanalyzer::msg(Instanalyzer::MSG_ERR, Term::process_colors(
        "Version of #{bold}Instaloader#{reset} didn't get! "
        "Try to update modules."));
    exit(EXIT_FAILURE);
  };

  cout << Term::process_colors("\rInstaloader: #{gray_out}...#{reset}") << flush;
  Modules::instaloader("--version", instaloader_fver, instaloader_ferr);
}
