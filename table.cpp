//table.cpp

#include "table.h"

Table::Table(){
  alm.push_back(new vector<Entry *>); 
}

void Table::levelUp() {
  alm.push_back(new vector<Entry *>);
}

void Table::levelDown() {
  alm.pop_back();
}

void Table::insert(string key,Value *s) {
  vector<Entry *> &v=*alm[alm.size()-1];
  for(size_t i=0; i<v.size(); i++) {
    if(v[i]->value && v[i]->key.compare(key)==0){
  //printf("redefine %d %s\n",i,s->name.c_str());
      v[i]->value=s;
      return;
    } 
  }
  //printf("insert %d %s\n",i,s->name.c_str());
    v.push_back(new Entry(key,s));
    return;
}

Value *Table::search(string key) {
  for(size_t l=alm.size(); l>=1; l--) { //size_t is unsigned
    vector<Entry *> &v=*alm[l-1];
    for(size_t i=0; i<v.size(); i++) {
    //printf("searching %s %s\n",key.c_str(),alm[i].key.c_str());
      if(v[i]->value && v[i]->key.compare(key)==0) return v[i]->value;
    }
  }
  return 0;
}

void Table::print() {
  printf("table print\n");
  for(size_t l=alm.size(); l>=1; l--) {
    vector<Entry *> &v=*alm[l-1];
    printf("level %ld \n",l-1);
    for(size_t i=0; i<v.size(); i++) {
      if(v[i]->value) {
        printf("* %s ",v[i]->key.c_str());
        v[i]->value->print();
      }
    }
  }
  printf("\n");
}
