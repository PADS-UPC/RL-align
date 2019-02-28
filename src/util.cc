
#include <iostream>
#include <sstream>
#include <algorithm>
#include "util.h"

using namespace std;

set<string> difference_set(const set<string> &big, const set<string> &small) {
  set<string> diff;
  set_difference(big.begin(), big.end(), small.begin(), small.end(), std::inserter(diff, diff.begin()));
  return diff;
}

set<string> union_set(const set<string> &big, const set<string> &small) {
  set<string> uni;
  set_union(big.begin(), big.end(), small.begin(), small.end(), std::inserter(uni, uni.begin()));
  return uni;
}

string set2string(const set<string> &ss) {  
  stringstream s;
  for (auto x : ss) s<<" "<<x;
  if (s.str().length()>0) return s.str().substr(1);
  else return "";
}

string vector2string(const vector<string> &ss) {  
  stringstream s;
  for (auto x : ss) s<<" "<<x;
  if (s.str().length()>0) return s.str().substr(1);
  else return "";
}
