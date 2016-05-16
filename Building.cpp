//
//  Building.cpp
//  val3dity
//
//  Created by Hugo Ledoux on 16/05/16.
//
//

#include "input.h"
#include "Building.h"


int Building::_counter = 0;

Building::Building(std::string id) {
  if (id.empty() == true)
    _id = std::to_string(_counter);
  else
    _id = id;
  _counter++;
  _is_valid = -1;
}


int Building::num_faces() {
  return 0;
}

int Building::num_vertices() {
  return 0;
}

bool Building::validate(double tol_planarity_d2p, double tol_planarity_normals) {
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
