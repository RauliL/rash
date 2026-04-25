#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <sys/wait.h>
#include <unistd.h>

using builtin_command = std::function<void(const std::vector<std::string>&)>;

static std::unordered_map<std::string, std::string> env;

static void
command_chdir(const std::vector<std::string>& args)
{
  std::string path;

  switch (args.size())
  {
    case 1:
      if (const auto home = std::getenv("HOME"))
      {
        path = home;
      }
      break;

    case 2:
      path = args[1];
      break;

    default:
      std::cerr << "chdir: too many arguments" << std::endl;
      return;
  }

  if (!std::filesystem::exists(path))
  {
    std::cerr << "chdir: no such file or directory: " << path << std::endl;
  }
  else if (!std::filesystem::is_directory(path))
  {
    std::cerr << "chdir: not a directory: " << path << std::endl;
  }
  else if (chdir(path.c_str()))
  {
    std::cerr << "chdir: failed to change directory: " << path << std::endl;
  }
}

static const std::unordered_map<std::string, builtin_command> builtin_commands =
{
  {
    "cd",
    command_chdir,
  },
  {
    "chdir",
    command_chdir,
  },
  {
    "exit",
    [](const std::vector<std::string>& args)
    {
      int status_code;

      switch (args.size())
      {
        case 1:
          status_code = EXIT_SUCCESS;
          break;

        case 2:
          try
          {
            status_code = std::stoi(args[1]);
          }
          catch (const std::invalid_argument&)
          {
            std::cerr << "exit: invalid argument" << std::endl;
            return;
          }
          break;

        default:
          std::cerr << "exit: too many arguments" << std::endl;
          return;
      }
      std::exit(status_code);
    },
  },
};

static std::optional<std::filesystem::path>
search_path(const std::string& executable)
{
  const auto it = env.find("PATH");

  if (it != std::end(env))
  {
    std::vector<std::filesystem::path> entries;
    std::string entry;
    std::istringstream iss(it->second);

    while (std::getline(iss, entry, ':'))
    {
      if (!entry.empty())
      {
        std::filesystem::path dir(entry);
        const auto candidate = dir / executable;

        if (std::filesystem::exists(candidate))
        {
          return candidate;
        }
      }
    }
  }

  return std::nullopt;
}

static std::vector<std::string>
parse_args(const std::string& input)
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

static bool
execute_builtin(const std::vector<std::string>& args)
{
  const auto it = builtin_commands.find(args[0]);

  if (it != builtin_commands.end())
  {
    it->second(args);

    return true;
  }

  return false;
}

static void
execute(const std::string& input)
{
  auto args = parse_args(input);
  pid_t pid;

  // Do nothing if the input was blank.
  if (args.empty())
  {
    return;
  }

  // Check for builtin shell commands first.
  if (execute_builtin(args))
  {
    return;
  }

  // If the first argument is not absolute path, search the $PATH environment
  // variable for a match.
  if (args[0].find(std::filesystem::path::preferred_separator) == std::string::npos)
  {
    const auto command = search_path(args[0]);

    // Woopsie doopsie. No such command was found so lets complain about it.
    if (!command)
    {
      std::cout << args[0] << ": not found" << std::endl;
      return;
    }
    args[0] = command->string();
  }

  // Fork the main process in order to prepare for the execve() call.
  pid = fork();
  if (pid == 0)
  {
    // We need to convert all std::string arguments into C style character
    // arrays for the execve() call. It also needs to be null terminated.
    // Same is true for the environment variables passed to execve().
    std::vector<const char*> child_args;
    std::vector<const char*> child_env;

    for (const auto& arg : args)
    {
      child_args.push_back(arg.c_str());
    }
    child_args.push_back(nullptr);

    for (const auto& [key, value] : env)
    {
      child_env.push_back((key + "=" + value).c_str());
    }
    child_env.push_back(nullptr);

    // Execute the command. On successful execution it should never return, so
    // if it does there was an error of some kind.
    execve(
      args[0].c_str(),
      const_cast<char* const*>(child_args.data()),
      const_cast<char* const*>(child_env.data())
    );

    // If we get at this point, there was an error of some kind. Output it to
    // the user and exit the forked process.
    std::cout << std::strerror(errno) << std::endl;
    std::exit(EXIT_FAILURE);
  } else {
    siginfo_t info;

    // Wait for the forked process to finish, since we don't want to run it in
    // the background.
    waitid(P_ALL, 0, &info, WEXITED);
  }
}

static void
initialize_env()
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

int
main()
{
  std::string input;

  initialize_env();
  for (;;)
  {
    std::cout << "# ";
    if (!std::getline(std::cin, input))
    {
      break;
    }
    execute(input);
  }
}
