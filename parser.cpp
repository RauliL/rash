#include "./rash.hpp"

std::vector<std::string>
parse(const std::string& input)
{
  std::vector<std::string> args;
  std::istringstream iss(input);
  std::string token;

  // Extract whitespace separated arguments from the input.
  // TODO: Add support for quoted arguments.
  // TODO: Add support for environment variables.
  // TODO: Add support for escaped characters.
  while (iss >> token)
  {
    args.push_back(token);
  }

  return args;
}
