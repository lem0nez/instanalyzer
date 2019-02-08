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

#include "data.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "graph.hpp"
#include "instanalyzer.hpp"
#include "profiles.hpp"

using namespace std;
using namespace nlohmann;

const vector<pair<string, Data::val_parser>> Data::m_profile_data = {
  {"Username", [] (const json& j) -> string {
    try {
      const string& v = j.at("node").at("username");
      return v.empty() ? "" : "#{red_out}" + v + "#{reset}";
    } catch (const json::exception&) { return ""; }
  }},
  {"Full name", [] (const json& j) -> string {
    try {
      const string& v = j.at("node").at("full_name");
      return v.empty() ? "" : "#{red_out}" + v + "#{reset}";
    } catch (const json::exception&) { return ""; }
  }},
  {"Following", [] (const json& j) -> string {
    try {
      const int v = j.at("node").at("edge_follow").at("count");
      return "#{red_out}" + to_string(v) + "#{reset}";
    } catch (const json::exception&) { return ""; }
  }},
  {"Followers", [] (const json& j) -> string {
    try {
      const int v = j.at("node").at("edge_followed_by").at("count");
      return "#{red_out}" + to_string(v) + "#{reset}";
    } catch (const json::exception&) { return ""; }
  }},
  {"Business category", [] (const json& j) -> string {
    try {
      const string& v = j.at("node").at("business_category_name");
      return v.empty() ? "" : "#{blue_out}" + v + "#{reset}";
    } catch (const json::exception&) { return ""; }
  }},
  {"Business email", [] (const json& j) -> string {
    try {
      const string& v = j.at("node").at("business_email");
      return v.empty() ? "" : "#{blue_out}" + v + "#{reset}";
    } catch (const json::exception&) { return ""; }
  }},
  {"Business phone", [] (const json& j) -> string {
    try {
      const string& v = j.at("node").at("business_phone_number");
      return v.empty() ? "" : "#{blue_out}" + v + "#{reset}";
    } catch (const json::exception&) { return ""; }
  }},
  {"Facebook page", [] (const json& j) -> string {
    try {
      const string& v = j.at("node").at("connected_fb_page");
      return v.empty() ? "" : "#{yellow_out}" + v + "#{reset}";
    } catch (const json::exception&) { return ""; }
  }},
  {"External URL", [] (const json& j) -> string {
    try {
      const string& v = j.at("node").at("external_url");
      return v.empty() ? "" : "#{yellow_out}" + v + "#{reset}";
    } catch (const json::exception&) { return ""; }
  }},
  {"Verified", [] (const json& j) -> string {
    try {
      const bool v = j.at("node").at("is_verified");
      return "#{cream_out}" + string(v ? "yes" : "no") + "#{reset}";
    } catch (const json::exception&) { return ""; }
  }},
  {"Biography", [] (const json& j) -> string {
    try {
      const string& v = j.at("node").at("biography");
      return v.empty() ? "" : "#{cream_out}" + v + "#{reset}";
    } catch (const json::exception&) { return ""; }
  }}
};

void Data::show_profile_info(const string& t_profile) {
  Profiles::check(t_profile);

  ifstream ifs(Profiles::get_profiles_path() / t_profile / "profile.json");
  if (ifs.fail()) {
    Instanalyzer::msg(Instanalyzer::MSG_ERR,
        "File with profile info didn't open!");
    exit(EXIT_FAILURE);
  }

  json j = json::parse(ifs);
  ifs.close();
  bool have_some_info = false;

  for (const auto& i : m_profile_data) {
    have_some_info = true;
    const string& val = i.second(j);

    if (val.empty()) {
      continue;
    } else {
      cout << Term::process_colors("#{gray_out}" + i.first + ":#{reset} ") +
          Term::process_colors(val) << endl;
    }
  }

  if (!have_some_info) {
    Instanalyzer::msg(Instanalyzer::MSG_ERR, "No profile info!");
  }
}

