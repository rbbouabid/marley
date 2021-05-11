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
#include <map>

// MARLEY includes
#include "marley/ElectronReaction.hh"
#include "marley/HauserFeshbachDecay.hh"
#include "marley/Logger.hh"
#include "marley/MatrixElement.hh"
#include "marley/NuclearReaction.hh"
#include "marley/Reaction.hh"
#include "marley/StructureDatabase.hh"
#include "marley/marley_kinematics.hh"
#include "marley/marley_utils.hh"

using ProcType = marley::Reaction::ProcessType;
using ME_Type = marley::MatrixElement::TransitionType;

namespace {

  using namespace marley_utils;

  std::map<ProcType, std::string> proc_type_to_string_map {
    { ProcType::NeutrinoCC, "\u03BD CC" },
    { ProcType::AntiNeutrinoCC, "anti-\u03BD CC" },
    { ProcType::NC, "NC" },
    { ProcType::NuElectronElastic, "(anti-)\u03BD + e\u207B ES" },
    { ProcType::DM, "DM" },
  };

  // Defines the neutrino species that can participate in each type
  // of scattering process
  std::map<ProcType, std::vector<int> > proc_type_to_nu_pdg = {

    { ProcType::NeutrinoCC,
      { ELECTRON_NEUTRINO, MUON_NEUTRINO, TAU_NEUTRINO }
    },

    { ProcType::AntiNeutrinoCC,
      { ELECTRON_ANTINEUTRINO, MUON_ANTINEUTRINO, TAU_ANTINEUTRINO }
    },

    { ProcType::NC,
      { ELECTRON_NEUTRINO, MUON_NEUTRINO, TAU_NEUTRINO,
        ELECTRON_ANTINEUTRINO, MUON_ANTINEUTRINO, TAU_ANTINEUTRINO }
    },

    { ProcType::NuElectronElastic,
        { ELECTRON_NEUTRINO, MUON_NEUTRINO, TAU_NEUTRINO,
          ELECTRON_ANTINEUTRINO, MUON_ANTINEUTRINO, TAU_ANTINEUTRINO }
    },

    { ProcType::DM,
        {DM}
    },

  };

