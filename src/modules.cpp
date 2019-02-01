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

#include "modules.hpp"

#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>

#include "boost/process.hpp"
#include "boost/regex.hpp"
#include "curlpp/Easy.hpp"
#include "curlpp/Options.hpp"
#include "libzippp.h"

#include "term.hpp"

using namespace std;

const vector<Modules::ZipModuleInfo> Modules::m_zip_modules = {
  {
    "certifi-2018.11.29",
    "https://api.github.com/repos/certifi/python-certifi/zipball/2018.11.29",
    {"certifi-python-certifi-10a1f8a/certifi"}
  },
  {
    "chardet-3.0.4",
    "https://api.github.com/repos/chardet/chardet/zipball/3.0.4",
    {"chardet-chardet-1ef35b0/chardet"}
  },
  {
    "idna-2.8",
    "https://api.github.com/repos/kjd/idna/zipball/v2.8",
    {"kjd-idna-375dc46/idna"}
  },
  {
    "instaloader-4.2.1",
    "https://api.github.com/repos/instaloader/instaloader/zipball/v4.2.1",
    {
      "instaloader-instaloader-213b78c/instaloader",
      "instaloader-instaloader-213b78c/instaloader.py"
    }
  },
  {
    "requests-2.21.0",
    "https://api.github.com/repos/requests/requests/zipball/v2.21.0",
    {"requests-requests-5a1e738/requests"}
  },
  {
    "urllib3-1.24.1",
    "https://api.github.com/repos/urllib3/urllib3/zipball/1.24.1",
    {"urllib3-urllib3-2cc8363/src/urllib3"}
  }
};

filesystem::path Modules::m_interpreter_path;

void Modules::init_interpreter() {
  using namespace boost::process;

  const set<filesystem::path> bin_paths = {
    "/bin", "/usr/bin", "/usr/local/bin", "/sbin", "/opt"
  };
  const set<string> bin_names = {
    "python3.7", "python3.7m", "python3.6", "python3.6m",
    "python3.5", "python3.5m", "python3", "python3m"
  };
  const boost::regex python_ver("^Python 3\\.[5-9]\\.[0-9]+$");

  for (const auto& p : bin_paths) {
    for (const auto& n : bin_names) {
      if (filesystem::directory_entry(p / n).exists()) {
        ipstream pstream;
        child c((p / n).c_str() + string(" --version"), std_out > pstream);

        string output;
        getline(pstream, output);
        boost::smatch match;

        if (boost::regex_match(output, match, python_ver)) {
          m_interpreter_path = p / n;
          return;
        }
      }
    }
  }
  throw runtime_error(Term::process_colors("Python interpreter with minimal "
      "version #{bold}3.5#{reset} didn't find! Please, install it."));
}

void Modules::update_modules() {
  using namespace curlpp;
  using namespace filesystem;
  using namespace libzippp;

  cout << "Updating modules..." << endl;

  try {
    remove_all(get_modules_path());
  } catch (const exception& e) {}

  if (!create_directory(get_modules_path())) {
    throw runtime_error("Parent directory for modules (" +
        string(get_modules_path()) + ") didn't create!");
  }

  for (const auto& m : m_zip_modules) {
    cout << Term::process_colors(
        "  #{gray_out}" + m.name + "\n\r#{reset}#{bold}Downloading...") << flush;
    const path& archive_path = Instanalyzer::get_tmp_prefix() + m.name + ".zip";

    try {
      remove(archive_path);
    } catch (const exception& e) {}

    ofstream archive(archive_path);
    options::WriteStream ws(&archive);
    Easy request;

    request.setOpt(options::Url(m.url));
    request.setOpt(options::WriteStream(ws));
    request.setOpt(options::FollowLocation(1));
    request.setOpt(options::UserAgent("instanalyzer"));

    request.perform();
    archive.close();

    cout << '\r' + string(Term::get_columns(), ' ') +
        Term::process_colors("\r#{bold}Extracting...") << flush;
    ZipArchive za(archive_path);
    za.open();

    if (za.getEntriesCount() == 0) {
      throw runtime_error(
          "Archive \"" + string(archive_path) + "\" doesn't contain any entry!");
    }

    const auto& entries = za.getEntries();
    for (const auto& e : entries) {
      for (const auto& p : m.paths) {
        if (e.getName().substr(0, p.size()) != p) {
          continue;
        }

        path target_path = get_modules_path() / p.substr(p.find_last_of('/') + 1);
        const string& target_child = e.getName().substr(p.size());

        if (target_child.size() > 1) {
          // Skip first slash character.
          target_path =
              target_path / target_child.substr(1);
        }

        if (e.isDirectory()) {
          if (!directory_entry(target_path).exists() &&
              !create_directory(target_path)) {
            throw runtime_error(
                "Child directory \"" + string(target_path) + "\" didn't create!");
          }
        } else {
          ofstream ofs(target_path);
          e.readContent(ofs);
        }
      }
    }
    remove(archive_path);
    cout << '\r' + string(Term::get_columns(), ' ') + '\r' + Term::reset()
        << flush;
  }
  cout << "All done." << endl;
}

void Modules::instaloader(const string& t_params,
    const Modules::parser_cb t_cb_out, const Modules::parser_cb t_cb_err) {
  using namespace boost::process;

  ipstream out_pstream, err_pstream;
  const string& exec = string(m_interpreter_path) + ' ' +
      string(get_instaloader_path()) + ' ' + t_params;
  child c(exec, std_out > out_pstream, std_err > err_pstream);

  string out_line, err_line;
  while (out_pstream || err_pstream) {
    if (getline(out_pstream, out_line) && !out_line.empty() &&
        t_cb_out != nullptr) {
      t_cb_out(out_line);
    }
    if (getline(err_pstream, err_line) && !err_line.empty() &&
        t_cb_err != nullptr) {
      t_cb_err(err_line);
    }
  }
}
