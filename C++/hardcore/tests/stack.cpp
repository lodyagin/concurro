#include <iostream>
#include "stack.hpp"

using namespace hc;

using A = void(int);
using PA = A*;

void req(int depth)
{
  std::cout << '.';
  if (depth <= 0)
  {
    std::cout << "stack frames:\n";
    for (auto& frame : stack::frames())
      std::cout << &frame << std::endl;

    std::cout << "returns:\n";
    for (auto& fun : stack::returns())
      std::cout << (void*) &fun << std::endl;
  }
  else req(depth - 1);
}

int main(int argc, char* argv[])
{
  std::cout << std::hex << __libc_stack_end << "\n\n";
  req(5);
}

