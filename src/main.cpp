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

#include <string>
#include <vector>

#include "curlpp/cURLpp.hpp"
#include "instanalyzer.hpp"

using namespace std;

int main(int argc, char* argv[]) {
  curlpp::initialize();
  Instanalyzer::init();

  vector<string> params(argv + 1, argv + argc);
  Instanalyzer::parse_params(params);
  return 0;
}