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

#include "boost/format.hpp"
#include "boost/regex.hpp"

using namespace std;

int Graph::draw_graphs(ostream& t_os, const vector<GraphInfo>& t_graphs) {
  using namespace boost;

  static const unsigned short MAX_TERM_COLUMNS = 80;
  const unsigned int term_columns = (Term::get_columns() > MAX_TERM_COLUMNS) ?
      MAX_TERM_COLUMNS : Term::get_columns();
  size_t idx = 0;

  for (const auto& g : t_graphs) {
    string percents = (format("%.1f %%") % g.percents).str();
    // Delete all text colors and styles.
    string label = regex_replace(g.label, regex("\x1b\\[[[:digit:];]+m"), "");
    // 1 is space to separate label and percents.
    const int max_label_len = term_columns - percents.length() - 1;

    // 4 is first character of label and three dots.
    if (max_label_len < 4) {
      return idx;
    } else if (static_cast<size_t>(max_label_len) < label.length()) {
      label = label.substr(0, max_label_len - 3) + "...";
    }

    const string& graph_start = Term::get_color(g.graph_col, true) +
        Term::get_color(g.text_col_in) + (g.is_bold_text ? Term::bold() : "");
    const size_t spaces = term_columns - label.length() - percents.length(),
        graph_end_char = term_columns * (g.percents / 100.0) + graph_start.length();

    string graph =
        graph_start + label + string(spaces, ' ') + percents + Term::reset();
    graph.insert(graph_end_char, Term::reset() +
        Term::get_color(g.text_col_out) + (g.is_bold_text ? Term::bold() : ""));

    t_os << graph << endl;
    ++idx;
  }

  return -1;
}
