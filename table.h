//table.h
#include "value.h"

class Entry {
  public:
  string key;
  Value *value;
  Entry(string k, Value *v):key(k),value(v){}
};

class Table {
  vector<vector<Entry *>*> alm;
  public:
  Table();
  void levelUp();
  void levelDown();
  void insert(string key,Value *s);
  Value *search(string n);
  void print(); 
  ~Table() {}
};
