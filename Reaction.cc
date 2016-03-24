#include "Kinematics.hh"
#include "Reaction.hh"

// Performs kinematics calculations for a two-two scattering reaction
// (a + b -> c + d)
void marley::Reaction::two_two_scatter(double Ea, double& s, double& Ec_cm,
  double& pc_cm, double& Ed_cm)
{
  // Compute Mandelstam s (the square of the total CM frame energy)
  s = ma2 + mb2 + 2 * mb * Ea;
  double sqrt_s = std::sqrt(s);

  // Determine the CM frame energy and momentum of the ejectile
  Ec_cm = (s + mc2 - md2) / (2 * sqrt_s);
  pc_cm = marley_utils::real_sqrt(std::pow(Ec_cm, 2) - mc2);

  // Determine the residue's CM frame energy. Roundoff errors may cause Ed_cm to
  // dip below md, which is unphysical. Prevent this from occurring by allowing
  // md to be the minimum value of Ed_cm. Also note that, in the CM frame, the
  // residue and ejectile have equal and opposite momenta.
  Ed_cm = std::max(sqrt_s - Ec_cm, md);
}

marley::Event marley::Reaction::make_event_object(double Ea,
  double pc_cm, double cos_theta_c_cm, double phi_c_cm,
  double Ec_cm, double Ed_cm, double E_level)
{
  double sin_theta_c_cm = marley_utils::real_sqrt(1
    - std::pow(cos_theta_c_cm, 2));

  // Determine the Cartesian components of the ejectile's CM frame momentum
  double pc_cm_x = sin_theta_c_cm * std::cos(phi_c_cm) * pc_cm;
  double pc_cm_y = sin_theta_c_cm * std::sin(phi_c_cm) * pc_cm;
  double pc_cm_z = cos_theta_c_cm * pc_cm;

  // Determine the magnitude of the lab-frame 3-momentum of the projectile
  double pa = marley_utils::real_sqrt(std::pow(Ea, 2) - ma2);

  // Create particle objects representing the projectile and target in the lab frame
  // TODO: edit this to allow for projectile directions other than along the z-axis
  marley::Particle projectile(pid_a, Ea, 0, 0, pa, ma);
  marley::Particle target(pid_b, mb, 0, 0, 0, mb);

  // Create particle objects representing the ejectile and residue in the CM frame.
  marley::Particle ejectile(pid_c, Ec_cm, pc_cm_x, pc_cm_y, pc_cm_z, mc);
  marley::Particle residue(pid_d, Ed_cm, -pc_cm_x, -pc_cm_y, -pc_cm_z, md);

  // Boost the ejectile and residue into the lab frame.
  // TODO: edit this to allow for projectile directions other than along the z-axis
  double beta_z = pa / (Ea + mb);
  marley::Kinematics::lorentz_boost(0, 0, -beta_z, ejectile);
  marley::Kinematics::lorentz_boost(0, 0, -beta_z, residue);

  // Create the event object and load it with the appropriate information
  marley::Event event(E_level);
  event.set_reaction(this);

  // Add the projectile to this event's initial particle list
  event.add_initial_particle(projectile,
    marley::Event::ParticleRole::pr_projectile);
  // Add the target to this event's initial particle list
  event.add_initial_particle(target, marley::Event::ParticleRole::pr_target);
  // Add the ejectile to this event's final particle list
  event.add_final_particle(ejectile, marley::Event::ParticleRole::pr_ejectile);
  // Add the residue to this event's final particle list.
  event.add_final_particle(residue, marley::Event::ParticleRole::pr_residue);
  // Return the completed event object
  return event;
}