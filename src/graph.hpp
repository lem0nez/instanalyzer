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
  struct GraphInfo {
    std::string label;
    double percents;
    bool is_bold_text;

    // Text colors in and out of graph.
    Term::Color text_col_in;
    Term::Color text_col_out;
    Term::Color graph_col;
  };

  // Return -1 on successful, otherwise graph index
  // which didn't print due to terminal didn't have space (columns) for it.
  static int draw_graphs(std::ostream&, const std::vector<GraphInfo>&);
};
