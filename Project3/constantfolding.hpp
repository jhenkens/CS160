#ifndef LATTICE_STUFF
#define LATTICE_STUFF

#include <map>
#include <string.h>

using namespace std;
    
static const int TOP = 0x7FFFFFFF;
static const int BOTTOM = 0x80000000;

class LatticeElem
{
  private:
    int value;

  public:

    LatticeElem(const int &value = BOTTOM) : value(value) {}

    int get_value() const { return value; } ;
    int set_value(const int &value) { this->value = value; }

    int join (const LatticeElem &other) const
    {
      if (other.value == TOP || this->value == TOP)
        return TOP;
      else if (this->value == BOTTOM)
	return other.value;
      else if (other.value == BOTTOM)
	return this->value;
      else if (other.value == this->value)
	return this->value;
      else
	return TOP;
    }
};

struct ltstr
{
  bool operator() (const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) < 0;
  }
};

typedef map<const char*, LatticeElem, ltstr> LatticeElemMap;


#endif //LATTICE_STUFF

