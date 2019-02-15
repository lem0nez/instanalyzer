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

#include "location.hpp"

#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>

#include <unac.h>

#include "curlpp/Easy.hpp"
#include "curlpp/Infos.hpp"
#include "curlpp/Options.hpp"

#include "instanalyzer.hpp"
#include "term.hpp"
#include "utils.hpp"

using namespace std;
using namespace nlohmann;

const map<Location::Geocoder, Location::GeocoderInfo> Location::m_geocoders = {
  {GEOCODER_HERE, {"#{blue_out}HERE", [] {
    return !string(HERE_APPID).empty() && !string(HERE_APPCODE).empty();
  }, getter_here}},
  {GEOCODER_YANDEX, {"#{red_out}Yandex", [] {
    return !string(YANDEX_API_KEY).empty();
  }, getter_yandex}}
};

Location::Geocoder Location::m_geocoder;

void Location::init() {
  if (Instanalyzer::get_pref("geocoder").empty()) {
    set_geocoder(request_geocoder());
    Instanalyzer::set_pref("geocoder", to_string(m_geocoder));
  } else {
    set_geocoder(static_cast<Geocoder>(stoi(Instanalyzer::get_pref("geocoder"))));
  }
}

Location::Geocoder Location::request_geocoder() {
  vector<Geocoder> available_geocoders;

  for (const auto& g : m_geocoders) {
    if (g.second.checker()) {
      available_geocoders.push_back(g.first);
    }
  }

  if (available_geocoders.empty()) {
    Instanalyzer::msg(Instanalyzer::MSG_WARN,
        "Geocoders doesn't available! You can't use location functions.");
    return GEOCODER_NONE;
  } else if (available_geocoders.size() == 1) {
    Instanalyzer::msg(Instanalyzer::MSG_INFO, Term::process_colors(
        "Selected #{bold}" + m_geocoders.at(*available_geocoders.cbegin()).name +
        "#{reset} geocoder."));
    return *available_geocoders.cbegin();
  }

  cout << Term::process_colors("Choose geocoder:") << endl;
  int n = 0;
  for (const auto& g : available_geocoders) {
    ++n;
    cout <<  "  " + Term::get_color(Term::COL_GRAY) + to_string(n) + ". " +
        Term::process_colors(m_geocoders.at(g).name + "#{reset}") << endl;
  }
  cout << Term::process_colors("item> #{bold}") << flush;

  int item = GEOCODER_NONE;
  while (!(cin >> item) || item < 1 || item > n) {
    cin.clear();
    cin.ignore(1000, '\n');
    cout << Term::process_colors(
        "#{reset}Invalid item! Try again.\nitem> #{bold}") << flush;
  }

  cout << Term::reset() << flush;
  return available_geocoders.at(item - 1);
}

set<Location::Place> Location::get_common_places(
    const set<Location::Coord>& t_coords) {
  if (t_coords.empty() || get_geocoder() == GEOCODER_NONE) {
    return {};
  }
  return m_geocoders.at(get_geocoder()).getter(t_coords);
}

