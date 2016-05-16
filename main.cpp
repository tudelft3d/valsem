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
#include "Surface.h"
#include "Building.h"
#include <tclap/CmdLine.h>
#include <time.h>  


std::string print_summary_validation(vector<Building>& lsSolids);
void write_report_xml (std::ofstream& ss, std::string ifile, 
                      double snap_tolerance, double planarity_d2p, double planarity_n, 
                      vector<Building>& lsBuilding, IOErrors ioerrs);

class MyOutput : public TCLAP::StdOutput
{
public:
  
  virtual void usage(TCLAP::CmdLineInterface& c)
  {
    std::cout << "===== val3dity =====" << std::endl;
    std::cout << "OPTIONS" << std::endl;
    std::list<TCLAP::Arg*> args = c.getArgList();
    for (TCLAP::ArgListIterator it = args.begin(); it != args.end(); it++) {
      if ((*it)->getFlag() == "")
        std::cout << "\t--" << (*it)->getName() << std::endl;
      else
        std::cout << "\t-" << (*it)->getFlag() << ", --" << (*it)->getName() << std::endl;
      std::cout << "\t\t" << (*it)->getDescription() << std::endl;
    }
    std::cout << "EXAMPLES" << std::endl;
    std::cout << "\tval3dity input.gml" << std::endl;
    std::cout << "\t\tValidates each gml:Solid in input.gml and outputs a summary" << std::endl;
    std::cout << "\tval3dity input.obj" << std::endl;
    std::cout << "\t\tValidates each object in the OBJ file and outputs a summary" << std::endl;
    std::cout << "\tval3dity input.gml -p MS" << std::endl;
    std::cout << "\t\tValidates each gml:MultiSurface in input.gml and outputs a summary" << std::endl;
    std::cout << "\tval3dity input.gml --oxml report.xml" << std::endl;
    std::cout << "\t\tValidates each gml:Solid in input.gml and outputs a detailed report in XML" << std::endl;
    std::cout << "\tval3dity data/poly/cube.poly --ishell data/poly/py.poly" << std::endl;
    std::cout << "\t\tValidates the solid formed by the outer shell cube.poly with the inner shell py.poly" << std::endl;
    std::cout << "\tval3dity input.gml --verbose" << std::endl;
    std::cout << "\t\tAll details of the validation of the solids is printed out" << std::endl;
    std::cout << "\tval3dity input.gml --snap_tolerance 0.1" << std::endl;
    std::cout << "\t\tThe vertices in gml:Solid closer than 0.1unit are snapped together" << std::endl;
    std::cout << "\tval3dity input.gml --planarity_d2p 0.1" << std::endl;
    std::cout << "\t\tValidates each gml:Solid in input.gml" << std::endl;
    std::cout << "\t\tand uses a tolerance of 0.1unit (distance point-to-fitted-plane)" << std::endl;
  }
};


