/*
 val3dity - Copyright (c) 2011-2016, Hugo Ledoux.  All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the authors nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL HUGO LEDOUX BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
*/

#include "input.h"


bool IOErrors::has_errors()
{
  if (_errors.size() == 0)
    return false;
  else
    return true;
}


void IOErrors::add_error(int code, std::string info)
{
  _errors[code].push_back(info);
  std::clog << "--> errors #" << code << " : " << info << std::endl;
}


std::string IOErrors::get_report_text()
{
  std::stringstream ss;
  for (auto& err : _errors)
  {
    for (auto i : err.second)
    {
      ss << err.first << " -- " << errorcode2description(err.first) << std::endl;
      ss << "\tInfo: " << i << std::endl;
    }
  }
  return ss.str();
}


std::string IOErrors::get_report_xml()
{
  std::stringstream ss;
  for (auto& err : _errors)
  {
    for (auto i : err.second)
    {
      ss << "\t<Error>" << std::endl;
      ss << "\t\t<code>" << err.first << "</code>" << std::endl;
      ss << "\t\t<type>" << errorcode2description(err.first) << "</type>" << std::endl;
      ss << "\t\t<info>" << i << "</info>" << std::endl;
      ss << "\t</Error>" << std::endl;
    }
  }
  return ss.str();
}


std::string errorcode2description(int code) {
  switch(code)
  {
    case 0:   return string("STATUS_OK"); break;
    //-- RING
    case 101: return string("TOO_FEW_POINTS"); break;
    case 102: return string("CONSECUTIVE_POINTS_SAME"); break;
    case 103: return string("RING_NOT_CLOSED"); break;
    case 104: return string("RING_SELF_INTERSECTION"); break;
    case 105: return string("RING_COLLAPSED"); break;
    //-- POLYGON
    case 201: return string("INTERSECTION_RINGS"); break;
    case 202: return string("DUPLICATED_RINGS"); break;
    case 203: return string("NON_PLANAR_POLYGON_DISTANCE_PLANE"); break;
    case 204: return string("NON_PLANAR_POLYGON_NORMALS_DEVIATION"); break;
    case 205: return string("POLYGON_INTERIOR_DISCONNECTED"); break;
    case 206: return string("HOLE_OUTSIDE"); break;
    case 207: return string("INNER_RINGS_NESTED"); break;
    case 208: return string("ORIENTATION_RINGS_SAME"); break;
    //-- SHELL
    case 300: return string("NOT_VALID_2_MANIFOLD"); break;
    case 301: return string("TOO_FEW_POLYGONS"); break;
    case 302: return string("SHELL_NOT_CLOSED"); break;
    case 303: return string("NON_MANIFOLD_VERTEX"); break;
    case 304: return string("NON_MANIFOLD_EDGE"); break;
    case 305: return string("SEPARATE_PARTS"); break;
    case 306: return string("SHELL_SELF_INTERSECTION"); break;
    case 307: return string("POLYGON_WRONG_ORIENTATION"); break;
    case 308: return string("ALL_POLYGONS_WRONG_ORIENTATION"); break;
    case 309: return string("VERTICES_NOT_USED"); break;
    //--SOLID
    case 401: return string("SHELLS_FACE_ADJACENT"); break;
    case 402: return string("INTERSECTION_SHELLS"); break;
    case 403: return string("INNER_SHELL_OUTSIDE_OUTER"); break;
    case 404: return string("SOLID_INTERIOR_DISCONNECTED"); break;
    case 901: return string("INVALID_INPUT_FILE"); break;
    case 902: return string("EMPTY_PRIMITIVE"); break;
    case 999: return string("UNKNOWN_ERROR"); break;
    default:  return string("UNKNOWN_ERROR"); break;
  }
}

//-- ignore XML namespace
std::string localise(std::string s)
{
  return "*[local-name(.) = '" + s + "']";
}


