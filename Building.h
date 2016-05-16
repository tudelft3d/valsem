//
//  Building.hpp
//  val3dity
//
//  Created by Hugo Ledoux on 16/05/16.
//
//

#ifndef Building_hpp
#define Building_hpp

#include "Surface.h"
#include "definitions.h"


class Building
{
public:
  Building(std::string id = "");
  int           num_faces();
  int           num_vertices();
  
  bool          validate(double tol_planarity_d2p, double tol_planarity_normals);
  std::string   get_report_xml();
  
  std::string   get_id();
  void          set_id(std::string id);
  static int    _counter;
private:
  std::string            _id;
  int                    _is_valid;
  vector<Surface*>       _lsSurfaces;
  
};


#endif /* Building_hpp */
