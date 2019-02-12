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

#include <map>

#include "instanalyzer.hpp"
#include "utils.hpp"

using namespace nlohmann;
using namespace std;

set<Comment> Comment::get_comments(const json& t_post) {
  if (!Utils::has_json_node(t_post, {"node", "edge_media_to_comment", "edges"}) ||
      !Utils::has_json_node(t_post, {"node", "shortcode"})) {
    return {};
  }

  set<Comment> comments;

  for (const auto& c : t_post["node"]["edge_media_to_comment"]["edges"]) {
    if (c.find("node") == c.end()) {
      continue;
    }

    const json& node = c["node"];
    Comment comment;

    comment.set_post_shortcode(t_post["node"].value("shortcode", ""));
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

  return comments;
}
