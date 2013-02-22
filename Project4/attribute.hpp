#ifndef ATTRIBUTE_HPP
#define ATTRIBUTE_HPP

#include <stdlib.h>
#include <map>
#include <string.h>
#include <stdio.h>
#include <iostream>

using namespace std;

//SymScope is declared in SymTab.cpp
class SymScope;
class LatticeElem;

enum Basetype
{
	bt_undef = 0x01,
	bt_integer = 0x02,
	bt_boolean = 0x04,
	bt_function = 0x08,
	bt_intarray = 0x10
};

static const int TOP = 0x7FFFFFFF;
static const int BOTTOM = 0x80000000;

class LatticeElem
{
  public:
    int value;

    LatticeElem(const int &value = BOTTOM) : value(value) {}
    LatticeElem(const LatticeElem &other) : value(other.value) {}

    LatticeElem* clone() const { return new LatticeElem(this->value); }

    bool operator == (const int &other) const { return other == this->value; }
    bool operator == (const LatticeElem &other) const { return other.value == this->value; }
    bool operator != (const int &other) const { return other != this->value; }
    bool operator != (const LatticeElem &other) const { return other.value != this->value; }

    LatticeElem& operator = (const LatticeElem &other) { this->value = other.value; return *this; }
    LatticeElem& operator = (const int &value) { this->value = value; return *this; }

    // Joins two lattice elements. The result is stored in the first element!
    void join (const LatticeElem &other)
    {
      if (other.value == TOP)
        this->value = TOP;
      else if (this->value == BOTTOM)
	this->value = other.value;
      else if (other.value == BOTTOM)
	; // don't do anything
      else if (this->value != other.value)
        this->value = TOP;
    }

    const char* to_string(char* buffer) const
    {
	if (this->value == TOP)
		sprintf(buffer, "TOP");
	else if (this-> value == BOTTOM)
		sprintf(buffer, "BOTTOM");
	else
		sprintf(buffer, "%d", this->value);
	return buffer;
    }
};

class Attribute
{
  public:
  Basetype m_basetype; //type of the subtree
  int m_place; //register where this value is stored
  SymScope* m_scope; //the scope of the current symbol
  int lineno; //line number on which that ast node resides
  LatticeElem m_lattice_elem; //the value of this expression; only to be used for expression nodes

  Attribute() { 
	m_basetype = bt_undef;
	lineno = 0;
	m_scope = NULL;
	m_lattice_elem.value = BOTTOM;
	int m_place = -1; //should never actually be negative
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

// Joins the two lattice elem maps. The result is stored in the first map
inline void join_lattice_elem_maps (LatticeElemMap *map1, LatticeElemMap *map2) {
	LatticeElemMap::iterator iter;

	for (iter = map1->begin(); iter != map1->end(); iter++) {
		iter->second.join((*map2)[iter->first]);
	}
}

inline bool lattice_maps_equal (LatticeElemMap *map1, LatticeElemMap *map2) {
	LatticeElemMap::iterator iter;
	for (iter = map1->begin(); iter != map1->end(); iter++)
		if (((*map2)[iter->first]) != iter->second)
			return false;
	for (iter = map2->begin(); iter != map2->end(); iter++)
		if (((*map1)[iter->first]) != iter->second)
			return false;
	return true;
}

inline void print_lattice_map(LatticeElemMap *map) {
	cout << "{";
	LatticeElemMap::iterator iter;
	for (iter = map->begin(); iter != map->end(); iter++)
		cout << " (" << iter->first << " = " << iter->second.value << ')';
	cout << " }";
}

#endif //ATTRIBUTE_HPP

