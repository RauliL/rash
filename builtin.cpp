#include <unistd.h>

#include "./rash.hpp"

static int
command_chdir(
  const std::vector<std::string>& args,
  std::ostream&,
  std::istream&,
  std::ostream& err
)
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
      err << "chdir: too many arguments" << std::endl;

      return EXIT_FAILURE;
  }

  if (!std::filesystem::exists(path))
  {
    err << "chdir: no such file or directory: " << path << std::endl;

    return EXIT_FAILURE;
  }
  else if (!std::filesystem::is_directory(path))
  {
    err << "chdir: not a directory: " << path << std::endl;

    return EXIT_FAILURE;
  }
  else if (chdir(path.c_str()))
  {
    err << "chdir: failed to change directory: " << path << std::endl;

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

static int
command_exit(
  const std::vector<std::string>& args,
  std::ostream&,
  std::istream&,
  std::ostream& err
)
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
        err << "exit: invalid argument" << std::endl;

        return EXIT_FAILURE;
      }
      break;

    default:
      err << "exit: too many arguments" << std::endl;

      return EXIT_FAILURE;
  }
  std::exit(status_code);

  return EXIT_FAILURE;
}

std::unordered_map<std::string, builtin_command> builtins =
{
  { "cd", command_chdir },
  { "chdir", command_chdir },
  { "exit", command_exit },
};
