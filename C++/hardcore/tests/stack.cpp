#include <iostream>
#include "stack.hpp"

using namespace hc;

void req(int depth)
{
  std::cout << '.';
  if (depth <= 0)
  {
    for (auto& frame : stack::frame())
      std::cout << &frame << std::endl;
  }
  else req(depth - 1);
}

int main(int argc, char* argv[])
{
  std::cout << std::hex << __libc_stack_end << "\n\n";
  req(5);
}
