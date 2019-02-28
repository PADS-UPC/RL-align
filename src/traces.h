//////////////////////////////////////////////////////////////////
//
//    FreeLing - Open Source Language Analyzers
//
//    Copyright (C) 2014   TALP Research Center
//                         Universitat Politecnica de Catalunya
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
//    contact: Lluis Padro (padro@lsi.upc.es)
//             TALP Research Center
//             despatx C6.212 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////

#ifndef _TRACES_H
#define _TRACES_H

#include <iostream>
#include <string>
#include <list>
#include <cstdlib>

/// possible values for MOD_TRACECODE
#define MAIN_TRACE          0x00000001
#define GRAPH_TRACE         0x00000002
#define BP_TRACE            0x00000004
#define CFG_TRACE           0x00000008
#define ALIGNMENT_TRACE     0x00000010
#define RELAX_TRACE         0x00000020

// MOD_TRACECODE and MOD_TRACENAME are empty. The class 
// using the trace is expected to set them
#undef MOD_TRACECODE
#undef MOD_TRACENAME

#undef TRACE
#undef ERROR_CRASH
#undef WARNING

////////////////////////////////////////////////////////////////
///  Class traces implements trace and error handling utilities.
////////////////////////////////////////////////////////////////

class traces {
 public:
  // current trace level
  static int Level;
  // modules to trace
  static unsigned long Module;

  static void set_tracing(const std::string &trace);
};


/// Macros that must be used to put traces in the code.
/// They may be either defined or null, depending on -DVERBOSE compilation flag. 
#define ERROR_CRASH(msg) { std::cerr<<MOD_TRACENAME<<": "<<msg<<std::endl; \
                           exit(1); \
                         }
 
/// Warning macros. Compile without -DNO_WARNINGS (default) to get a code that warns about suspicious things.
/// Compile with -DNO_WARNINGS to get non-warning, exploitation version.
#ifdef NO_WARNINGS
#define WARNING(msg)
#else
#define WARNING(msg)  { std::cerr<<MOD_TRACENAME<<": "<<msg<<std::endl; }
#endif

/// Tracing macros. Compile with -DVERBOSE to get a traceable code.
/// Compile without -DVERBOSE (default) to get faster, non-traceable, exploitation version.
#ifdef VERBOSE   
/// ifdef VERBOSE --> TRACE macros exists

#define TRACE(lv,msg) { if (traces::Level>=lv && (traces::Module&MOD_TRACECODE)) { \
                          std::cerr << MOD_TRACENAME << ": " << msg << std::endl;          \
                        }  \
                      }
                      
#else
/// ifndef VERBOSE --> No messages displayed. Faster code.
#define TRACE(x,y)
#endif

#endif
