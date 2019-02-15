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

#include "comment.hpp"

#include <algorithm>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <vector>

#include "boost/regex.hpp"

#include "graph.hpp"
#include "instanalyzer.hpp"
#include "term.hpp"
#include "utils.hpp"

using namespace nlohmann;
using namespace std;

set<Comment> Comment::get_comments(const set<json>& t_posts) {
  set<Comment> comments;

  for (const auto& p : t_posts) {
    if (!Utils::has_json_node(p, {"node", "edge_media_to_comment", "edges"}) ||
        !Utils::has_json_node(p, {"node", "shortcode"})) {
      continue;
    }

    for (const auto& c : p["node"]["edge_media_to_comment"]["edges"]) {
      if (c.find("node") == c.end()) {
        continue;
      }

      const json& node = c["node"];
      Comment comment;

      comment.set_post_shortcode(p["node"].value("shortcode", ""));
      comment.set_id(node.value("id", ""));
      comment.set_text(node.value("text", ""));
      comment.set_creation_time(node.value("created_at", 0));
      comment.set_spam(node.value("did_report_as_spam", false));

      if (Utils::has_json_node(node, {"edge_liked_by", "count"})) {
        comment.set_likes(node["edge_liked_by"]["count"]);
      } else {
        comment.set_likes(0);
      }

      Profile profile;
      if (node.find("owner") != node.end()) {
        const json& owner = node["owner"];

        profile.set_id(owner.value("id", ""));
        profile.set_name(owner.value("username", ""));
        profile.set_verified(owner.value("is_verified", false));
      }
      comment.set_profile(profile);

      comments.insert(comment);
    }
  }
  return comments;
}

void Comment::show_commentators(const Profile& t_profile) {
  t_profile.check();

  cout << "\rProcessing posts..." << flush;
  const set<json>& posts = t_profile.get_posts();

  cout << Term::clear_line() + "Processing comments..." << flush;
  const set<Comment>& comments = get_comments(posts);
  map<Profile, unsigned int> commentators;

  for (const auto& c : comments) {
    ++commentators[c.get_profile()];
  }

  vector<pair<Profile, unsigned int>> commentators_sorted(
      commentators.cbegin(), commentators.cend());
  const auto& cmp = [] (const pair<Profile, unsigned int>& lhs,
      const pair<Profile, unsigned int>& rhs) {
    return lhs.second > rhs.second;
  };
  sort(commentators_sorted.begin(), commentators_sorted.end(), cmp);

  map<unsigned int, Graph::Colors> graphs_style;
  vector<Graph> graphs;

  for (const auto& c : commentators_sorted) {
    if (c.first.get_name().empty()) {
      continue;
    }

    Graph graph;
    graph.set_label('@' + c.first.get_name() + " (" + to_string(c.second) +
        " comment" + (c.second == 1 ? "" : "s") + ')');
    graph.set_percents((static_cast<double>(c.second) / comments.size()) * 100.0);

    if (graphs_style.count(c.second) == 0) {
      graphs_style[c.second] = Graph::get_random_style();
    }

    graph.set_colors(graphs_style[c.second]);
    graph.set_bold_text(c.first.get_name() == t_profile.get_name());
    graphs.push_back(graph);
  }

  cout << Term::clear_line() << flush;
  if (graphs.empty()) {
    Instanalyzer::msg(Instanalyzer::MSG_WARN, "No commentators!");
    return;
  }

  cout << Term::process_colors("Processing done.\n\n"
      "#{bold}> Most active commentators#{reset}") << endl;
  Graph::draw_graphs(cout, graphs);
  cout << endl;
  Instanalyzer::msg(Instanalyzer::MSG_INFO, Term::process_colors(
      "Total comments: #{blue_out}" + to_string(comments.size()) +
      "#{reset}; commentators: #{blue_out}" +
      to_string(commentators_sorted.size()) + "#{reset}."));
}

