#include <iostream>
#include "stack.hpp"

void req(int depth)
{
  std::cout << '.';
  if (depth <= 0)
  {
    std::cout << "stack (fp, ip):\n";
    for (auto frame : hc::stack())
      std::cout << '(' << frame.fp << ", " 
        << (void*) frame.ip << ')' << std::endl;
  }
  else req(depth - 1);
}

int main(int argc, char* argv[])
{
  std::cout << std::hex << __libc_stack_end << "\n\n";
  req(5);
}