vector<int> process_gml_ring(pugi::xml_node n, Surface* sur, IOErrors& errs) {
  std::string s = "./" + localise("LinearRing") + "/" + localise("pos");
  pugi::xpath_node_set npos = n.select_nodes(s.c_str());
  vector<int> r;
  if (npos.size() > 0) //-- <gml:pos> used
  {
    for (pugi::xpath_node_set::const_iterator it = npos.begin(); it != npos.end(); ++it) {
      std::string buf;
      std::stringstream ss(it->node().child_value());
      std::vector<std::string> tokens;
      while (ss >> buf)
        tokens.push_back(buf);
      Point3 p(std::stod(tokens[0]), std::stod(tokens[1]), std::stod(tokens[2]));
      r.push_back(sur->add_point(p));
    }
  }
  else //-- <gml:posList> used
  {
    std::string s = "./" + localise("LinearRing") + "/" + localise("posList");
    pugi::xpath_node pl = n.select_node(s.c_str());
    if (pl == NULL)
    {
      throw 901;
    }
    std::string buf;
    std::stringstream ss(pl.node().child_value());
    std::vector<std::string> coords;
    while (ss >> buf)
      coords.push_back(buf);
    if (coords.size() % 3 != 0)
    {
      errs.add_error(901, "Error: <gml:posList> has bad coordinates.");
      return r;
    }
    for (int i = 0; i < coords.size(); i += 3)
    {
      Point3 p(std::stod(coords[i]), std::stod(coords[i+1]), std::stod(coords[i+2]));
      r.push_back(sur->add_point(p));
    }
  }
  return r;
}


void process_GML_MultiSurface(pugi::xml_node n, Building& b, map<std::string, pugi::xpath_node>& dallpoly, double tol_snap, IOErrors& errs) 
{
  std::string s = ".//" + localise("surfaceMember");
  pugi::xpath_node_set nsm = n.select_nodes(s.c_str());
  int i = 0;
  for (pugi::xpath_node_set::const_iterator it = nsm.begin(); it != nsm.end(); ++it)
  {
    bool bxlink = false;
    pugi::xml_node tmpnode = it->node();
    pugi::xpath_node p;
    bool fliporientation = false;
    for (pugi::xml_attribute attr = tmpnode.first_attribute(); attr; attr = attr.next_attribute())
    {
      if (strcmp(attr.value(), "xlink:href") != 0) {
        bxlink = true;
        break;
      }
    }
    if (bxlink == true) 
    {
      std::string k = it->node().attribute("xlink:href").value();
      if (k[0] == '#')
        k = k.substr(1);
      p = dallpoly[k];
    }
    else
    {
      for (pugi::xml_node child : it->node().children()) 
      {
        if (std::string(child.name()).find("Polygon") != std::string::npos) {
          p = child;
          break;
        }
        else if (std::string(child.name()).find("OrientableSurface") != std::string::npos) {
          if (std::strncmp(child.attribute("orientation").value(), "-", 1) == 0)
            fliporientation = true;
          for (pugi::xml_node child2 : child.children()) 
          {
            if (std::string(child2.name()).find("baseSurface") != std::string::npos) 
            {
              std::string k = child2.attribute("xlink:href").value();
              if (k[0] == '#')
                k = k.substr(1);
              p = dallpoly[k];
              break;
            }
          }
          break;
        }
        else if (std::string(child.name()).find("CompositeSurface") != std::string::npos) 
          break;
        else {
          throw 901;
        }
      }
    }

    //-- this is to handle CompositeSurfaces part of MultiSurfaces
    if (p == NULL) 
      continue;
    
    if (std::strncmp(p.node().attribute("orientation").value(), "-", 1) == 0)
      fliporientation = true;
    Surface* sur = new Surface(n.name());
    sur->set_id(p.node().attribute("gml:id").value());
    //-- exterior ring (only 1)
    s = ".//" + localise("exterior");
    pugi::xpath_node ring = p.node().select_node(s.c_str());
    vector<int> r = process_gml_ring(ring.node(), sur, errs);
    if (fliporientation == true) 
      std::reverse(r.begin(), r.end());
    if (r.front() != r.back())
      sur->add_error(103, p.node().attribute("gml:id").value());
    else
      r.pop_back(); 
    sur->add_ring(r);
    //-- interior rings
    s = ".//" + localise("interior");
    pugi::xpath_node_set nint = it->node().select_nodes(s.c_str());
    for (pugi::xpath_node_set::const_iterator it = nint.begin(); it != nint.end(); ++it) {
      vector<int> r = process_gml_ring(it->node(), sur, errs);
      if (fliporientation == true) 
        std::reverse(r.begin(), r.end());
      if (r.front() != r.back())
        sur->add_error(103, p.node().attribute("gml:id").value());
      else
        r.pop_back(); 
      sur->add_ring(r);
    }
    b.add_surface(sur);
    i++;
  }
}


