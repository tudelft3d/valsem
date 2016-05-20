

#ifndef __val3dity__geomtools__
#define __val3dity__geomtools__

#include "definitions.h"

//-- misc
#define PI 3.14159265

bool    cmpPoint3(Point3 &p1, Point3 &p2, double tol);
int     projection_plane(const vector< Point3 > &lsPts, const vector<int> &ids);
bool    create_polygon(const vector<Point3>& lsPts, const vector<int>& ids, Polygon &pgn);
bool    is_face_planar_distance2plane(const vector<Point3> &pts, double& value, float tolerance);
bool    is_face_planar_normals(const vector<int*> &trs, const vector<Point3>& lsPts, double& value, float angleTolerance);
bool    polygon_normal(const vector< Point3 > &lsPts, const vector<int> &ids, Vector &n);

#endif /* defined(__val3dity__geomtools__) */
