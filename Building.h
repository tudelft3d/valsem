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
  std::string   validate(int &total, int &valid);
  std::string   get_id();
  void          set_id(std::string id);
  static int    _counter;
private:
  std::string            _id;
  int                    _is_valid;
  vector<Surface*>       _lsSurfaces;
};


#endif /* Building_hpp */
