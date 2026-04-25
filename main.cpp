#include "./rash.hpp"

int
main()
{
  std::string input;

  init_env();
  for (;;)
  {
    std::cout << "# ";
    if (!std::getline(std::cin, input))
    {
      break;
    }
    execute(input, std::cout, std::cin, std::cerr);
  }
}
