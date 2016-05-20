#ifndef VAL3DITY_INPUT_DEFINITIONS_H
#define VAL3DITY_INPUT_DEFINITIONS_H



#include "Building.h"
#include "Surface.h"
#include <fstream>
#include <string>
#include "pugixml.hpp"



class IOErrors {
  std::map<int, vector<std::string> >  _errors;
public:
  bool has_errors();
  void add_error(int code, std::string info);
  std::string get_report_text();
  std::string get_report_xml();
};

std::string      errorcode2description(int code);
vector<Building> readGMLfile(std::string &ifile, IOErrors& errs);
void             process_GML_MultiSurface(pugi::xml_node n, Building& b, map<std::string, pugi::xpath_node>& dallpoly, double tol_snap, IOErrors& errs);
vector<int>      process_gml_ring(pugi::xml_node n, Surface* s, IOErrors& errs);

void          printProgressBar(int percent);
std::string   localise(std::string s);
#endif
