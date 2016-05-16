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
  _sem = sem;
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


bool Surface::has_errors()
{
return !(_errors.empty());
}

bool Surface::is_empty()
{
  return _lsPts.empty();
}


void Surface::add_error(int code, std::string faceid, std::string info)
{
  std::tuple<std::string, std::string> a(faceid, info);
  _errors[code].push_back(a);
  std::clog << "\tERROR " << code << ": " << errorcode2description(code);
  if (faceid.empty() == false)
    std::clog << " (face " << faceid << ")";
  std::clog << std::endl;
  if (info.empty() == false)
    std::clog << "\t[" << info << "]" << std::endl;
}

std::set<int> Surface::get_unique_error_codes()
{
  std::set<int> errs;
  for (auto& err : _errors)
  {
    errs.insert(std::get<0>(err));
  }
  return errs;
}


std::string Surface::get_report_xml()
{
  std::stringstream ss;
  for (auto& err : _errors)
  {
    for (auto& e : _errors[std::get<0>(err)])
    {
      ss << "\t\t<Error>" << std::endl;
      ss << "\t\t\t<code>" << std::get<0>(err) << "</code>" << std::endl;
      ss << "\t\t\t<type>" << errorcode2description(std::get<0>(err)) << "</type>" << std::endl;
      ss << "\t\t\t<shell>" << this->_id << "</shell>" << std::endl;
      ss << "\t\t\t<face>" << std::get<0>(e) << "</face>" << std::endl;
      ss << "\t\t\t<info>" << std::get<1>(e) << "</info>" << std::endl;
      ss << "\t\t</Error>" << std::endl;
    }
  }
  return ss.str();
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


bool Surface::validate(double tol_planarity_d2p, double tol_planarity_normals)
{
    vector< Point3 > allpts;
    vector<int>::const_iterator itp = _lsRings[0].begin();
    for ( ; itp != (_lsRings[0]).end(); itp++)
    {
      allpts.push_back(_lsPts[*itp]);
    }
    //-- irings
    for (int j = 1; j < static_cast<int>(_lsRings.size()); j++)
    {
      vector<int> &ids2 = _lsRings[j]; // helpful alias for the inner boundary
      vector<int>::const_iterator itp2 = ids2.begin();
      for ( ; itp2 != ids2.end(); itp2++)
      {
        allpts.push_back(_lsPts[*itp2]);
      }
    }

    K::Plane_3 plane;
    linear_least_squares_fitting_3(allpts.begin(), allpts.end(), plane, CGAL::Dimension_tag<0>());
    std::cout << plane << std::endl;

    return true;
}
//   std::clog << "--2D validation of each surface" << std::endl;
//   bool isValid = true;
//   size_t num = _lsFaces.size();
//   for (int i = 0; i < static_cast<int>(num); i++)
//   {
//     //-- test for too few points (<3 for a ring)
//     if (has_face_rings_toofewpoints(_lsFaces[i]) == true)
//     {
//       this->add_error(101, _lsFacesID[i]);
//       isValid = false;
//       continue;
//     }
//     //-- test for 2 repeated consecutive points
//     if (has_face_2_consecutive_repeated_pts(_lsFaces[i]) == true)
//     {
//       this->add_error(102, _lsFacesID[i]);
//       isValid = false;
//       continue;
//     }
//     size_t numf = _lsFaces[i].size();
//     vector<int> &ids = _lsFaces[i][0]; // helpful alias for the outer boundary

//     //-- if only 3 pts it's now valid, no need to process further
//     if ( (numf == 1) && (ids.size() == 3)) 
//       continue;

//     vector< Point3 > allpts;
//     vector<int>::const_iterator itp = ids.begin();
//     for ( ; itp != ids.end(); itp++)
//     {
//       allpts.push_back(_lsPts[*itp]);
//     }
//     //-- irings
//     for (int j = 1; j < static_cast<int>(numf); j++)
//     {
//       vector<int> &ids2 = _lsFaces[i][j]; // helpful alias for the inner boundary
//       vector<int>::const_iterator itp2 = ids2.begin();
//       for ( ; itp2 != ids2.end(); itp2++)
//       {
//         allpts.push_back(_lsPts[*itp2]);
//       }
//     }
//     double value;
//     if (false == is_face_planar_distance2plane(allpts, value, tol_planarity_d2p))
//     {
//       std::stringstream msg;
//       msg << "distance to fitted plane: " << value << " (tolerance=" << tol_planarity_d2p << ")";
//       this->add_error(203, _lsFacesID[i], msg.str());
//       isValid = false;
//       continue;
//     }
//     //-- get projected oring
//     Polygon pgn;
//     vector<Polygon> lsRings;
//     if (false == create_polygon(_lsPts, ids, pgn))
//     {
//       this->add_error(104, _lsFacesID[i], " outer ring self-intersects or is collapsed to a point or a line");
//       isValid = false;
//       continue;
//     }
//     lsRings.push_back(pgn);
//     //-- check for irings
//     for (int j = 1; j < static_cast<int>(numf); j++)
//     {
//       vector<int> &ids2 = _lsFaces[i][j]; // helpful alias for the inner boundary
//       //-- get projected iring
//       Polygon pgn;
//       if (false == create_polygon(_lsPts, ids2, pgn))
//       {
//         this->add_error(104, _lsFacesID[i], "Inner ring self-intersects or is collapsed to a point or a line");
//         isValid = false;
//         continue;
//       }
//       lsRings.push_back(pgn);
//     }
//     //-- use GEOS to validate projected polygon
//     if (!validate_polygon(lsRings, _lsFacesID[i]))
//       isValid = false;
//   }
//   if (isValid)
//   {
//     //-- triangulate faces of the shell
//     triangulate_shell();
//     //-- check planarity by normal deviation method (of all triangle)
//     std::clog << "--Planarity of surfaces (with normals deviation)" << std::endl;
//     vector< vector<int*> >::iterator it = _lsTr.begin();
//     int j = 0;
//     double deviation;
//     for ( ; it != _lsTr.end(); it++)
//     { 
//       if (is_face_planar_normals(*it, _lsPts, deviation, tol_planarity_normals) == false)
//       {
//         std::ostringstream msg;
//         msg << "deviation normals: " << deviation << " (tolerance=" << tol_planarity_normals << ")";
//         this->add_error(204, _lsFacesID[j], msg.str());
//         isValid = false;
//       }
//       j++;
//     }
//   }
//   _is_valid_2d = isValid;
//   return isValid;
// }
