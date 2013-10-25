#include "smtk/util/UUID.h"

#include <iostream>
#include <sstream>

#include <assert.h>

using smtk::util::UUID;

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  // Default constructor.
  UUID a;
  assert(a.IsNull() && "Empty constructor must create NULL UUID");
  assert(a.ToString() == "00000000-0000-0000-0000-000000000000" && "ToString(NULL)");

  // Raw data constructor
  uint8_t data[] = "\x00\x00\xff\xff\x00\x00\xff\xff\x00\x00\xff\xff\x00\x00\xff\xff";
  UUID fromRaw(data, data + 16);

  // String constructor
  UUID fromStr("a3d75703-fc9b-4d99-a104-ee67cf6d11b9");

  // Try the << operator
  std::ostringstream os;
  os << fromRaw;
  assert(fromRaw.ToString() == os.str() && "operator << failed");

  // Try the >> operator:
  std::istringstream is(fromStr.ToString());
  is >> fromRaw;
  assert(fromRaw == fromStr && "operator >> failed");

  // Test comparators
  UUID b(std::string("a3d75703-fc9b-4d99-a104-ee67cf6d11b9"));
  UUID c(std::string("5f0dee12-8b03-46dd-af36-ec8f9ca33882"));
  UUID d(std::string("5f0dee12-8b03-46dd-af36-ec8f9ca33882"));
  assert(c < b && "Less-than operator failed (TRUE)");
  assert(!(c < d) && "Less-than operator failed (FALSE)");
  assert(b != c && "Inequality operator failed (TRUE)");
  assert(!(c != c) && "Inequality operator failed (FALSE)");
  assert(c == d && "Equality operator failed (TRUE)");
  assert(!(b == c) && "Equality operator failed (FALSE)");

  // Generators:
  UUID e = UUID::Random();
  UUID f = UUID::Null();
  assert(!e.IsNull() && "Random() constructor must not create NULL UUID");
  assert(f.IsNull() && "Null() constructor must create NULL UUID");

  return 0;
}