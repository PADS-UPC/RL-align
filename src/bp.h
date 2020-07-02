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

#ifndef __BP_H
#define __BP_H

#include <map>
#include <list>

class behavioral_profile {

 public:
   typedef enum {NO_RELATION, PRECEDES, FOLLOWS, INTERLEAVED, EXCLUSIVE} relType;

   // empty constructor
   behavioral_profile();
   // constructor, load BP from a file
   behavioral_profile(const std::string &bpfile);
   // destructor
  ~behavioral_profile();

   // add relation between two nodes (overwritting previous relation) 
   void add_relation(const std::string &node1, const std::string &node2, relType rel);
   // remove relation between two nodes
   void remove_relation(const std::string &node1, const std::string &node2);
   // get existing relation between two nodes
   relType get_relation(const std::string &n1, const std::string &n2) const;
   // convert from relation internal code to printable symbol
   static std::string get_rel_name(relType rt, bool table=false);
   // return BP as a string, for tracing. Two possible formats: table or list
   std::string dump(bool table=false) const;
   // obtain the inverse relation of r
   static relType inverse(relType r);
   
 private:
   std::map<std::pair<std::string,std::string>, relType> relations;

   // convert from relation symbol to internal code
   static relType get_rel_type(const std::string &relname);
   
};

#endif
