// Executables must have the following defined if the library contains
// doctest definitions. For builds with this disabled, e.g. code shipped to
// users, this can be left out.
#ifdef ENABLE_DOCTEST_IN_LIBRARY
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"
#endif

#include <iostream>
#include <stdlib.h>
#include <chrono>
#include <boost/version.hpp>

#include "config.h"

using namespace std;

int main()
{
  auto start = chrono::high_resolution_clock::now();
  // cout << "testing" << endl;
  auto finish = chrono::high_resolution_clock::now();
  cout << chrono::duration_cast<chrono::nanoseconds>(finish-start).count() << "ns\n";
  cout << "Boost version: " << BOOST_LIB_VERSION << endl;

  for(;;) {

  }
  return 0;
}
