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

  void          add_surface(Surface* s);
  int           num_faces();
  
  bool          validate(double tol_planarity_d2p = 0, double tol_planarity_normals = 0);
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
