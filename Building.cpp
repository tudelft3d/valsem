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


bool Building::validate(double tol_planarity_d2p, double tol_planarity_normals) {
  for (auto& s : _lsSurfaces) {
    std::cout << s->get_id() << std::endl;
    std::cout << s->get_semantics() << std::endl;
    s->validate();
  }

  return true;
}

std::string Building::get_report_xml() {
  return "<report>";
}


std::string Building::get_id() {
  return _id;
}

void Building::set_id(std::string id) {
  _id = id;
}
