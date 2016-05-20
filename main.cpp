

#include "input.h"
#include "Surface.h"
#include "Building.h"
#include <tclap/CmdLine.h>
#include <time.h>  



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

    //-- now the validation starts
    std::stringstream ss;
    ss << "<semantic>" << std::endl;
    int totalsurfaces = 0;
    int totalvalidsurfaces = 0;
    if ( (lsBuilding.empty() == false) && (ioerrs.has_errors() == false) )
    {
      std::cout << "Validating " << lsBuilding.size() << " Buildings.";
      std::cout << std::endl;
      int i = 1;
      for (auto& b : lsBuilding)
      {
        if ( (i % 10 == 0) && (verbose.getValue() == false) )
          printProgressBar(100 * (i / double(lsBuilding.size())));
        i++;
        int total;
        int valid;
        ss << b.validate(total, valid);
        totalsurfaces += total;
        totalvalidsurfaces += valid;
      }
      printProgressBar(100);
    }
    ss << "</semantic>" << std::endl;
    std::cout << std::endl;

    //-- SUMMARY
    std::stringstream ss2;
    ss2 << std::endl;
    ss2 << "+++++++++++++++++++ SUMMARY +++++++++++++++++++" << std::endl;
    ss2 << "total # of Buildings: " << setw(12) << lsBuilding.size() << std::endl;
    ss2 << "total # of Surfaces: " << setw(13) << totalsurfaces << std::endl;
    ss2 << "# valid: " << setw(25) << totalvalidsurfaces << std::endl;
    ss2 << "# invalid: " << setw(23) << (totalsurfaces - totalvalidsurfaces) << std::endl;
    ss2 << "+++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
    std::cout << ss2.str() << std::endl;


    if (outputxml.getValue() != "")
    {
      std::ofstream thereport;
      thereport.open(outputxml.getValue());
      thereport << ss.str();
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

