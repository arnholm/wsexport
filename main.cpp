#include <iostream>
#include <fstream>
using namespace std;

#include <wx/defs.h>
#include <wx/platinfo.h>  // platform info
#include <wx/app.h>

#include <wx/string.h>   // wxString
#include <wx/filefn.h>   // File Functions
#include <wx/filename.h> // wxFileName
#include <wx/cmdline.h>  // command line parser

#include <boost/lexical_cast.hpp>

#include <map>
using namespace std;
typedef map<wxString,wxString> CmdLineMap;    // CmdLineMap


#include "op_lite/op_manager.h"
#include "op_lite/op_database.h"
#include "op_lite/op_class_name.h"
#include "op_lite/op_transaction.h"
#include "op_lite/op_ptr.h"

#include "sqlWeatherStation.h"
#include "sqlTextExporter.h"

/*
   switch 	  This is a boolean option which can be given or not, but which doesn't have any value.
              We use the word switch to distinguish such boolean options from more generic options
              like those described below. For example, -v might be a switch meaning "enable verbose mode".

   option 	  Option for us here is something which comes with a value unlike a switch.
              For example, -o:filename might be an option which allows to specify the name of the output file.

   parameter  This is a required program argument.

   More info at: http://docs.wxwidgets.org/2.8/wx_wxcmdlineparser.html#wxcmdlineparser

*/

static const wxCmdLineEntryDesc cmdLineDesc[] =
{
  //   kind            shortName          longName            description                                            parameterType          flag(s)
  { wxCMD_LINE_PARAM,  wxT_2("database"),  wxT_2("database"), wxT_2("   <database_path>"),                           wxCMD_LINE_VAL_STRING, wxCMD_LINE_OPTION_MANDATORY  },
  { wxCMD_LINE_OPTION, wxT_2("xd"),       wxT_2("xdays"),     wxT_2("Export <num> days to standard output"),         wxCMD_LINE_VAL_STRING, wxCMD_LINE_OPTION_MANDATORY  },
  { wxCMD_LINE_OPTION, wxT_2("xe"),       wxT_2("xelev"),     wxT_2("Elevation above sea level, <str>=[m]"),         wxCMD_LINE_VAL_STRING, wxCMD_LINE_OPTION_MANDATORY  },
  { wxCMD_LINE_OPTION, wxT_2("xr"),       wxT_2("xrdat"),     wxT_2("Rain level datum, <str>=[mm]"),                 wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL    },
  { wxCMD_LINE_OPTION, wxT_2("xh"),       wxT_2("xhtml"),     wxT_2("latest values to html file, <str>=[filename]"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL    },
  { wxCMD_LINE_SWITCH, wxT_2("xu"),       wxT_2("xutc") ,     wxT_2("use UTC time instead of local time"),           wxCMD_LINE_VAL_NONE,   wxCMD_LINE_PARAM_OPTIONAL    },
  { wxCMD_LINE_NONE,   wxT_2(""),         wxT_2(""),          wxT_2(""),                                             wxCMD_LINE_VAL_NONE,   wxCMD_LINE_PARAM_OPTIONAL  }
};


void ParserToMap(wxCmdLineParser& parser, CmdLineMap& cmdMap)
{
   size_t pcount = sizeof(cmdLineDesc)/sizeof(wxCmdLineEntryDesc) - 1;
   for(size_t i=0; i<pcount; i++) {
      wxString pname = cmdLineDesc[i].longName;
      if(cmdLineDesc[i].kind == wxCMD_LINE_PARAM) {
         cmdMap.insert(make_pair(pname,parser.GetParam(0)));
      }
      else {
         // switch or option, must check if present
         if(parser.Found(pname)) {
            wxString pvalue;
            if(cmdLineDesc[i].type == wxCMD_LINE_VAL_STRING) {
               parser.Found(pname,&pvalue);
            }
            else if(cmdLineDesc[i].type == wxCMD_LINE_VAL_NUMBER) {
               long lvalue=0;
               parser.Found(pname,&lvalue);
               pvalue.Printf(wxT("%i"),lvalue);
            }
            cmdMap.insert(make_pair(pname,pvalue));
         }
      }
   }
}


int main(int argc, char **argv)
{
   // initialise wxWidgets library
   wxInitializer initializer(argc,argv);

   // parse command line
   wxCmdLineParser parser(cmdLineDesc);
   parser.SetSwitchChars(wxT_2("-"));
   parser.SetCmdLine(argc,argv);
   if(parser.Parse() != 0) {
      // command line parameter error
      return 1;
   }

   // parser success
   // convert parameters to map
   CmdLineMap cmdMap;
   ParserToMap(parser,cmdMap);

   wxFileName  db_fname(cmdMap[wxT("database")]);

   int ndays   = boost::lexical_cast<int>(cmdMap["xdays"]);
   double elevation = boost::lexical_cast<double>(cmdMap["xelev"]);

   bool xrdat  = cmdMap.find("xrdat") != cmdMap.end();
   bool xhtml  = cmdMap.find("xhtml") != cmdMap.end();
   bool xutc   = cmdMap.find("xutc")  != cmdMap.end();


   // compute local time offset in seconds, relative to UTC
   // positive value means ahead of UTC
   // This value is only used in exported tables & reports
   int utc_offset = (xutc)? 0 : sqlTextExporter::utc_offset_seconds();

   // declare our one and only persistent class
   sqlWeatherStation::declare_persistent();

   string db_path               = db_fname.GetFullPath().ToStdString();
   const string db_logical_name = "wstation";

   op_database* db = op_mgr()->open_database(db_logical_name,db_path);
   if(!db) {
      cout << "Database not found " << db_path << endl;
      return 0;
   }

   {
      // start a read only transaction
      bool readonly=true;
      op_transaction trans(readonly);

      // query the previous "ndays" number of days
      list<op_pid> ids;
      time_t t_beg = sqlWeatherStation::time_instance(-ndays);
      time_t t_end = sqlWeatherStation::time_instance(0);
      db->select_ids(ids,op_typeid<sqlWeatherStation>(),sqlWeatherStation::time_clause(t_beg,t_end));

      // export data and compute rain intensities (i.e. mm/h)
      // latest rain intensity is returned as irain.
      double irain = 0;
      sqlTextExporter::export_list(ids,utc_offset,elevation,cout,irain);

      if(xhtml) {
         // report latest db entry in HTML format
         string xhtml_filename =  cmdMap[wxT("xhtml")].ToStdString();
         ofstream out(xhtml_filename.c_str());
         sqlTextExporter::export_numeric_html(ids,utc_offset,elevation,irain,out);
      }

   }

   op_mgr()->close_database(db_logical_name,false);
   return 0;
}


