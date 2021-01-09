#ifndef SQLWEATHERSTATION_H
#define SQLWEATHERSTATION_H

#include "op_lite/op_object.h"
#include <string>
using namespace std;

// sqlWeatherStation privides access to a single measurement instance


class sqlWeatherStation : public op_object {
public:
   sqlWeatherStation();
   virtual ~sqlWeatherStation();

   // convenience function to declare this as persistent class
   // called at program startup
   static void declare_persistent();

   /// get persistence layout of this type
   /*! /param pvalues container of persistent member variables */
   virtual void op_layout(op_values& pvalues);

   // returns true if the sensor values are within expected ranges
   bool is_valid() const;

   time_t time_utc() const { return (time_t)m_time; }
   double itemp() const    { return m_itemp; }
   double ihumi() const    { return m_ihumi; }

   double otemp() const    { return m_otemp; }
   double ohumi() const    { return m_ohumi; }
   double opres(double elevation=0.0) const;  // pressure adjusted to sea level
   double owspd() const    { return m_owspd; }
   double owgus() const    { return m_owgus; }
   size_t owdir() const    { return m_owdir; }
   double owdir_deg() const;
   double orain() const;
   size_t osens() const    { return m_osens; }

   // generate a time clause string for the time span
   // an open interval can be specified with t_begin=0 or t_end=0
   static string time_clause(time_t t_begin, time_t t_end);

   // compute a time relative to now. (times in the past require negative day_offset)
   static time_t time_instance(int day_offset);

private:

   op_int    m_time;   // unix time (time_t)

   // indoor sensors
   op_double m_itemp;  // indoor temperature [C]
   op_double m_ihumi;  // indoor humidity [%]

   // outdoor sensors
   op_double m_otemp;  // outdoor temperature [C]
   op_double m_ohumi;  // outdoor humidity [%]
   op_double m_opres;  // pressure [mbar]
   op_double m_owspd;  // wind speed [m/s]
   op_double m_owgus;  // wind gust [m/s]
   op_int    m_owdir;  // wind direction [deg]
   op_int    m_orain;  // rain accumulated [mm]
   op_int    m_osens;  // outdoor sensors available [1/0]

};

#endif // SQLWEATHERSTATION_H
