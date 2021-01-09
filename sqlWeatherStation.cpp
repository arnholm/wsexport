#include "sqlWeatherStation.h"
#include "op_lite/op_manager.h"
#include <sstream>
#include <cmath>
#include <ctime>

// http://code.google.com/p/weatherpoller/
#define WS_RAIN_MM 0.3
#define WS_WIND_DEG 22.5

void sqlWeatherStation::declare_persistent()
{
   if(!op_mgr()->type_factory()->type_installed("sqlWeatherStation")) {
      op_mgr()->type_factory()->install(new op_persistent_class<sqlWeatherStation>());
   }
}


sqlWeatherStation::sqlWeatherStation()
: m_time("time_t",sqPK_1)
, m_itemp("itemp")    // we don't use op_construct here, to allow "m_" prefixes for the class member variables
, m_ihumi("ihumi")
, m_otemp("otemp")
, m_ohumi("ohumi")
, m_opres("opres")
, m_owspd("owspd")
, m_owgus("owgus")
, m_owdir("owdir")
, m_orain("orain")
, m_osens("osens")
{}

sqlWeatherStation::~sqlWeatherStation()
{}

void sqlWeatherStation::op_layout(op_values& pvalues)
{
   op_bind(pvalues,m_time);
   op_bind(pvalues,m_itemp);
   op_bind(pvalues,m_ihumi);
   op_bind(pvalues,m_otemp);
   op_bind(pvalues,m_ohumi);
   op_bind(pvalues,m_opres);
   op_bind(pvalues,m_owspd);
   op_bind(pvalues,m_owgus);
   op_bind(pvalues,m_owdir);
   op_bind(pvalues,m_orain);
   op_bind(pvalues,m_osens);
}

double sqlWeatherStation::opres(double elevation) const
{
   // http://en.wikipedia.org/wiki/Barometric_formula
   double otemp_K = 273.15 + m_otemp;
   return m_opres / (exp((-9.80665*0.0289664*elevation)/(8.31432*otemp_K)));
}

double sqlWeatherStation::owdir_deg() const
{
   return m_owdir*WS_WIND_DEG;
}

double sqlWeatherStation::orain() const
{
   return m_orain*WS_RAIN_MM;
}

string sqlWeatherStation::time_clause(time_t t_begin, time_t t_end)
{
   ostringstream qout;
   if(t_begin > 0 || t_end > 0) {
      qout << "WHERE ( ";
      if(t_begin > 0)qout << " ( time_t >= " << t_begin << " ) ";
      if(t_begin > 0 && t_end > 0)qout << " AND ";
      if(t_end   > 0)qout << " ( time_t < " << t_end << " ) ";
      qout << " )";
   }

   return qout.str();
}

time_t sqlWeatherStation::time_instance(int day_offset)
{
   time_t now;
   time(&now);

   return now + day_offset*24*60*60;
}

bool sqlWeatherStation::is_valid() const
{
   if(m_itemp <  -50.0 || m_itemp >  +100.0 )return false;
   if(m_otemp < -100.0 || m_otemp >  +100.0 )return false;
   if(m_opres < +500.0 || m_opres > +1500.0 )return false;
 //  if(/*m_ohumi <    1.0 ||*/ m_ohumi >  100.0  )return false;
   return true;
}
