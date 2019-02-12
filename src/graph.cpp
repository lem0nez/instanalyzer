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

#include "graph.hpp"

#include <random>

#include "boost/format.hpp"
#include "boost/regex.hpp"

using namespace std;

const vector<Graph::Colors> Graph::m_graph_styles = {
  {Term::COL_RED, Term::COL_BLACK, Term::COL_RED},
  {Term::COL_ORANGE, Term::COL_BLACK, Term::COL_ORANGE},
  {Term::COL_YELLOW, Term::COL_BLACK, Term::COL_YELLOW},
  {Term::COL_GREEN, Term::COL_BLACK, Term::COL_GREEN},
  {Term::COL_BLUE, Term::COL_BLACK, Term::COL_BLUE},
  {Term::COL_GRAY, Term::COL_BLACK, Term::COL_GRAY}
};

const vector<Graph::Colors> Graph::m_graph_dark_styles = {
  {Term::COL_WHITE, Term::COL_BLACK, Term::COL_WHITE},
  {Term::COL_CREAM, Term::COL_BLACK, Term::COL_CREAM}
};

const vector<Graph::Colors> Graph::m_graph_light_styles = {
  {Term::COL_BLACK, Term::COL_WHITE, Term::COL_BLACK}
};

int Graph::draw_graphs(ostream& t_os, const vector<Graph>& t_graphs) {
  using namespace boost;

  static const unsigned short MAX_TERM_COLUMNS = 80;
  const unsigned int term_columns = (Term::get_columns() > MAX_TERM_COLUMNS) ?
      MAX_TERM_COLUMNS : Term::get_columns();
  size_t idx = 0;

  for (const auto& g : t_graphs) {
    string percents = (format("%.1f %%") % g.get_percents()).str();
    // Delete all text colors and styles.
    string label = regex_replace(g.get_label(),
        regex("\x1b\\[[[:digit:];]+m"), "");
    // 1 is space to separate label and percents.
    const int max_label_len = term_columns - percents.length() - 1;

    // 4 is first character of label and three dots.
    if (max_label_len < 4) {
      return idx;
    } else if (static_cast<size_t>(max_label_len) < label.length()) {
      label = label.substr(0, max_label_len - 3) + "...";
    }

    const string& graph_start = Term::get_color(g.get_colors().graph, true) +
        Term::get_color(g.get_colors().text_in) +
        (g.is_bold_text() ? Term::bold() : "");
    const size_t spaces = term_columns - label.length() - percents.length(),
        graph_end_char = term_columns * (g.get_percents() / 100.0) +
        graph_start.length();

    string graph =
        graph_start + label + string(spaces, ' ') + percents + Term::reset();
    graph.insert(graph_end_char, Term::reset() + Term::get_color(
        g.get_colors().text_out) + (g.is_bold_text() ? Term::bold() : ""));

    t_os << graph << endl;
    ++idx;
  }
  return -1;
}

Graph::Colors Graph::get_random_style() {
  vector<Colors> styles(m_graph_styles);

  if (Term::is_dark_theme()) {
    styles.insert(styles.cend(),
        m_graph_dark_styles.cbegin(), m_graph_dark_styles.cend());
  } else {
    styles.insert(styles.cend(),
        m_graph_light_styles.cbegin(), m_graph_light_styles.cend());
  }

  random_device rd;
  mt19937 engine(rd());
  uniform_int_distribution<int> uni(0, styles.size() - 1);

  // For prevent repeat of styles one after another.
  static size_t previous_idx;
  size_t idx;

  do {
    idx = uni(rd);
  } while (idx == previous_idx);
  previous_idx = idx;

  return styles[idx];
}
