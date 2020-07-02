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
#include <fstream>
#include "bp.h"

using namespace std;


/// empty constructor

behavioral_profile::behavioral_profile() {}
  

/// constructor, load BP from a file

behavioral_profile::behavioral_profile(const std::string &bpfile) {
  
  ifstream fin;
  fin.open(bpfile);
  string n1,n2,rel;
  while (fin>>n1>>n2>>rel) {
    relType r = get_rel_type(rel);
    add_relation(n1,n2,r);
    if (n1!=n2) add_relation(n2,n1,inverse(r));   
  }
}

/// destructor

behavioral_profile::~behavioral_profile() {}

/// get existing relation between two nodes

behavioral_profile::relType behavioral_profile::get_relation(const string &n1, const string &n2) const {
  auto p = relations.find(make_pair(n1,n2));
  if (p!=relations.end()) return p->second;
  else return NO_RELATION;
}

/// delete existing relation between two nodes

void behavioral_profile::remove_relation(const string &node1, const string &node2) {
  auto p = relations.find(make_pair(node1,node2));
  if (p!=relations.end()) relations.erase(p);
}

/// add relation between two nodes (overwritting previous relation)

void  behavioral_profile::add_relation(const string &node1, const string &node2, relType rel) {
  remove_relation(node1,node2);
  relations.insert(make_pair(make_pair(node1,node2), rel));
}

/// convert from relation symbol to internal code

behavioral_profile::relType behavioral_profile::get_rel_type(const std::string &relname) {
  if (relname=="->") return PRECEDES;
  else if (relname=="<-") return FOLLOWS;
  else if (relname=="||") return INTERLEAVED;
  else if (relname=="+" or relname=="xx") return EXCLUSIVE;
  else return NO_RELATION;
}

/// convert from relation internal code to printable symbol

std::string behavioral_profile::get_rel_name(relType r, bool table) {
  switch (r) {
    case PRECEDES : return "->";
    case FOLLOWS: return "<-";
    case INTERLEAVED: return "||";
    case EXCLUSIVE: if (table) return "xx"; else return "+";

    case NO_RELATION: 
    default:
      return "ERROR";
  }

}

/// obtain the inverse relation of r    

behavioral_profile::relType behavioral_profile::inverse(relType r) {
  switch (r) {
    case PRECEDES : return FOLLOWS;
    case FOLLOWS: return PRECEDES;
    default: return r;
  }
}

/// return BP as a string, for tracing

string behavioral_profile::dump(bool table) const {

  string s = "";
  string ant = "";
  for (auto r : relations) {
    if (table) {
      if (r.first.first != ant) s += "\n ";
      else s += "\t";
      s += r.first.first + " " + get_rel_name(r.second,table) + " " + r.first.second;
      ant = r.first.first;
    }
    else
      s += r.first.first + " " + r.first.second + " " + get_rel_name(r.second,table) + "\n";      
  }
  return s;
}
