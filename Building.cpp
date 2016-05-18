//
//  Building.cpp
//  val3dity
//
//  Created by Hugo Ledoux on 16/05/16.
//
//

#include "input.h"
#include "Building.h"
#include "Surface.h"


int Building::_counter = 0;

Building::Building(std::string id) {
  if (id.empty() == true)
    _id = std::to_string(_counter);
  else
    _id = id;
  _counter++;
  _is_valid = -1;
}


void Building::add_surface(Surface* s) {
  _lsSurfaces.push_back(s);
}

int Building::num_faces() {
  return _lsSurfaces.size();
}


std::string Building::validate(int &total, int &valid) {
  double anglenormal;
  total = this->num_faces();
  int totalvalid = 0;
  std::stringstream ss;
  ss << "\t<building ID=\"" << this->_id << "\">" << std::endl;
  ss << "\t\t<numbersurfaces>" << this->num_faces() << "</numbersurfaces>" << std::endl;
  // ss << "\t\t<numbervalidsurfaces>" << this->num_faces() << "</numbervalidsurfaces>" << std::endl;
  for (auto& s : _lsSurfaces) {
    ss << "\t\t<Surface ID=\"" << s->get_id() << "\" type=\"" << s->get_semantics() << "\">" << std::endl;
    int re = s->validate(anglenormal);
    if (re == 1) {
      totalvalid += 1;
      ss << "\t\t\t<valdity>True</valdity>" << std::endl; 
    }
    else if (re == -1)
      ss << "\t\t\t<valdity>False</valdity>" << std::endl; 
    else if (re == 0)
      ss << "\t\t\t<valdity>Unknown</valdity>" << std::endl; 
    ss << "\t\t\t<orientation>" << anglenormal << "</orientation>" << std::endl; 
    ss << "\t\t</Surface>" << std::endl;
  }
  ss << "\t</building>" << std::endl;
  valid = totalvalid;
  return ss.str();
}


std::string Building::get_id() {
  return _id;
}

void Building::set_id(std::string id) {
  _id = id;
}
