//////////////////////////////////////////////////////////////////
//
//    Copyright (C) 2019  Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Affero General Public
//    License as published by the Free Software Foundation; either
//    version 3 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    contact: Lluis Padro (padro@cs.upc.es)
//             Computer Science Department
//             Omega-320 - Campus Nord UPC
//             C/ Jordi Girona 31
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////

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

string list2string(const list<string> &ss) {  
  stringstream s;
  for (auto x : ss) s<<" "<<x;
  if (s.str().length()>0) return s.str().substr(1);
  else return "";
}
