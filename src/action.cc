// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include "action.h"

#include <vector>

#include "util.h"

using std::string;
using std::vector;

namespace wmderland {

Action::Action(const string& s) {
  // For example, "goto_workspace 1" is an action.
  // We split this string into two tokens by whitespace.
  vector<string> tokens = string_utils::Split(s, ' ', 1);

  // The first token is an action type.
  type_ = Action::StrToActionType(tokens[0]);

  // The second token (if exists) is an argument.
  if (tokens.size() > 1) {
    argument_ = tokens[1];
  }
}

Action::Action(Action::Type type) : type_(type) {}

Action::Action(Action::Type type, const string& arguments) 
    : type_(type), argument_(arguments) {}


Action::Type Action::type() const {
  return type_;
}

const string& Action::argument() const {
  return argument_;
}


Action::Type Action::StrToActionType(const string& s) {
  if (s == "tile_horizontally") {
    return Action::Type::TILE_H;
  } else if (s == "tile_vertically") {
    return Action::Type::TILE_V;
  } else if (s == "focus_left") {
    return Action::Type::FOCUS_LEFT;
  } else if (s == "focus_right") {
    return Action::Type::FOCUS_RIGHT;
  } else if (s == "focus_down") {
    return Action::Type::FOCUS_DOWN;
  } else if (s == "focus_up") {
    return Action::Type::FOCUS_UP;
  } else if (s == "toggle_floating") {
    return Action::Type::TOGGLE_FLOATING;
  } else if (s == "toggle_fullscreen") {
    return Action::Type::TOGGLE_FULLSCREEN;
  } else if (s == "goto_workspace") {
    return Action::Type::GOTO_WORKSPACE;
  } else if (s == "move_app_to_workspace") {
    return Action::Type::MOVE_APP_TO_WORKSPACE;
  } else if (s == "kill") {
    return Action::Type::KILL;
  } else if (s == "exit") {
    return Action::Type::EXIT;
  } else if (s == "exec") {
    return Action::Type::EXEC;
  } else if (s == "reload") {
    return Action::Type::RELOAD;
  } else {
    return Action::Type::UNDEFINED;
  }
}

} // namespace wmderland