set<Location::Place> Location::getter_here(
    const set<Location::Coord>& t_coords) {
  using namespace curlpp;
  cout << "\rReverse geocoding..." << flush;

  Easy request;
  stringstream ss_json;
  request.setOpt(options::WriteStream(&ss_json));
  request.setOpt(options::HttpHeader({"Content-Type: *"}));

  const string& API_VERSION = "6.2";
  const unsigned short API_GEN = 9;
  request.setOpt(options::Url("https://reverse.geocoder.api.here.com/" +
      API_VERSION + "/multi-reversegeocode.json?"
      "mode=retrieveAddresses"
      "&responseattributes=none"
      "&locationattributes=ar,ad,-mr,-mv,-dt,-sd,-ai,-li,-in,-tz,-nb,-rn"
      "&addressattributes=ctr,sta,cty,cit,str,hnr,add,-dis,-sdi,-pst,-aln"
      "&sortby=distance"
      "&jsonattributes=1"
      "&strictlanguagemode=false"
      "&language=en"
      "&gen=" + to_string(API_GEN) +
      "&app_id=" + string(HERE_APPID) +
      "&app_code=" + string(HERE_APPCODE)));

  string data;
  unsigned int id = 1;

  for (const auto& c : t_coords) {
    data += "id=" + to_string(id) + "&prox=" + to_string(c.lat) + ',' +
        to_string(c.lon) + ',' + to_string(c.radius) + '\n';
    ++id;
  }
  request.setOpt(options::PostFields(data));

  try {
    request.perform();
  } catch (const exception& e) {
    cout << Term::clear_line() << flush;
    Instanalyzer::msg(Instanalyzer::MSG_ERR, Term::process_colors(
        "Request error: \"#{gray_out}" + string(e.what()) + "#{reset}\"."));
    exit(EXIT_FAILURE);
  }

  const int response_code = infos::ResponseCode::get(request);
  if (response_code != 200) {
    cout << Term::clear_line() << flush;

    json json_err;
    try {
      json_err = json::parse(ss_json.str());
    } catch (const exception&) {
      Instanalyzer::msg(Instanalyzer::MSG_ERR, Term::process_colors(
          "Reverse geocoding failed. Response code: #{red_out}" +
          to_string(response_code) + "#{reset}."));
      exit(EXIT_FAILURE);
    }

    const string& msg = "Reverse geocoding using " +
        m_geocoders.at(GEOCODER_HERE).name + "#{reset} failed:";

    if (json_err.find("details") != json_err.end()) {
      Instanalyzer::msg(Instanalyzer::MSG_ERR, Term::process_colors(msg +
          " \"#{gray_out}" + string(json_err["details"]) + "#{reset}\"."));
    } else {
      Instanalyzer::msg(Instanalyzer::MSG_ERR, Term::process_colors(msg +
          "\n #{red_out}--- --- --- ---\n#{gray_out}" + ss_json.str() +
          "\n #{red_out}--- --- --- ---#{reset}"));
    }
    exit(EXIT_FAILURE);
  }

  json response_json;
  try {
    response_json = json::parse(ss_json.str());
  } catch (const exception& e) {
    cout << Term::clear_line() << flush;
    Instanalyzer::msg(Instanalyzer::MSG_ERR, Term::process_colors(
        "Can't parse JSON: \"#{gray_out}" + string(e.what()) + "#{reset}\"."));
    exit(EXIT_FAILURE);
  }

  cout << Term::clear_line() << flush;
  return getter_here_process(response_json);
}

set<Location::Place> Location::getter_here_process(const json& t_json) {
  cout << "\rProcessing response..." << flush;
  const string& NO_PLACES_MSG = "No places found!";

  if (!Utils::has_json_node(t_json, {"response", "item"})) {
    cout << Term::clear_line() << flush;
    Instanalyzer::msg(Instanalyzer::MSG_WARN, NO_PLACES_MSG);
    return {};
  }

  set<Place> places;
  Place place;

  const vector<PlaceJsonStrVal> json_place_add_adresses = {
    {ACCUR_COUNTRY, "CountryName", place.country},
    {ACCUR_STATE, "StateName", place.state},
    {ACCUR_COUNTY, "CountyName", place.county}
  };

  const vector<PlaceJsonStrVal> json_place_adresses = {
    {ACCUR_COUNTRY, "country", place.country},
    {ACCUR_STATE, "state", place.state},
    {ACCUR_COUNTY, "county", place.county},
    {ACCUR_CITY, "city", place.city},
    {ACCUR_STREET, "street", place.street},
    {ACCUR_HOUSE, "houseNumber", place.house}
  };

  for (const auto& item : t_json["response"]["item"]) {
    if (item.find("result") == item.end()) {
      continue;
    }

    for (const auto& p : item["result"]) {
      if (p.find("location") == p.end()) {
        continue;
      }
      const auto& location = p["location"];

      if (location.find("locationId") != location.end()) {
        place.id = location["locationId"];
      } else {
        continue;
      }

      if (location.find("address") != location.end()) {
        const auto& address = location["address"];

        if (address.find("additionalData") != address.end()) {
          for (const auto& a : address["additionalData"]) {
            if (a.find("key") != a.end() && a.find("value") != a.end()) {
              for (const auto& v : json_place_add_adresses) {
                if (a["key"] == v.key) {
                  v.val = a["value"];
                  place.accur = v.accur;
                }
              }
            }
          }
        }
        for (const auto& v : json_place_adresses) {
          if (v.val.empty() && address.find(v.key) != address.end()) {
            v.val = address[v.key];
            place.accur = v.accur;
          }
        }
      }
      places.insert(place);
      place = Place();
    }
  }

  cout << Term::clear_line() << flush;
  if (places.empty()) {
    Instanalyzer::msg(Instanalyzer::MSG_WARN, NO_PLACES_MSG);
  } else {
    cout << "Reverse geocoding finished" << endl;
  }
  return places;
}