void Comment::show_commentator_info(const Profile& t_owner,
    const string& t_commentator) {
  t_owner.check();

  cout << "\n\rProcessing posts..." << flush;
  const set<json>& posts = t_owner.get_posts();

  cout << Term::clear_line() + "Processing comments..." << flush;
  const set<Comment>& all_comments = get_comments(posts);
  vector<Comment> target_comments;

  for (const auto& c : all_comments) {
    if (c.get_profile().get_name() == t_commentator) {
      target_comments.push_back(c);
    }
  }

  if (target_comments.empty()) {
    cout << Term::clear_line() << flush;
    Instanalyzer::msg(Instanalyzer::MSG_WARN, Term::process_colors(
        "No comments for #{orange_out}@" + t_commentator + "#{reset}!"));
    return;
  }

  unsigned int likes = 0, spam_count = 0;
  for (const auto& c : target_comments) {
    likes += c.get_likes();
    spam_count += c.is_spam();
  }

  cout << Term::clear_line() + Term::process_colors("Commentator: #{blue_out}" +
      t_commentator + "#{reset}") << endl;

  if (likes != 0) {
    cout << Term::process_colors(
        "Total likes: #{blue_out}" + to_string(likes) + "#{reset}") << endl;
  }
  if (spam_count != 0) {
    cout << Term::process_colors("Spam comments: #{blue_out}" +
        to_string(spam_count) + "#{reset}") << endl;
  }

  print_comments(set<Comment>(target_comments.cbegin(),
      target_comments.cend()), "Comments");
  show_references(t_owner, t_commentator, all_comments);
}

void Comment::print_comments(const set<Comment>& t_comments,
    const std::string& t_label) {
  ostringstream ss;

  for (const auto& c : t_comments) {
    if (c.get_text().empty() || c.get_creation_time() == 0 ||
        c.get_post_shortcode().empty()) {
      continue;
    }

    const time_t& creation_time = c.get_creation_time();
    ostringstream date;
    date << put_time(localtime(&creation_time), "%a %b %d %H:%M %Y");

    const unsigned int& comment_likes = c.get_likes();
    const string& text_likes = to_string(comment_likes) + " like" +
        (comment_likes == 1 ? "" : "s");

    const string& additional_info = '(' + date.str() +
        (comment_likes == 0 ? "" : ", " + text_likes) +
        ", #{cream_out}" + c.get_post_shortcode() + "#{reset})";

    const string& text = Term::process_colors(string("  #{bold}") + (c.is_spam() ?
        "#{red_out}*" : "#{green_out}-") + string("#{reset}#{gray_out} ") +
        c.get_text() + " #{reset}" + additional_info);

    ss << text << endl;
  }

  if (!ss.str().empty()) {
    if (!t_label.empty()) {
      const time_t& time = 0;
      ostringstream time_zone;
      time_zone << put_time(localtime(&time), "%Z");

      cout << Term::process_colors(
          "#{bold}" + t_label + " (#{red_out}*#{reset}#{bold} - spam, #{gray_out}" +
          time_zone.str() + "#{reset} time zone):#{reset}") << endl;
    }
    cout << ss.str() << flush;
  }
}

void Comment::show_references(const Profile& t_owner,
    const string& t_commentator, set<Comment> t_comments) {
  using namespace boost;

  if (t_comments.empty()) {
    cout << "\rProcessing posts..." << flush;
    const set<json>& posts = t_owner.get_posts();

    cout << Term::clear_line() + "Processing comments..." << flush;
    t_comments = get_comments(posts);
    cout << Term::clear_line() << flush;
  }

  cout << "\rSearching references..." << flush;

  const regex& reg = regex("^.*@" + t_commentator + "(\\s.*$|$)", regex::extended);
  smatch match;
  set<Comment> ref_comments;

  for (const auto& c : t_comments) {
    if (regex_match(c.get_text(), match, reg) &&
        c.get_profile().get_name() == t_owner.get_name()) {
      ref_comments.insert(c);
    }
  }

  cout << Term::clear_line() << flush;
  if (!ref_comments.empty()) {
    cout << Term::process_colors("\nTotal references: #{blue_out}" +
        to_string(ref_comments.size()) + "#{reset}") << endl;
    print_comments(ref_comments, "Owner references");
  }
}
