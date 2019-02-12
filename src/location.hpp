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

#include "nlohmann/json.hpp"

class Location {
public:
  enum Geocoder {
    GEOCODER_NONE = -1,
    GEOCODER_HERE,
    GEOCODER_YANDEX
  };

  enum AccuracyLevel {
    ACCUR_COUNTRY,
    ACCUR_STATE,
    ACCUR_COUNTY,
    ACCUR_CITY,
    ACCUR_STREET,
    ACCUR_HOUSE
  };

  struct Coord {
    inline bool operator<(const Coord& rhs) const {
      return lat < rhs.lat || lon < rhs.lon || radius < rhs.radius;
    }

    long double lat, lon;
    unsigned int radius;
  };

  struct Place {
    inline bool operator<(const Place& rhs) const { return id != rhs.id; }

    std::string id;
    AccuracyLevel accur;
    std::string country, state, county, city, street, house;
  };

  inline static Geocoder get_geocoder() { return m_geocoder; }
  inline static void set_geocoder(const Geocoder& t_geocoder) {
    m_geocoder = t_geocoder;
  };

  static void init() noexcept(false);
  static Geocoder request_geocoder();
  static std::set<Place> get_common_places(const std::set<Coord>&);
  inline static unsigned int get_default_radius() { return 250; }

private:
  typedef bool (*geocoder_checker)();
  typedef std::set<Place> (*common_places_getter)(const std::set<Coord>&);

  struct GeocoderInfo {
    std::string name;
    geocoder_checker checker;
    common_places_getter getter;
  };

  struct PlaceJsonStrVal {
    AccuracyLevel accur;
    std::string key;
    std::string& val;
  };

  static std::set<Place> getter_here(const std::set<Coord>&);
  static std::set<Place> getter_here_process(const nlohmann::json&);

  static std::set<Place> getter_yandex(const std::set<Coord>&);
  static nlohmann::json getter_yandex_download(
      const double& lat, const double& lon);

  static const std::map<Geocoder, GeocoderInfo> m_geocoders;
  static Geocoder m_geocoder;
};
