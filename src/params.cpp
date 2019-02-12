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

#include "data.hpp"
#include "instanalyzer.hpp"
#include "location.hpp"
#include "modules.hpp"
#include "profile.hpp"
#include "term.hpp"

using namespace std;

const map<Params::Parameters, Params::ParamInfo> Params::m_params = {
  {PARAM_PROFILE_LOCATION, {{"-l", "--location"}, "",
      "Show info of most visited places.", true}},
  {PARAM_PROFILE_INFO, {{"-i", "--info"}, "", "Show profile info.", true}},
  {PARAM_UPDATE_PROFILE,
      {{"-u", "--update"}, "", "Force update local copy of profile.", true}},
  {PARAM_GEOCODER, {{"-g", "--geocoder"}, "", "Change geocoder.", false}},
  {PARAM_THEME, {{"-t", "--theme"}, "", "Change theme.", false}},
  {PARAM_UPDATE, {{"-m", "--update-modules"}, "", "Force update modules.", false}},
  {PARAM_VERSION, {{"-v", "--version"}, "",
      "Show version of tools and exit.", false}},
  {PARAM_HELP, {{"-h", "--help"}, "", "Show help and exit.", false}}
};

void Params::process_params(const vector<string>& t_params) {
  if (t_params.empty()) {
    show_help();
    exit(EXIT_SUCCESS);
  }

  bool request_profile = false;
  string profile;
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

      switch (i.first) {
        case PARAM_PROFILE_INFO:
          request_profile = true;
          funcs.push_back([&profile] { Data::show_profile_info(profile); });
          continue;
        case PARAM_PROFILE_LOCATION:
          request_profile = true;
          funcs.push_back([&profile] { Data::show_location_info(profile); });
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
