/// @file
/// @copyright Copyright (C) 2016-2021 Steven Gardiner
/// @license GNU General Public License, version 3
//
// This file is part of MARLEY (Model of Argon Reaction Low Energy Yields)
//
// MARLEY is free software: you can redistribute it and/or modify it under the
// terms of version 3 of the GNU General Public License as published by the
// Free Software Foundation.
//
// For the full text of the license please see COPYING or
// visit http://opensource.org/licenses/GPL-3.0
//
// Please respect the MCnet academic usage guidelines. See GUIDELINES
// or visit https://www.montecarlonet.org/GUIDELINES for details.

// Standard library includes
#include <fstream>
#include <iostream>
#include <string>

// MARLEY includes
#include "marley/DecayScheme.hh"
#include "marley/Generator.hh"
#include "marley/JSON.hh"
#include "marley/NeutrinoSource.hh"
#include "marley/NuclearReaction.hh"
#include "marley/StructureDatabase.hh"
#include "marley/marley_utils.hh"

#ifdef USE_ROOT
  #include "marley/RootJSONConfig.hh"
#else
  #include "marley/JSONConfig.hh"
#endif

namespace {
  // Default settings for the projectile kinetic energy range and number of
  // steps for the total cross section dump
  constexpr double DEFAULT_MASS_MIN = 1.5;
  constexpr double DEFAULT_MASS_MAX = 15.;
  constexpr double DEFAULT_LAMBDA_MIN = 100000.;
  constexpr double DEFAULT_LAMBDA_MAX = 100000000.;
  constexpr int DEFAULT_NUM_STEPS = 50;

  // Default projectile PDG code
  constexpr int DEFAULT_PDG = marley_utils::DM;

  // Helper functions for loading custom dump parameters from the job
  // configuration file
  // TODO: reduce code duplication between these two functions
  void get_double_dump_param( const marley::JSON& json,
    const std::string& param_key, double& value )
  {
    // If the key doesn't appear in the job configuration file, just return
    // without doing anything (all cross section dump parameters are optional)
    if ( !json.has_key(param_key) ) return;

    const auto& temp_js = json.at( param_key );
    bool ok = false;
    value = temp_js.to_double( ok );
    if ( !ok ) throw marley::Error("Unrecognized " + param_key
      + " value " + temp_js.to_string() + " encountered in the"
      " job configuration file.");
  }

  void get_int_dump_param( const marley::JSON& json,
    const std::string& param_key, int& value )
  {
    // If the key doesn't appear in the job configuration file, just return
    // without doing anything (all cross section dump parameters are optional)
    if ( !json.has_key(param_key) ) return;

    const auto& temp_js = json.at( param_key );
    bool ok = false;
    value = temp_js.to_long( ok );
    if ( !ok ) throw marley::Error("Unrecognized " + param_key
      + " value " + temp_js.to_string() + " encountered in the"
      " job configuration file.");
  }

}

int main(int argc, char* argv[]) {

  // If the user has not supplied enough command-line arguments, display the
  // standard help message and exit
  if (argc <= 2) {
    std::cout << "Usage: " << argv[0] << " OUTPUT_FILE CONFIG_FILE\n";
    return 1;
  }

  // Get the output and config file names from the command line
  std::string output_file_name( argv[1] );
  std::string config_file_name( argv[2] );

  // Check whether the output file exists and warn the user before
  // overwriting it if it does
  std::ifstream temp_stream( output_file_name );
  if ( temp_stream ) {
    bool overwrite = marley_utils::prompt_yes_no(
      "Really overwrite " + output_file_name + '?');
    if ( !overwrite ) {
      std::cout << "Total cross section dump aborted.\n";
      return 0;
    }
  }

  // Open the output file for writing
  std::ofstream out_file( output_file_name );

  // Configure a new Generator object
  #ifdef USE_ROOT
    marley::RootJSONConfig config( config_file_name );
  #else
    marley::JSONConfig config( config_file_name );
  #endif
  marley::Generator gen = config.create_generator();

  // Initialize the projectile kinetic energy range and step size to use for
  // the total cross section dump
  double dm_mass_min = DEFAULT_MASS_MIN;
  double dm_mass_max = DEFAULT_MASS_MAX;
  double UV_cutoff_min = DEFAULT_LAMBDA_MIN;
  double UV_cutoff_max = DEFAULT_LAMBDA_MAX;
  int num_steps = DEFAULT_NUM_STEPS;
  int projectile_pdg = DEFAULT_PDG;

  // If the user has specified a non-default energy range or step size, then
  // retrieve that information from the config file.
  const marley::JSON& json = config.get_json();

  //get_double_dump_param( json, "xsec_dump_KEmin", KEmin );
  //get_double_dump_param( json, "xsec_dump_KEmax", KEmax );

  //get_int_dump_param( json, "xsec_dump_steps", num_steps );
  //get_int_dump_param( json, "xsec_dump_pdg", projectile_pdg );

  // Compute the (linear) step size based on the number of steps and the
  // desired projectile energy range
  double delta_mass_step = ( dm_mass_max - dm_mass_min ) / num_steps;
  double delta_cutoff_step = ( UV_cutoff_max - UV_cutoff_min ) / num_steps;

  // Initial projectile energy to use in the loop
  double dm_mass = dm_mass_min;
  double UV_cutoff = UV_cutoff_min;

  std::cout<<"Masses will range from "<<dm_mass_min<<" to "<<dm_mass_max<<std::endl;

  // adding some calculations for signal vs solar nu background
  // all of this is being hard coded in for 1kT of 40Ar and not at all configurable yet
  double background = 9430.; // count / kT-year
  double exposure = 1000000.; // 1kT
  double mi = 37214.65445386492; 
  double num_atoms = (40.-19.)*exposure/(1.79 * std::pow(10.,-30.))/(mi);
  double convertSigmaCMsquared = std::pow(1000. * 1.98 * std::pow(10.,-14.),2);
  double sec_per_yr = 3.154 * std::pow(10.,7.);
  double rho = 200.;
  double pi = 3.1415926539;



  // Ready for the dump now
  for ( int s = 0; s < num_steps; ++s ) {

    // Update the projectile energy for this step
    dm_mass += delta_mass_step;

    for ( int r = 0; r < num_steps; ++r ) {
    
      UV_cutoff +=delta_cutoff_step;

      // Compute the total cross section at this projectile
      double dm_xsec = gen.total_xs( 17, 1.0, dm_mass, UV_cutoff);
      double signal_events = dm_xsec*sec_per_yr*convertSigmaCMsquared*(3*std::pow(10.,10.))*rho/dm_mass*num_atoms;

      // Compute the confidence interval
      double significance = signal_events / std::sqrt(background);

      // Compute the y axis
      double y = convertSigmaCMsquared * std::pow(dm_mass,2.) / ( 4. * pi * std::pow(UV_cutoff,4.) );
      


      // Write the current kinetic energy and total cross section to the output
      // file
      out_file << dm_mass << ' ' << UV_cutoff << ' ' << y << ' ' << signal_events << ' ' << significance << '\n';

      // Also write these quantities to the Logger
      MARLEY_LOG_INFO() << "dm mass = " << dm_mass << ", UV cutoff = " << UV_cutoff << ", dm total xsec = "
        << dm_xsec << ", Events = " << signal_events;// << '\n';

    }
    UV_cutoff = UV_cutoff_min;

  }

  return 0;
}