  // Helper function that assigns Level pointers to MatrixElement objects
  // that represent transitions to discrete nuclear levels
  void set_level_ptrs(std::vector<marley::MatrixElement>& matrix_elements,
    int pdg_b, int pdg_d, marley::StructureDatabase& db)
  {
    // If discrete level data are available for the residual nucleus, use them
    // to assign values to the level pointers and refine the level energies. If
    // not, just return without doing anything. This will keep all of the level
    // pointers nullptr (treating them just like unbound levels) and have the
    // matrix elements use the energies given in the reaction dataset.
    marley::DecayScheme* ds = db.get_decay_scheme( pdg_d );
    if ( !ds ) return;

    // Check to see if the decay scheme being associated with this
    // reaction is for the correct nuclide. If the PDG code in the decay
    // scheme object does not match the one we'd expect for this reaction's
    // final state nucleus, complain
    int scheme_pdg = marley_utils::get_nucleus_pid( ds->Z(), ds->A() );
    if ( pdg_d != scheme_pdg ) throw marley::Error( "Nuclear data mismatch:"
        " attempted to associate a decay scheme object that has PDG code "
        + std::to_string(scheme_pdg) + " with a reaction object that has"
        " PDG code " + std::to_string(pdg_d) );

    // Use the smallest nuclear fragment emission threshold to check for
    // unbound levels.
    const auto& mt = marley::MassTable::Instance();
    double unbound_threshold = mt.unbound_threshold( pdg_d );

    // Get the spin-parity of the ground state of the initial nucleus.
    // This will be used below to check the matchups between MatrixElement and
    // Level objects based on spin-parity selection rules.
    int twoJi;
    marley::Parity Pi;
    marley::StructureDatabase::get_gs_spin_parity( pdg_b, twoJi, Pi );

    // Cycle through each of the level energies given in the reaction dataset.
    for ( auto& mat_el : matrix_elements ) {

      // Get the excitation energy for the level accessed by the transition
      // represented by this matrix element. Use the value from the reaction
      // data file rather than that owned by any previous discrete level
      // assignment. We'll use that value because we need to (re-)assign
      // levels to each matrix element using the DecayScheme ds.
      double en = mat_el.tabulated_level_energy();

      // If the level is above the fragment emission threshold, assign it a null
      // level pointer. Such levels will be handled by a fragment evaporation
      // routine and therefore do not need pointers to level objects describing
      // their de-excitation gammas.
      if ( en > unbound_threshold ) {
        mat_el.set_level(nullptr);
        continue;
      }

      // For each energy, find a pointer to the level with the closest energy
      // owned by the decay scheme object.
      marley::Level* plevel = ds->get_pointer_to_closest_level( en );
      MARLEY_LOG_DEBUG() << "reaction level at " << en
        << " MeV was matched to the decay scheme level at "
        << plevel->energy() << " MeV";

      // Print a warning if the spin-parity of the matched level does not
      // satisfy the expected selection rules for a transition from the nuclear
      // ground state

      // Retrieve the final nuclear spin (multiplied by two) and parity
      int twoJf = plevel->twoJ();
      marley::Parity Pf = plevel->parity();

      // Check the relevant selection rules
      bool rules_ok = true;
      ME_Type mtype = mat_el.type();
      if ( mtype == ME_Type::FERMI ) {
        if ( twoJi != twoJf || Pi != Pf ) rules_ok = false;
      }
      else if ( mtype == ME_Type::GAMOW_TELLER ) {
        if ( Pi != Pf ) rules_ok = false;
        int twoJf_min = std::abs( twoJi - 2 );
        int twoJf_max = twoJi + 2;
        if ( twoJf < twoJf_min || twoJf > twoJf_max ) rules_ok = false;
      }

      // Print a warning message if there was a problem
      if ( !rules_ok ) {

        // Use TargetAtom objects for easy printing of the nuclear symbols
        marley::TargetAtom nuc_b( pdg_b );
        marley::TargetAtom nuc_d( pdg_d );

        MARLEY_LOG_WARNING() << "The tabulated " << nuc_d << " level at "
          << plevel->energy() << " MeV does not satisfy the selection rules"
          << " for a " << mat_el.type_str() << " transition from the " << nuc_b
          << " ground state.\n Initial J\u03C0 = "
          << static_cast<double>( twoJi ) / 2. << Pi
          << "\n   Final J\u03C0 = " << static_cast<double>( twoJf ) / 2.
          << Pf;
      }

      // Complain if there are duplicates (if there are duplicates, we'll have
      // two different B(F) + B(GT) values for the same level object)
      const auto begin = matrix_elements.cbegin();
      const auto end = matrix_elements.cend();
      const auto found = std::find_if(begin, end,
        [plevel](const marley::MatrixElement& me) -> bool
        { return plevel == me.level(); });
      if ( found != end )
      {
        // One of the matrix elements already uses a level pointer equal to
        // plevel
        throw marley::Error("Reaction dataset gives two level energies that"
          " refer to the same DecayScheme level at "
          + std::to_string( plevel->energy() ) + " MeV");
      }

      /// @todo Add check to see if the energy of the chosen level is very
      /// different from the energy given in the reaction dataset. If it is,
      /// the level matchup is likely incorrect.

      // Set the level pointer in the MatrixElement object
      mat_el.set_level( plevel );
    }
  }

} // Anonymous namespace


