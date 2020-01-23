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


#include "alignment.h"

using namespace std;

align_elem::align_elem(const string &i, const string &n, const string &t) : id(i), name(n), type(t) {}

align_elem::~align_elem() {}

string align_elem::dump(bool show_id) const {
  return this->type + this->name + (show_id ? "("+this->id+")" : "");
}

bool align_elem::operator<(const align_elem &e) const {
  bool b;
  if (this->type < e.type) b = true;
  else if (this->type == e.type and this->name < e.name) b = true;
  else if (this->type == e.type and this->name == e.name and this->id < e.id) b = true;
  else b = false;
  return b;
}

bool align_elem::operator==(const align_elem &e) const {
  return (this->type == e.type and this->name == e.name and this->id == e.id);
}

void alignment::purge() {
  auto prev = this->begin();
  auto ev = prev; ++ev;
  while (ev!=this->end()) {
    if (prev->type=="[L]" and ev->type=="[M-REAL]" and prev->name==ev->name) {
      ev->type = "[L/M]";
      this->erase(prev);
    }

    prev = ev;
    ++ev;
  }
}

string alignment::dump(bool id) const {
  string s;
  for (auto e : *this) 
    s += "|" + e.dump(id);
  return (s.length()>1 ? s.substr(1) : s);
}