set<Location::Place> Location::getter_yandex(
    const set<Location::Coord>& t_coords) {
  using namespace filesystem;

  cout << Term::process_colors("For retrieving places info used geocoder by "
      "#{red_out}\u00a9 YANDEX, LLC#{reset}.") << endl;

  const path& cache_path = Instanalyzer::get_cache_path() / "geocoder-yandex";
  if (!directory_entry(cache_path).exists()) {
    try {
      create_directory(cache_path);
    } catch (const exception& e) {
      Instanalyzer::msg(Instanalyzer::MSG_ERR, Term::process_colors(
          "Can't create parent directory for cache: \"#{gray_out}" +
          string(e.what()) + "#{reset}\"."));
      exit(EXIT_FAILURE);
    }
  }

  set<Place> places;
  Place place;

  const vector<string> places_node = {
    "response", "GeoObjectCollection", "featureMember"
  }, pos_node = {
    "GeoObject", "Point", "pos"
  }, address_node = {
    "GeoObject", "metaDataProperty", "GeocoderMetaData", "Address", "Components"
  };

  const vector<PlaceJsonStrVal> json_place_adresses = {
    {ACCUR_COUNTRY, "country", place.country},
    {ACCUR_STATE, "province", place.state},
    {ACCUR_CITY, "locality", place.city},
    {ACCUR_STREET, "street", place.street},
    {ACCUR_HOUSE, "house", place.house}
  };

  unsigned int item = 1;
  for (auto c = t_coords.cbegin(); c != t_coords.cend(); ++c) {
    const string& text_progress = Term::process_colors(
        "Reverse geocoding #{orange_out}" +
        to_string(item) + "#{reset} of #{orange_out}" +
        to_string(t_coords.size()) + "#{reset} places");
    cout << Term::clear_line() + text_progress + "..." << flush;
    ++item;

    const path& json_path = cache_path / (to_string(hash<string>{}(
        to_string(c->lat) + '-' + to_string(c->lon))) + ".json");
    json json_data;

    if (!directory_entry(json_path).exists()) {
      json_data = getter_yandex_download(c->lat, c->lon);

      ofstream ofs(json_path);
      ofs << json_data;
    } else {
      ifstream ifs(json_path);
      stringstream ss;
      ss << ifs.rdbuf();
      ifs.close();

      try {
        json_data = json::parse(ss.str());
      } catch (const exception&) {
        cout << Term::clear_line() + text_progress +
            " (can't parse item from cache, re-downloading)..." << flush;

        json_data = getter_yandex_download(c->lat, c->lon);
        ofstream ofs(json_path);
        ofs << json_data;
      }
    }

    if (!Utils::has_json_node(json_data, places_node)) {
      continue;
    }
    for (const auto& p :
        json_data["response"]["GeoObjectCollection"]["featureMember"]) {
      if (!Utils::has_json_node(p, pos_node)) {
        continue;
      }

      string pos = p["GeoObject"]["Point"]["pos"];
      place.id = to_string(hash<string>{}(pos.replace(pos.find(' '), 1, "-")));

      if (Utils::has_json_node(p, address_node)) {
        for (const auto& a : p["GeoObject"]["metaDataProperty"]
            ["GeocoderMetaData"]["Address"]["Components"]) {
          if (a.find("kind") == a.end() || a.find("name") == a.end()) {
            continue;
          }

          for (const auto& v : json_place_adresses) {
            if (a["kind"] == v.key) {
              v.val = a["name"];
              place.accur = v.accur;
            }
          }
        }
      }

      places.insert(place);
      place = Place();
    }
  }

  cout << Term::clear_line() << flush;
  if (places.empty()) {
    Instanalyzer::msg(Instanalyzer::MSG_WARN, "No places found!");
  } else {
    cout << "Reverse geocoding finished." << endl;
  }
  return places;
}