int main(int argc, char* const argv[])
{
#ifdef VAL3DITY_USE_EPECSQRT
  std::clog << "***** USING EXACT-EXACT *****" << std::endl;
#endif

  IOErrors ioerrs;
  std::streambuf* savedBufferCLOG;
  std::ofstream mylog;

  //-- tclap options
  TCLAP::CmdLine cmd("Allowed options", ' ', "1.0");
  MyOutput my;
  cmd.setOutput(&my);
  try {
    TCLAP::UnlabeledValueArg<std::string>  inputfile("inputfile", "input file in either CityGML", true, "", "string");
    TCLAP::ValueArg<std::string>           outputxml("", "oxml", "output report in XML format", false, "", "string");
    TCLAP::SwitchArg                       verbose("", "verbose", "verbose output", false);
    TCLAP::ValueArg<double>                snap_tolerance("", "snap_tolerance", "tolerance for snapping vertices in GML (default=0.001)", false, 0.001, "double");
    TCLAP::ValueArg<double>                planarity_d2p("", "planarity_d2p", "tolerance for planarity distance_to_plane (default=0.01)", false, 0.01, "double");
    TCLAP::ValueArg<double>                planarity_n("", "planarity_n", "tolerance for planarity based on normals deviation (default=1.0degree)", false, 1.0, "double");

    cmd.add(planarity_d2p);
    cmd.add(planarity_n);
    cmd.add(snap_tolerance);
    cmd.add(verbose);
    cmd.add(inputfile);
    cmd.add(outputxml);
    cmd.parse( argc, argv );
  
    //-- if verbose == false then log to a file
    if (verbose.getValue() == false)
    {
      savedBufferCLOG = clog.rdbuf();
      mylog.open("val3dity.log");
      std::clog.rdbuf(mylog.rdbuf());
    }

    vector<Building> lsBuilding;

    std::string extension = inputfile.getValue().substr(inputfile.getValue().find_last_of(".") + 1);
    if ( (extension == "gml") ||  
         (extension == "GML") ||  
         (extension == "xml") ||  
         (extension == "XML") ) 
    {
      try
      {
        lsBuilding = readGMLfile(inputfile.getValue(), ioerrs);
        if (ioerrs.has_errors() == true) {
          std::cout << "Errors while reading the input file, aborting." << std::endl;
          std::cout << ioerrs.get_report_text() << std::endl;
        }
      }
      catch (int e)
      {
        if (e == 901)
          ioerrs.add_error(901, "Invalid GML structure, or that particular (twisted and obscure) construction of GML is not supported. Please report at https://github.com/tudelft3d/val3dity/issues");
      }
    }
    else
    {
      std::cout << "Unknown file type (only GML/XML accepted)" << std::endl;
      ioerrs.add_error(901, "Unknown file type (only GML/XML accepted)");
    }

    std::cout << "==========" << std::endl;

    Building b = lsBuilding[0];
    b.validate();


    // //-- now the validation starts
    // if ( (lsBuilding.empty() == false) && (ioerrs.has_errors() == false) )
    // {
    //   std::cout << "Validating " << lsBuilding.size() << " Buildings.";
    //   std::cout << std::endl;
    //   int i = 1;
    //   for (auto& b : lsBuilding)
    //   {
    //     if ( (i % 10 == 0) && (verbose.getValue() == false) )
    //       printProgressBar(100 * (i / double(lsBuilding.size())));
    //     i++;
    //     std::clog << std::endl << "===== Validating Building #" << b.get_id() << " =====" << std::endl;
    //     // std::clog << "Number shells: " << (b.num_ishells() + 1) << std::endl;
    //     // std::clog << "Number faces: " << b.num_faces() << std::endl;
    //     std::clog << "Number vertices: " << b.num_vertices() << std::endl;
    //     if (b.validate(planarity_d2p.getValue(), planarity_n.getValue()) == false)
    //       std::clog << "===== INVALID =====" << std::endl;
    //     else
    //       std::clog << "===== VALID =====" << std::endl;
    //   }
    //   if (verbose.getValue() == false)
    //     printProgressBar(100);
    // }

    //-- print summary of errors
//    std::cout << "\n" << print_summary_validation(lsBuilding, prim3d) << std::endl;        
   
    if (outputxml.getValue() != "")
    {
      std::ofstream thereport;
      thereport.open(outputxml.getValue());
      write_report_xml(thereport, 
                       inputfile.getValue(),
                       snap_tolerance.getValue(),
                       planarity_d2p.getValue(),
                       planarity_n.getValue(),
                       lsBuilding,
                       ioerrs);
      thereport.close();
      std::cout << "Full validation report saved to " << outputxml.getValue() << std::endl;
    }
  
    if ( (outputxml.getValue() == "") ) 
      std::cout << "-->The validation report wasn't saved, use option '--oxml' or '--otxt'." << std::endl;

    if (verbose.getValue() == false)
    {
      clog.rdbuf(savedBufferCLOG);
      mylog.close();
    }
    return(1);
  }
  catch (TCLAP::ArgException &e) {
    std::cout << "ERROR: " << e.error() << " for arg " << e.argId() << std::endl;
    return(0);
  }
}


