#pragma once

#include <filesystem>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

using builtin_command = std::function<
  int(
    const std::vector<std::string>&,
    std::ostream&,
    std::istream&,
    std::ostream&
  )
>;

extern std::unordered_map<std::string, std::string> env;
extern std::unordered_map<std::string, builtin_command> builtins;

// env.cpp
void init_env();
std::optional<std::filesystem::path> search_for_executable(const std::string&);

// exec.cpp
int execute(const std::string&, std::ostream&, std::istream&, std::ostream&);

// parser.cpp
std::vector<std::string> parse(const std::string&);
