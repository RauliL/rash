#include <cstring>

#include <unistd.h>

#include "./rash.hpp"

std::unordered_map<std::string, std::string> env;

void
init_env()
{
  for (auto e = environ; *e; ++e)
  {
    const auto pos = std::strchr(*e, '=');

    if (pos != nullptr)
    {
      *pos = 0;
      env[*e] = pos + 1;
    }
  }
}

std::optional<std::string>
get_env(const std::string& name)
{
  const auto it = env.find(name);

  if (it != std::end(env))
  {
    return it->second;
  }

  return std::nullopt;
}

static std::vector<std::filesystem::path>
split_path(const std::string& var)
{
  const auto it = env.find(var);
  std::vector<std::filesystem::path> result;

  if (it != std::end(env))
  {
    std::istringstream iss(it->second);
    std::string token;

    while (std::getline(iss, token, ':'))
    {
      if (!token.empty())
      {
        result.emplace_back(token);
      }
    }
  }

  return result;
}

std::optional<std::filesystem::path>
search_for_executable(const std::string& name)
{
  for (const auto& dir : split_path("PATH"))
  {
    const auto path = dir / name;

    if (std::filesystem::exists(path))
    {
      return path;
    }
  }

  return std::nullopt;
}