// Performs kinematics calculations for a two-two scattering reaction
// (a + b -> c + d)
void marley::Reaction::two_two_scatter(double KEa, double& s, double& Ec_cm,
  double& pc_cm, double& Ed_cm) const
{
  // Get the lab-frame total energy of the projectile
  double Ea = KEa + ma_;

  // Compute Mandelstam s (the square of the total CM frame energy)
  s = ma_*ma_ + mb_*mb_ + 2.*mb_*Ea;
  double sqrt_s = std::sqrt(s);

  // Determine the CM frame energy and momentum of the ejectile
  Ec_cm = (s + mc_*mc_ - md_*md_) / (2 * sqrt_s);
  pc_cm = real_sqrt(std::pow(Ec_cm, 2) - mc_*mc_);

  // Determine the residue's CM frame energy. Roundoff errors may cause Ed_cm to
  // dip below md, which is unphysical. Prevent this from occurring by allowing
  // md to be the minimum value of Ed_cm. Also note that, in the CM frame, the
  // residue and ejectile have equal and opposite momenta.
  Ed_cm = std::max(sqrt_s - Ec_cm, md_);
}

marley::Event marley::Reaction::make_event_object(double KEa,
  double pc_cm, double cos_theta_c_cm, double phi_c_cm,
  double Ec_cm, double Ed_cm, double E_level, int twoJ,
  const marley::Parity& P) const
{
  double sin_theta_c_cm = real_sqrt(1.
    - std::pow(cos_theta_c_cm, 2));

  // Determine the Cartesian components of the ejectile's CM frame momentum
  double pc_cm_x = sin_theta_c_cm * std::cos(phi_c_cm) * pc_cm;
  double pc_cm_y = sin_theta_c_cm * std::sin(phi_c_cm) * pc_cm;
  double pc_cm_z = cos_theta_c_cm * pc_cm;

  // Get the lab-frame total energy of the projectile
  double Ea = KEa + ma_;

  // Determine the magnitude of the lab-frame 3-momentum of the projectile
  double pa = real_sqrt(KEa*(KEa + 2*ma_));

  // Create particle objects representing the projectile and target in the lab
  // frame
  // @todo Allow for projectile directions other than along the z-axis
  marley::Particle projectile(pdg_a_, Ea, 0, 0, pa, ma_);
  marley::Particle target(pdg_b_, mb_, 0, 0, 0, mb_);

  // Create particle objects representing the ejectile and residue in the CM
  // frame.
  marley::Particle ejectile(pdg_c_, Ec_cm, pc_cm_x, pc_cm_y, pc_cm_z, mc_);
  marley::Particle residue(pdg_d_, Ed_cm, -pc_cm_x, -pc_cm_y, -pc_cm_z, md_);

  // Boost the ejectile and residue into the lab frame.
  double beta_z = pa / (Ea + mb_);
  marley_kinematics::lorentz_boost(0, 0, -beta_z, ejectile);
  marley_kinematics::lorentz_boost(0, 0, -beta_z, residue);

  // Create the event object and load it with the appropriate information
  marley::Event event(projectile, target, ejectile, residue, E_level, twoJ, P);
  return event;
}

int marley::Reaction::get_ejectile_pdg(int pdg_a, ProcType proc_type) {
  int pdg_c = 0;

  // First, check that the projectile PDG code is valid for the
  // given process type
  const auto& vec = proc_type_to_nu_pdg.at( proc_type );
  if ( std::find(vec.cbegin(), vec.cend(), pdg_a) != vec.end() ) {
    if ( proc_type == ProcType::NeutrinoCC ) pdg_c = pdg_a - 1;
    else if ( proc_type == ProcType::AntiNeutrinoCC ) pdg_c = pdg_a + 1;
    else if ( proc_type == ProcType::NC ) pdg_c = pdg_a;
    else if ( proc_type == ProcType::NuElectronElastic ) pdg_c = pdg_a;
    else if ( proc_type == ProcType::DM ) pdg_c = 11;
    else throw marley::Error("Unrecognized ProcessType encountered in"
      " marley::Reaction::get_ejectile_pdg()");
  }
  else throw marley::Error("A projectile with PDG code "
    + std::to_string(pdg_a) + " cannot participate in reactions of type "
    + proc_type_to_string_map.at(proc_type));

  return pdg_c;
}

