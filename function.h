//function.h
#ifndef FUNCTION_H
#define FUNCTION_H
#include <vector>
using namespace std;

class Function {
  public:
  vector <string> parameters;
  string body;
  Function(vector <string> p, string b):parameters(p),body(b){}
};
#endif
