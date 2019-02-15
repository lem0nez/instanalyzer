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
#include <iomanip>
#include <iostream>
#include <vector>

#include "graph.hpp"
#include "instanalyzer.hpp"
#include "post.hpp"
#include "utils.hpp"

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

void Data::show_profile_info(const Profile& t_profile) {
  t_profile.check();

  ifstream ifs(
      Profile::get_profiles_path() / t_profile.get_name() / "profile.json");
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
    const Profile& t_profile, const unsigned int& t_radius) {
  t_profile.check();

  const set<Location::Place> places = Location::get_common_places(
      get_coords(t_profile, t_radius));
  if (places.empty()) {
    return;
  }

  bool at_least_one_group_printed = false;
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
    }

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

    vector<Graph> graphs;
    for (const auto& p : places_sorted) {
      Graph graph;
      graph.set_label(p.first);
      graph.set_percents((static_cast<double>(p.second) /
          static_cast<double>(places_count)) * 100.0);
      graph.set_colors(colors);
      graph.set_bold_text(false);

      graphs.push_back(graph);
    }

    Graph::draw_graphs(cout, graphs);
    at_least_one_group_printed = true;
  }

  if (at_least_one_group_printed) {
    cout << endl;
    Instanalyzer::msg(Instanalyzer::MSG_INFO, "All location groups printed.");
  }
}