std::string marley::Reaction::proc_type_to_string(const ProcType& pt) {
  return proc_type_to_string_map.at( pt );
}

const std::vector<int>& marley::Reaction::get_projectiles(ProcType pt) {
  return proc_type_to_nu_pdg.at( pt );
}

std::vector< std::unique_ptr<marley::Reaction> >
  marley::Reaction::load_from_file(const std::string& filename,
  marley::StructureDatabase& db)
{
  std::cout<<"i get here top -- reaction.cc"<<std::endl;
  // Create an empty vector to start
  std::vector< std::unique_ptr<marley::Reaction> > loaded_reactions;

  std::regex rx_comment("#.*"); // Matches comment lines

  // Open the reaction data file for parsing
  std::ifstream file_in( filename );

  // If the file doesn't exist or some other error
  // occurred, complain and give up.
  if ( !file_in.good() ) {
    throw marley::Error("Could not read from the file " + filename);
  }

  // String to store the current line of the reaction data file during parsing
  std::string line;

  /// @todo Add error handling for parsing problems
  line = marley_utils::get_next_line( file_in, rx_comment, false );

  // Read in the ProcessType code and the target PDG code
  std::istringstream iss( line );
  int integer_proc_type;
  iss >> integer_proc_type;

  auto proc_type = static_cast<ProcType>( integer_proc_type );

  // For neutrino-electron elastic scattering, we won't have a table of
  // matrix elements. Instead, a table of atomic target PDG codes appears.
  // Make Reaction objects for each atomic target for each of the possible
  // projectiles (every neutrino species) and return the result.
  if ( proc_type == ProcessType::NuElectronElastic ) {

    do {
      // Loop over target atoms
      int target_pdg;
      while ( iss >> target_pdg ) {
        // Loop over neutrino species
        for ( const int& pdg_a : get_projectiles(proc_type) ) {
          loaded_reactions.emplace_back(
            std::make_unique<marley::ElectronReaction>(pdg_a, target_pdg) );
        }
      }

      line = marley_utils::get_next_line( file_in, rx_comment, false );
      /// @todo This can be done more cleanly via
      /// iss = std::istringstream( line );
      /// However, GCC 4 does not include the move constructor for string
      /// streams. This was fixed for GCC 5. Change this line after MARLEY
      /// drops support for GCC < 5. See https://stackoverflow.com/a/27152585
      /// and https://stackoverflow.com/q/7623650 for more details.
      iss.str( line );
      iss.clear();
    } while ( !line.empty() );

    return loaded_reactions;
  }

  // For nuclear reaction modes, there is a single target nucleus PDG code
  // per file. After parsing it, we proceed to read in the matrix elements.
  int pdg_b;
  iss >> pdg_b;

  // Read in all of the level energy (MeV), squared matrix element (B(F) or
  // B(GT) strength), and matrix element type identifier (0 represents B(F), 1
  // represents B(GT)) triplets. Create a vector of MatrixElement objects based
  // on this information. Use a shared pointer so that the vector can be
  // re-used by multiple Reaction objects, one for each neutrino species for
  // which the matrix elements are relevant. This avoids unnecessary
  // duplication of storage for the matrix elements.
  auto matrix_elements = std::make_shared<std::vector<
    marley::MatrixElement> >();

  // Set the old energy entry to the lowest representable double
  // value. This guarantees that we always read in the first energy
  // value given in the reaction data file
  double old_energy = std::numeric_limits<double>::lowest();
  while (line = marley_utils::get_next_line(file_in, rx_comment, false),
    file_in.good())
  {
    iss.str(line);
    iss.clear();
    /// @todo Consider implementing a sorting procedure rather than strictly
    /// enforcing that energies must be given in ascending order.

    // The order of the entries is important because later uses of the vector of
    // matrix elements assume that they are sorted in order of ascending final
    // level energy.
    double energy, strength;
    int integer_me_type;
    iss >> energy >> strength >> integer_me_type;
    if (old_energy >= energy) throw marley::Error(std::string("Invalid")
      + " reaction dataset. Level energies must be unique and must be"
      + " given in ascending order.");

    // @todo Right now, 0 corresponds to a Fermi transition, and 1 corresponds
    // to a Gamow-Teller transition. As you add new matrix element types,
    // consider changing the convention and its implementation.
    // All of the level pointers owned by the matrix elements will initially be
    // set to nullptr. This may be changed later if discrete level data can be
    // found for the residual nucleus.
    matrix_elements->emplace_back(energy, strength,
      static_cast<ME_Type>(integer_me_type), nullptr);
    old_energy = energy;
  }

  // We now have all the information that we need. Build Reaction objects
  // for all neutrino species that can participate in the process described
  // by the matrix elements in the table. Use the ProcessType code to
  // figure this out

  // First, figure out the PDG code for the final nucleus and its ionization
  // state (net atomic charge after the 2->2 scatter)
  int Zi = marley_utils::get_particle_Z( pdg_b );
  int A = marley_utils::get_particle_A( pdg_b );

  int pdg_d, q_d;
  // NC scattering leaves the target nucleus the same
  if ( proc_type == ProcessType::NC ) {
    pdg_d = pdg_b;
    q_d = 0;
  }
  // Neutrino CC scattering raises Z by one
  else if ( proc_type == ProcessType::NeutrinoCC ) {
    // Check that the neutron number of the target is positive
    int Ni = A - Zi;
    if ( Ni <= 0 ) throw marley::Error("A NeutrinoCC process requires"
      " a target nucleus with N > 0");
    int Zf = Zi + 1;
    pdg_d = marley_utils::get_nucleus_pid(Zf, A);
    // Recoil ion has charge +1
    q_d = 1;
  }
  // Antineutrino CC scattering lowers Z by one
  else if ( proc_type == ProcessType::AntiNeutrinoCC ) {
    // Check that the neutron number of the target is positive
    if ( Zi <= 0 ) throw marley::Error("An AntiNeutrinoCC process requires"
      " a target nucleus with Z > 0");
    int Zf = Zi - 1;
    pdg_d = marley_utils::get_nucleus_pid(Zf, A);
    // Recoil ion has charge -1
    q_d = -1;
  }
  // DM induced beta- decay scattering raises Z by one
  else if ( proc_type == ProcessType::DM ) {
    // Check that the neutron number of the target is positive
    int Ni = A - Zi;
    if ( Ni <= 0 ) throw marley::Error("A DM process requires"
      " a target nucleus with N > 0");
    int Zf = Zi + 1;
    pdg_d = marley_utils::get_nucleus_pid(Zf, A);
    // Recoil ion has charge +1
    q_d = 1;
  }
  else throw marley::Error("Unrecognized ProcessType encountered in"
    " marley::NuclearReaction::load_from_file()");

  // Now that we know the PDG code for the final nucleus, look up discrete
  // level data for it. Set the level pointers for matrix elements representing
  // transitions to discrete nuclear levels
  set_level_ptrs( *matrix_elements, pdg_b, pdg_d, db );

  // Now loop over the projectile PDG codes that can participate in the
  // scattering process of interest. For each one, decide what the ejectile
  // PDG code should be, then produce a corresponding Reaction object
  for ( const int& pdg_a : get_projectiles(proc_type) ) {
    int pdg_c = get_ejectile_pdg(pdg_a, proc_type);

    loaded_reactions.emplace_back( std::make_unique<marley::NuclearReaction>(
      proc_type, pdg_a, pdg_b, pdg_c, pdg_d, q_d, matrix_elements) );

  }

  return loaded_reactions;
}
