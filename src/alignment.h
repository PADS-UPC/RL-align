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

#ifndef _ALIGNMENT_H
#define _ALIGNMENT_H

#include <list>
#include <string>

class align_elem {
 public:
  std::string id;    // node id
  std::string name;  // task name
  std::string type;  // [L], [L/M], or [M-REAL]

  align_elem(const std::string &i, const std::string &n, const std::string &t);
  ~align_elem();

  std::string dump(bool id=false) const;

  bool operator<(const align_elem &a) const;
};


class alignment : public std::list<align_elem> {
 public:
  std::string dump(bool id=false) const;
  void purge();
};


#endif