set<Location::Coord> Data::get_coords(
    const Profile& t_profile, const unsigned int& t_radius) {
  using namespace filesystem;

  cout << "\rProcessing posts..." << flush;

  const set<string> picture_types = {
    "GraphSidecar", "GraphImage"
  };

  const set<json> posts = t_profile.get_posts();
  set<Location::Coord> coords;
  unsigned int pictures = 0, geotags = 0;

  for (const auto& p : posts) {
    if (!Utils::has_json_node(p, {"node", "location"})) {
      continue;
    }

    if (p["node"].find("__typename") != p["node"].end() &&
        picture_types.count(p["node"]["__typename"]) != 0) {
      ++pictures;

      const auto& location = p["node"]["location"];
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
    cout << Term::process_colors("Processed #{yellow_out}" +
        to_string(posts.size()) + "#{reset} posts and #{yellow_out}" +
        to_string(geotags) + "#{reset} of #{yellow_out}" + to_string(pictures) +
        "#{reset} pictures have location info.") << endl;

    cout << Term::process_colors("Common places count: #{yellow_out}" +
        to_string(coords.size()) + "#{reset}.") << endl;
  }
  return coords;
}

void Data::show_posts_top(const Profile& t_profile, const int t_count) {
  t_profile.check();

  cout << "\rProcessing posts..." << flush;
  const set<json>& json_posts = t_profile.get_posts();
  vector<Post> posts;

  for (const auto& p : json_posts) {
    if (!Utils::has_json_node(p, {"node", "shortcode"}) ||
        !Utils::has_json_node(p, {"node", "edge_media_preview_like", "count"})) {
      continue;
    }

    const string& shortcode = p["node"]["shortcode"];
    if (!shortcode.empty()) {
      Post post;
      post.set_shortcode(shortcode);
      post.set_likes(p["node"]["edge_media_preview_like"]["count"]);

      if (Utils::has_json_node(p, {"node", "taken_at_timestamp"})) {
        post.set_creation_time(p["node"]["taken_at_timestamp"]);
      } else {
        post.set_creation_time(0);
      }
      posts.push_back(post);
    }
  }

  const auto& cmp = [&t_count] (const Post& lhs, const Post& rhs) {
    return t_count < 0 ? (lhs.get_likes() < rhs.get_likes()) :
        (lhs.get_likes() > rhs.get_likes());
  };
  sort(posts.begin(), posts.end(), cmp);

  cout << Term::clear_line() << endl;
  if (posts.empty()) {
    Instanalyzer::msg(Instanalyzer::MSG_WARN, "No posts!");
    return;
  }

  vector<Post>::const_iterator end_it;
  if (t_count == 0) {
    end_it = posts.cend();
  } else {
    if (static_cast<size_t>(abs(t_count)) > posts.size()) {
      end_it = posts.cend();
    } else {
      end_it = posts.cbegin() + abs(t_count);
    }
  }

  const time_t& time_zero = 0;
  ostringstream time_zone;
  time_zone << put_time(localtime(&time_zero), "%Z");

  cout << Term::process_colors("#{bold}Top posts (#{gray_out}" +
      time_zone.str() + "#{reset} time zone" + string(t_count < 0 ?
      ", #{red_out}reverse#{reset}#{bold}" : "") + "):#{reset}") << endl;

  for (auto p = posts.cbegin(); p != end_it; ++p) {
    const string& item = "#{bold}" +
        to_string((p - posts.cbegin()) + 1) + ".#{reset}";
    const string& likes = "#{yellow_out}" + to_string(p->get_likes()) +
        "#{reset} like" + (p->get_likes() == 1 ? "" : "s");

    const time_t& creation_time = p->get_creation_time();
    ostringstream date;
    date << put_time(localtime(&creation_time), "%a %b %d %H:%M %Y");

    cout << Term::process_colors("  " + item + "#{cream_out} instagram.com/p/" +
        p->get_shortcode() + " #{reset}(" + likes + (creation_time == 0 ?
        "" : ", " + date.str()) + ")#{reset}") << endl;
  }
}

set<Profile::TaggedProfile> Data::get_tagged_profiles(const Profile& t_owner) {
  const set<json>& posts = t_owner.get_posts();
  set<Profile::TaggedProfile> tagged_profiles;

  typedef void (*profile_setter) (Profile&, const json::value_type&);
  const map<string, profile_setter>& setters = {
    {"id", [] (Profile& prof, const json::value_type& val) {
      prof.set_id(val);
    }},
    {"username", [] (Profile& prof, const json::value_type& val) {
      prof.set_name(val);
    }},
    {"full_name", [] (Profile& prof, const json::value_type& val) {
      prof.set_full_name(val);
    }},
    {"is_verified", [] (Profile& prof, const json::value_type& val) {
      prof.set_verified(val);
    }}
  };

  for (const auto& p : posts) {
    if (!Utils::has_json_node(p, {"node", "shortcode"}) ||
        !Utils::has_json_node(p, {"node", "edge_media_to_tagged_user", "edges"})) {
      continue;
    }

    for (const auto& u : p["node"]["edge_media_to_tagged_user"]["edges"]) {
      if (!Utils::has_json_node(u, {"node", "user"})) {
        continue;
      }

      const auto& node = u["node"];
      Profile profile;

      for (const auto& s : setters) {
        if (node["user"].find(s.first) != node["user"].end()) {
          s.second(profile, node["user"][s.first]);
        }
      }

      Profile::TaggedProfile tagged_profile(profile);
      tagged_profile.post_shortcode = p["node"]["shortcode"];

      if (node.find("x") != node.end() && node.find("y") != node.end()) {
        tagged_profile.x = node["x"];
        tagged_profile.y = node["y"];
      }
      tagged_profiles.insert(tagged_profile);
    }
  }
  return tagged_profiles;
}

void Data::show_tagged_profiles(const Profile& t_owner) {
  t_owner.check();

  cout << "\n\rProcessing posts..." << flush;
  const auto& tagged_profiles = get_tagged_profiles(t_owner);
  map<Profile, unsigned int> tags_count;

  for (const auto& p : tagged_profiles) {
    if (p.profile->get_name().empty()) {
      continue;
    }
    ++tags_count[*p.profile];
  }

  vector<pair<Profile, unsigned int>>
      tags_count_sorted(tags_count.cbegin(), tags_count.cend());
  const auto& cmp = [] (const pair<Profile, unsigned int>& lhs,
      const pair<Profile, unsigned int>& rhs) {
    return lhs.second > rhs.second;
  };
  sort(tags_count_sorted.begin(), tags_count_sorted.end(), cmp);

  cout << Term::clear_line() << flush;
  if (tags_count_sorted.empty()) {
    Instanalyzer::msg(Instanalyzer::MSG_WARN, "No tagged profiles!");
    return;
  }

  cout << Term::process_colors("#{bold}> Often tagged profiles:#{reset}") << endl;
  const auto& graph_style = Graph::get_random_style();

  for (const auto& p : tags_count_sorted) {
    Graph graph;

    graph.set_label('@' + p.first.get_name() + " (" + to_string(p.second) +
        " time" + (p.second == 1 ? "" : "s") + ')');
    graph.set_bold_text(p.first.get_name() == t_owner.get_name());
    graph.set_colors(graph_style);
    graph.set_percents((static_cast<double>(p.second) /
        tagged_profiles.size()) * 100.0);

    graph.draw(cout);
  }
}
