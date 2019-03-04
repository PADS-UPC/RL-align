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


#ifndef _CONFIG_H
#define _CONFIG_H

#include <string>

class config {

 public:

    config();
    config(const std::string &fconfig);
    ~config();
  
    /// Relaxation Labelling parameters
    int MAX_ITER=500;
    double SCALE_FACTOR=100.0;
    double EPSILON=0.001;
    
    double DUMMY_INITIAL_WEIGHT = +0.1;
    /// Constraint default compatibilities and other stuff
    double DUMMY_COMPAT = +1.0;
    double EXCLUSIVE_COMPAT = -50.0;
    double CROSS_COMPAT = -10.0;
    double ORDER_COMPAT = +5.0;
    bool ORDER_PROGRESSIVE = false;
    double PARALLEL_COMPAT = +5.0;
    bool PARALLEL_PROGRESSIVE = false;
    double REPEAT_COMPAT = -5.0;
    int MAX_DIST = 3;
    
    // options about unfolding BP
    bool ADD_IFS = false;
    bool ADD_LOOPS = false;
    
};


#endif
