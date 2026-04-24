#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include <sys/wait.h>
#include <unistd.h>

#if !defined(BUFSIZ)
#  define BUFSIZ 1024
#endif

static std::optional<std::filesystem::path>
search_path(const std::string& executable)
{
  if (const auto path_var = std::getenv("PATH"))
  {
    std::vector<std::filesystem::path> entries;
    std::string entry;
    std::istringstream iss(path_var);

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

static void
execute(const std::string& input)
{
  std::vector<std::string> args;
  std::istringstream iss(input);
  std::string token;
  pid_t pid;

  // Extract whitespace separated arguments from the input.
  while (iss >> token)
  {
    args.push_back(token);
  }

  // Do nothing if the input was blank.
  if (args.empty())
  {
    return;
  }

  // Check for builtin shell commands.
  if (args[0] == "exit")
  {
    std::exit(EXIT_SUCCESS);
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
    std::vector<const char*> child_args;

    for (const auto& arg : args)
    {
      child_args.push_back(arg.c_str());
    }
    child_args.push_back(nullptr);

    // Execute the command. On successful execution it should never return, so
    // if it does there was an error of some kind.
    execve(
      args[0].c_str(),
      const_cast<char* const*>(child_args.data()),
      environ
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

int
main()
{
  std::string input;

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
