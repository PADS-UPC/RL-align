
#include "alignment.h"

using namespace std;

align_elem::align_elem(const string &i, const string &n, const string &t) : id(i), name(n), type(t) {}

align_elem::~align_elem() {}

string align_elem::dump(bool use_id) const {
  return this->type + (use_id? this->id : this->name);
}

bool align_elem::operator<(const align_elem &a) const {
  return this->type < a.type;
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
