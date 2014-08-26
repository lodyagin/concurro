#include <iostream>
#include "stack.hpp"

void req(int depth)
{
  if (depth <= 0) {
    std::cout << "stack req (fp, ip):\n";
    for (auto frame : hc::stack())
      std::cout << '(' << frame.fp << ", " 
        << (void*) frame.ip << ")" << std::endl;
  }
  else req(depth - 1);
}

void req1() 
{ 
  std::cout << "stack req1 (fp, ip):\n";
  for (auto frame : hc::stack())
    std::cout << '(' << frame.fp << ", " 
      << (void*) frame.ip << ')' << std::endl;
  std::cout << '\n';
  req(0); 
}

void req2() 
{ 
  std::cout << "stack req2 (fp, ip):\n";
  for (auto frame : hc::stack())
    std::cout << '(' << frame.fp << ", " 
      << (void*) frame.ip << ')' << std::endl;
  std::cout << '\n';
  req1(); 
}

void req3() 
{ 
  std::cout << "stack req3 (fp, ip):\n";
  for (auto frame : hc::stack())
    std::cout << '(' << frame.fp << ", " 
      << (void*) frame.ip << ')' << std::endl;
  std::cout << '\n';
  req2(); 
}

void req4() { 
  std::cout << "stack req4 (fp, ip):\n";
  for (auto frame : hc::stack())
    std::cout << '(' << frame.fp << ", " 
      << (void*) frame.ip << ')' << std::endl;
  std::cout << '\n';
  req3(); 
}

int main(int argc, char* argv[])
{
  std::cout << std::hex << __libc_stack_end << "\n\n";
  req4();
  std::cout << "\nrecursion:\n";
  req(4);
}

