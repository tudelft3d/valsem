//
//  Surface.hpp
//  val3dity
//
//  Created by Hugo Ledoux on 16/05/16.
//
//

#ifndef Surface_hpp
#define Surface_hpp

#include "definitions.h"


class Surface
{
public:
  Surface  (std::string sem, std::string id = "");
  
  int         number_vertices();
  bool        is_empty();
  std::string get_id();
  void        set_id(std::string id);
  std::string get_semantics();
  
  int         add_point(Point3 p);
  void        add_ring(vector<int> r);
  int         validate(double &anglenormal);

  std::string   get_report_xml();
  static int    _counter;
  
private:
  vector<Point3>                  _lsPts;
  vector< vector<int> >           _lsRings;
  std::string                     _id;
  std::string                     _sem;


  // bool validate_polygon(vector<Polygon> &lsRings, std::string polygonid);
  // bool has_face_rings_toofewpoints(const vector< vector<int> >& theface);
  // bool has_face_2_consecutive_repeated_pts(const vector< vector<int> >& theface);

};

#endif /* Surface_hpp */