vector<Building> readGMLfile(std::string &ifile, IOErrors& errs)
{
  std::cout << "Reading file: " << ifile << std::endl;
  vector<Building> lsBuilding;
  pugi::xml_document doc;
  if (!doc.load_file(ifile.c_str())) 
  {
    errs.add_error(901, "Input file not found.");
    return lsBuilding;
  }
  std::string s = "//";
  s +=  localise("Building");
  pugi::xpath_query myquery(s.c_str());
  pugi::xpath_node_set nbuildings = myquery.evaluate_node_set(doc);
  std::cout << "Parsing the file..." << std::endl;
  std::cout << "# of Building found: ";
  std::cout << nbuildings.size() << std::endl;

  //-- build dico of xlinks
  //-- for <gml:Polygon>
  s = "//" + localise("Polygon") + "[@" + localise("id") + "]";
  pugi::xpath_node_set nallpoly = doc.select_nodes(s.c_str());
  if (nallpoly.size() > 0)
    std::cout << "XLinks found, resolving them..." << std::endl;
  map<std::string, pugi::xpath_node> dallpoly;
  for (pugi::xpath_node_set::const_iterator it = nallpoly.begin(); it != nallpoly.end(); ++it)
  {
    dallpoly[it->node().attribute("gml:id").value()] = *it;
  }

  //-- for <gml:OrientableSurface>
  s = "//" + localise("OrientableSurface") + "[@" + localise("id") + "]";
  pugi::xpath_node_set nallosurf = doc.select_nodes(s.c_str());
  // map<std::string, pugi::xpath_node> dallpoly;
  for (pugi::xpath_node_set::const_iterator it = nallosurf.begin(); it != nallosurf.end(); ++it)
  {
    dallpoly[it->node().attribute("gml:id").value()] = *it;
  }

  //-- checking xlinks validity now not to be bitten later
  s = "//" + localise("surfaceMember") + "[@" + localise("href") + "]";
  pugi::xpath_node_set nsmxlink = doc.select_nodes(s.c_str());
  for (pugi::xpath_node_set::const_iterator it = nsmxlink.begin(); it != nsmxlink.end(); ++it) {
    std::string k = it->node().attribute("xlink:href").value();
    if (k[0] == '#')
      k = k.substr(1);
    if (dallpoly.count(k) == 0) {
      std::string r = "One XLink couldn't be resolved (";
      r += it->node().attribute("xlink:href").value();
      r += ")";
      errs.add_error(901, r);
      return lsBuilding;
    }
  }
  
  for(auto& nbuilding: nbuildings)
  {
    Building b;
    if (nbuilding.node().attribute("gml:id") != 0)
      b.set_id(std::string(nbuilding.node().attribute("gml:id").value()));
    std::string s = "./" + localise("boundedBy");
    pugi::xpath_node_set nbounds = nbuilding.node().select_nodes(s.c_str());
    std::cout << nbounds.size() << std::endl;
    
    for (pugi::xpath_node_set::const_iterator it = nbounds.begin(); it != nbounds.end(); ++it) {
      for (pugi::xml_node child : it->node().children()) {
        std::cout << child.name() << std::endl;
        process_GML_MultiSurface(child, b, dallpoly, 0.001, errs);
      }
    }
    lsBuilding.push_back(b);
  }
  std::cout << "Input file correctly parsed without errors." << std::endl;
  return lsBuilding;
}



void printProgressBar(int percent) {
  std::string bar;
  for(int i = 0; i < 50; i++){
    if( i < (percent / 2)) {
      bar.replace(i, 1, "=");
    }
    else if( i == (percent / 2)) {
      bar.replace(i, 1, ">");
    }
    else{
      bar.replace(i, 1, " ");
    }
  }
  std::cout << "\r" "[" << bar << "] ";
  std::cout.width(3);
  std::cout << percent << "%     " << std::flush;
}