void Data::show_location_info(
    const string& t_profile, const unsigned int t_radius) {
  Profiles::check(t_profile);

  const set<Location::Place> places =
      Location::get_common_places(get_coords(t_profile, t_radius));
  if (places.empty()) {
    return;
  }
  Location::Place place;

  const vector<LocationGroup> groups = {
    {"Country", {&place.country}},
    {"Region", {&place.state, &place.country}},
    {"District", {&place.county, &place.state, &place.country}},
    {"City", {&place.city, &place.county, &place.state, &place.country}}
  };

  for (const auto& g : groups) {
    // Map of labels and repeat count.
    map<string, unsigned int> group_places;
    size_t places_count = places.size();

    for (const auto& p : places) {
      place = p;
      string label;
      size_t i = 0, count = g.address_tree.size();

      for (const auto& a : g.address_tree) {
        ++i;
        if (a->empty()) {
          if (i == 1) {
            break;
          } else {
            continue;
          }
        }

        label += *a;
        if (i != count) {
          label += ", ";
        }
      }

      if (label.empty()) {
        --places_count;
        continue;
      } else {
        ++group_places[label];
      }
    }

    if (group_places.empty()) {
      continue;
    } else {
      cout << Term::process_colors("\n#{bold}> " + g.name) << flush;
      const size_t unknown = places.size() - places_count;

      if (unknown > 0) {
        cout << Term::process_colors(
            " #{gray_out}(" + to_string(unknown) + " unknown)") << flush;
      }
      cout << Term::reset() << endl;

      Graph::Colors colors = Graph::get_random_style();
      const auto& cmp = [] (const pair<string, unsigned int>& lhs,
          const pair<string, unsigned int>& rhs) {
        return lhs.second > rhs.second;
      };
      vector<pair<string, unsigned int>>
          places_sorted(group_places.cbegin(), group_places.cend());
      sort(places_sorted.begin(), places_sorted.end(), cmp);

      vector<Graph::GraphInfo> graphs;
      for (const auto& p : places_sorted) {
        graphs.push_back( {p.first, (static_cast<double>(p.second) /
            static_cast<double>(places_count)) * 100.0, false, colors});
      }
      Graph::draw_graphs(cout, graphs);
    }
  }
}

set<Location::Coord> Data::get_coords(
    const string& t_profile, const unsigned int t_radius) {
  using namespace filesystem;

  cout << "\rProcessing posts..." << flush;

  const set<string> picture_types = {
    "GraphSidecar", "GraphImage"
  };

  set<Location::Coord> coords;
  const path& profile_path = Profiles::get_profiles_path() / t_profile;
  unsigned int posts = 0, pictures = 0, geotags = 0;

  for (const auto& p : directory_iterator(profile_path)) {
    if (p.is_directory() || p.path() == profile_path / "profile.json") {
      continue;
    }

    ++posts;
    ifstream ifs(p.path());
    stringstream ss_json;
    ss_json << ifs.rdbuf();
    json json_data;

    try {
      json_data = json::parse(ss_json.str());
    } catch (const exception&) {
      continue;
    }

    if (json_data.find("node") == json_data.end() ||
        json_data["node"].find("location") == json_data["node"].end()) {
      continue;
    }

    if (json_data["node"].find("__typename") != json_data["node"].end() &&
        picture_types.count(json_data["node"]["__typename"]) != 0) {
      ++pictures;

      const auto& location = json_data["node"]["location"];
      if (location.find("lat") != location.end() &&
          location.find("lng") != location.end()) {
        coords.insert({location["lat"], location["lng"], t_radius});
        ++geotags;
      }
    }
  }

  cout << Term::clear_line() << flush;
  if (geotags == 0) {
    Instanalyzer::msg(Instanalyzer::MSG_WARN, "No pictures with location info!");
  } else {
    cout << Term::process_colors("Processed #{yellow_out}" + to_string(posts) +
        "#{reset} posts and #{yellow_out}" + to_string(geotags) +
        "#{reset} of #{yellow_out}" + to_string(pictures) +
        "#{reset} pictures have location info.") << endl;
  }
  return coords;
}
