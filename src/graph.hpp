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

#include <ostream>
#include <string>
#include <vector>

#include "term.hpp"

class Graph {
public:
  struct Colors {
    Term::Color graph;
    // Text colors in and out of graph.
    Term::Color text_in;
    Term::Color text_out;
  };

  Graph() = default;

  inline std::string get_label() const { return m_label; }
  inline double get_percents() const { return m_percents; }
  inline Colors get_colors() const { return m_col; }
  inline bool is_bold_text() const { return m_is_bold_text; }

  inline void set_label(const std::string& t_label) { m_label = t_label; }
  inline void set_percents(const double& t_percents) { m_percents = t_percents; }
  inline void set_colors(const Colors& t_col) { m_col = t_col; }
  inline void set_bold_text(const bool& t_is_bold) { m_is_bold_text = t_is_bold; }

  // Return -1 on successful, otherwise graph index
  // which didn't print due to terminal didn't have space (columns) for it.
  static int draw_graphs(std::ostream&, const std::vector<Graph>&);
  static Colors get_random_style();

private:
  std::string m_label;
  double m_percents;
  Colors m_col;
  bool m_is_bold_text;

  static const std::vector<Colors>
      m_graph_styles, m_graph_dark_styles, m_graph_light_styles;
};
