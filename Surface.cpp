//
//  Surface.cpp
//  val3dity
//
//  Created by Hugo Ledoux on 16/05/16.
//
//

#include "Surface.h"
#include "input.h"
#include "geomtools.h"
#include <sstream>

#include "CGAL/squared_distance_3.h"
#include <CGAL/linear_least_squares_fitting_3.h>


int Surface::_counter = 0;


Surface::Surface(std::string sem, std::string id) {
  if (id.empty() == true)
    _id = std::to_string(_counter);
  else
    _id = id;

  std::size_t found = sem.find_first_of(":");
  if (found != std::string::npos) {
    _sem = sem.substr(found + 1);
  }
  else {
    _sem = sem;
  }
  _counter++;
}


std::string Surface::get_id()
{
  return _id;
}

void Surface::set_id(std::string id) {
  _id = id;
}

std::string Surface::get_semantics()
{
  return _sem;
}



bool Surface::is_empty()
{
  return _lsPts.empty();
}


void Surface::add_ring(vector<int> r) {
  _lsRings.push_back(r);
}


int Surface::add_point(Point3 p)
{
  int pos = -1;
  int cur = 0;
  for (auto &itr : _lsPts)
  {
    // std::clog << "---" << itr << std::endl;
    if (cmpPoint3(p, itr, 0.001) == true)
    {
      pos = cur;
      break;
    }
    cur++;
  }
  if (pos == -1)
  {
    _lsPts.push_back(p);
    return (_lsPts.size() - 1);
  }
  else
    return pos;
}


int Surface::number_vertices()
{
  return _lsPts.size();
}


int Surface::validate(double &anglenormal)
{
    vector< Point3 > uniquepts;
    vector<int>::const_iterator itp = _lsRings[0].begin();
    for ( ; itp != (_lsRings[0]).end(); itp++)
    {
      uniquepts.push_back(_lsPts[*itp]);
    }
    //-- irings
    for (int j = 1; j < static_cast<int>(_lsRings.size()); j++)
    {
      vector<int> &ids2 = _lsRings[j]; // helpful alias for the inner boundary
      vector<int>::const_iterator itp2 = ids2.begin();
      for ( ; itp2 != ids2.end(); itp2++)
      {
        uniquepts.push_back(_lsPts[*itp2]);
      }
    }

    if (uniquepts.size() < 3) {
      anglenormal = -9999;
      return 0;
    }

    K::Plane_3 plane;
    linear_least_squares_fitting_3(uniquepts.begin(), uniquepts.end(), plane, CGAL::Dimension_tag<0>());
    Vector n = plane.orthogonal_vector();
    Vector up(0, 0, 1);
    double angle = std::acos(n * up) * 180 / PI;
    int re = 1;
    if ( (_sem == "RoofSurface") || (_sem == "OuterFloorSurface") ) {
      if ( angle > 85 )
        re = -1;
    }
    else if (_sem == "WallSurface") {
      if ( ( angle < 85) || (angle > 95) )
        re = -1;
    }
    else if ( (_sem == "GroundSurface") || (_sem == "OuterCeilingSurface") ) {
      Polygon pgn;
      vector<int>::const_iterator itp = _lsRings[0].begin();
      for ( ; itp != (_lsRings[0]).end(); itp++)
      {
        Point3 p = _lsPts[*itp];
        pgn.push_back(Point2(p.x(), p.y()));
      }
      if (pgn.orientation() == CGAL::CLOCKWISE) {
        n = plane.opposite().orthogonal_vector();
        angle = std::acos(n * up) * 180 / PI;
      }
      if (angle < 175)
        re = -1;
    }
    anglenormal = angle;
    return re;
}

