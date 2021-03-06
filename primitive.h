#ifndef PRIMITIVE_HPP
#define PRIMITIVE_HPP

#include "ast.h"
#include "attribute.h"

class Primitive
{
  public:
  int m_data;
  Attribute* m_parent_attribute;

  Primitive(const Primitive &);

  Primitive &operator=(const Primitive &);
  Primitive(int x);
  ~Primitive();
  virtual void accept(Visitor *v);
  LatticeElemMap* accept(CFVisitor *v, LatticeElemMap *in);
  virtual Primitive *clone() const;
  void swap(Primitive &);
};

#endif //PRIMITIVE_HPP