std::string print_summary_validation(vector<Building>& lsBuilding)
{
  return "SUMMARY";
  // std::stringstream ss;
  // ss << std::endl;
  // std::string primitives;
  // if (prim3d == SOLID)
  //   primitives = "<gml:Solid>";
  // else if (prim3d == COMPOSITESURFACE)
  //   primitives = "<gml:CompositeSurface>";
  // else 
  //   primitives = "<gml:MultiSurface>";
  // ss << "+++++++++++++++++++ SUMMARY +++++++++++++++++++" << std::endl;
  // ss << "Primitives validated: " << primitives << std::endl;
  // ss << "total # of primitives: " << setw(8) << lsBuilding.size() << std::endl;
  // int bValid = 0;
  // for (auto& s : lsBuilding)
  //   if (s.is_valid() == true)
  //     bValid++;
  // int percentage;
  // if (lsBuilding.size() == 0)
  //   percentage = 0;
  // else
  //   percentage = 100 * ((lsBuilding.size() - bValid) / float(lsBuilding.size()));
  // ss << "# valid: " << setw(22) << bValid;
  // if (lsBuilding.size() == 0)
  //   ss << " (" << 0 << "%)" << std::endl;
  // else
  //   ss << " (" << 100 - percentage << "%)" << std::endl;
  // ss << "# invalid: " << setw(20) << (lsBuilding.size() - bValid);
  // ss << " (" << percentage << "%)" << std::endl;
  // std::map<int,int> errors;
  // for (auto& s : lsBuilding)
  // {
  //   for (auto& code : s.get_unique_error_codes())
  //     errors[code] = 0;
  // }
  // for (auto& s : lsBuilding)
  // {
  //   for (auto& code : s.get_unique_error_codes())
  //     errors[code] += 1;
  // }
  // if (errors.size() > 0)
  // {
  //   ss << "Errors present:" << std::endl;
  //   for (auto e : errors)
  //   {
  //     ss << "  " << e.first << " --- " << errorcode2description(e.first) << std::endl;
  //     ss << setw(11) << "(" << e.second << " primitives)" << std::endl;
  //   }
  // }
  // ss << "+++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
  // return ss.str();
}


void write_report_xml(std::ofstream& ss,
                      std::string ifile, 
                      double snap_tolerance,
                      double planarity_d2p,
                      double planarity_n,
                      vector<Building>& lsBuilding,
                      IOErrors ioerrs)
{

  // ss << "<val3dity>" << std::endl;
  // ss << "\t<inputFile>" << ifile << "</inputFile>" << std::endl;
  // ss << "\t<primitives>";
  // if (prim3d == SOLID)
  //   ss << "gml:Solid";
  // else if (prim3d == COMPOSITESURFACE)
  //   ss << "gml:CompositeSurface";
  // else
  //   ss << "gml:MultiSurface";
  // ss << "</primitives>" << std::endl;
  // ss << "\t<snap_tolerance>" << snap_tolerance << "</snap_tolerance>" << std::endl;
  // ss << "\t<planarity_d2p>" << planarity_d2p << "</planarity_d2p>" << std::endl;
  // ss << "\t<planarity_n>" << planarity_n << "</planarity_n>" << std::endl;
  // ss << "\t<totalprimitives>" << lsSolids.size() << "</totalprimitives>" << std::endl;
  // int bValid = 0;
  // for (auto& s : lsSolids)
  //   if (s.is_valid() == true)
  //     bValid++;
  // ss << "\t<validprimitives>" << bValid << "</validprimitives>" << std::endl;
  // ss << "\t<invalidprimitives>" << (lsSolids.size() - bValid) << "</invalidprimitives>" << std::endl;
  // std::time_t rawtime;
  // struct tm * timeinfo;
  // std::time (&rawtime);
  // timeinfo = std::localtime ( &rawtime );
  // char buffer[80];
  // std::strftime(buffer, 80, "%c %Z", timeinfo);
  // ss << "\t<time>" << buffer << "</time>" << std::endl;
  // if (ioerrs.has_errors() == true)
  // {
  //   ss << ioerrs.get_report_xml();
  // }
  // else
  // {
  //   for (auto& s : lsSolids) 
  //   {
  //     if ( !((onlyinvalid == true) && (s.is_valid() == true)) )
  //       ss << s.get_report_xml();
  //   }
  // }
  // ss << "</val3dity>" << std::endl;
}