#include <cerrno>
#include <cstring>

#include <sys/wait.h>
#include <unistd.h>

#include "./rash.hpp"

static inline std::optional<int>
execute_builtin(
  const std::vector<std::string>& args,
  std::ostream& out,
  std::istream& in,
  std::ostream& err
)
{
  const auto it = builtins.find(args[0]);

  if (it != std::end(builtins))
  {
    return it->second(args, out, in, err);
  }

  return std::nullopt;
}

static inline bool
is_absolute_path(const std::string& path)
{
  return path.find(std::filesystem::path::preferred_separator) != std::string::npos;
}

int
execute(
  const std::string& input,
  std::ostream& out,
  std::istream& in,
  std::ostream& err
)
{
  auto args = parse(input);
  pid_t pid;

  // Do nothing if the input was blank.
  if (args.empty())
  {
    return EXIT_SUCCESS;
  }

  // Check for builtin shell commands first.
  if (const auto result = execute_builtin(args, out, in, err))
  {
    return *result;
  }

  // If the first argument is not absolute path, search the $PATH environment
  // variable for a match.
  if (!is_absolute_path(args[0]))
  {
    const auto command = search_for_executable(args[0]);

    // Woopsie doopsie. No such command was found so lets complain about it.
    if (!command)
    {
      err << args[0] << ": not found" << std::endl;

      return EXIT_FAILURE;
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

    return info.si_status;
  }

  return EXIT_FAILURE;
}