json Location::getter_yandex_download(const double& t_lat, const double& t_lon) {
  using namespace curlpp;

  Easy request;
  stringstream ss_json;
  request.setOpt(options::WriteStream(&ss_json));
  request.setOpt(options::FollowLocation(true));

  const string& API_VERSION = "1.x";
  const unsigned int MAX_RESULTS = 100;

  request.setOpt(options::Url("https://geocode-maps.yandex.ru/" + API_VERSION +
      "?apikey=" + string(YANDEX_API_KEY) +
      "&geocode=" + to_string(t_lat) + ',' + to_string(t_lon) +
      "&sco=latlong"
      "&kind=house"
      "&results=" + to_string(MAX_RESULTS) +
      "&lang=en_RU&format=json"));

  try {
    request.perform();
  } catch (const exception& e) {
    cout << Term::clear_line() << flush;
    Instanalyzer::msg(Instanalyzer::MSG_ERR, Term::process_colors(
        "Request error: \"#{gray_out}" + string(e.what()) + "#{reset}\"."));
    exit(EXIT_FAILURE);
  }

  const int response_code = Infos::ResponseCode::get(request);
  if (response_code != 200) {
    string err_msg;

    try {
      json json_err = json::parse(ss_json.str());
      if (json_err["error"].find("message") != json_err["error"].end()) {
        err_msg = "Places data didn't get: \"#{gray_out}" +
            string(json_err["error"]["message"]) + "#{reset}\".";
      }
    } catch (const exception&) {}

    if (err_msg.empty()) {
      err_msg = "Places data didn't get! Response code: #{red_out}" +
          to_string(response_code) + "#{reset}.";
    }

    cout << Term::clear_line() << flush;
    Instanalyzer::msg(Instanalyzer::MSG_ERR, Term::process_colors(err_msg));
    exit(EXIT_FAILURE);
  }

  char* json_processed = nullptr;
  size_t processed_len = 0;
  // Remove accented characters which may contains in addresses.
  int err_code = unac_string("UTF-8", ss_json.str().c_str(),
      ss_json.str().length(), &json_processed, &processed_len);

  if (err_code) {
    cout << Term::clear_line() << flush;
    Instanalyzer::msg(Instanalyzer::MSG_ERR,
        "Can't remove accented characters from JSON (error code: #{red_out}" +
        to_string(err_code) + "#{reset}).");
    exit(EXIT_FAILURE);
  }

  json json_data;
  try {
    json_data = json::parse(json_processed);
  } catch (const exception& e) {
    cout << Term::clear_line() << flush;
    Instanalyzer::msg(Instanalyzer::MSG_ERR, Term::process_colors(
        "Can't parse JSON: \"#{gray_out}" + string(e.what()) + "#{reset}\"."));
    exit(EXIT_FAILURE);
  }
  return json_data;
}
