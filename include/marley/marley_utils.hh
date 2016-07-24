// Nonstandard but widely-supported (see
// http://en.wikipedia.org/wiki/Pragma_once) preprocessor directive that
// prevents this file from being included multiple times.  Consider changing to
// an include guard (http://en.wikipedia.org/wiki/Include_guard) if desired by
// project collaborators.
#pragma once

#include <algorithm>
#include <chrono>
#include <complex>
#include <functional>
#include <unordered_map>
#include <random>
#include <regex>
#include <string>

namespace marley_utils {

  // MARLEY version string
  constexpr const char* MARLEY_VERSION = "0.9.4";

  // Frequently used particle IDs
  constexpr int PHOTON = 22;
  constexpr int ELECTRON = 11;
  constexpr int POSITRON = -11;
  constexpr int ELECTRON_NEUTRINO = 12;
  constexpr int ELECTRON_ANTINEUTRINO = -12;
  constexpr int MUON = 13;
  constexpr int MUON_NEUTRINO = 14;
  constexpr int MUON_ANTINEUTRINO = -14;
  constexpr int TAU = 15;
  constexpr int TAU_NEUTRINO = 16;
  constexpr int TAU_ANTINEUTRINO = -16;
  constexpr int NEUTRON = 2112;
  constexpr int PROTON = 2212;
  constexpr int DEUTERON = 1000010020;
  constexpr int TRITON = 1000010030;
  constexpr int HELION = 1000020030;
  constexpr int ALPHA = 1000020040;

  // Fermi coupling constant (MeV^(-2))
  constexpr double GF = 1.16637e-11;

  // Absolute value of the CKM matrix element for mixing between the up and
  // down quarks abs(V_ud)
  constexpr double Vud = 0.97427;

  // sin^2(theta_W) (sine squared of the weak mixing angle)
  // Effective value taken from 2014 PDG Review of Particle Physics,
  // Table 1.1, "Physical Constants"
  constexpr double sin2thetaw = 0.23155;

  // Conversion factor to use when expressing ENSDF energies (keV) in
  // standard MARLEY energy units (MeV)
  constexpr double MeV = 1e-3;

  // Conversion factor to use when expressing atomic masses (micro-amu)
  // in standard MARLEY energy units (MeV)
  constexpr double micro_amu = 0.000931494061;

  // Infinities
  constexpr double infinity = std::numeric_limits<double>::max();
  constexpr double minus_infinity = -infinity;

  // Muon mass
  constexpr double m_mu = 113428.9267; // micro-amu

  // Consistent value of pi to use throughout all of MARLEY
  constexpr double pi = 3.14159265358979323846;
  constexpr double two_pi = 2*pi;
  const double sqrt_two_pi = std::sqrt(pi);
  constexpr double half_pi = pi/2.0;

  // Imaginary unit
  constexpr std::complex<double> i(0, 1);

  // Natural logarithm of 2
  const double log_2 = std::log(2);

  // The physical constants given here were all taken from
  // the 2014 edition of the Review of Particle Physics
  // published by the Particle Data Group.

  // Fine structure constant
  constexpr double alpha = 7.2973525698e-3;
  // Conversion factor used to switch to natural units (hbar = c = 1)
  constexpr double hbar_c = 197.3269718; // MeV*fm
  constexpr double hbar_c2 = hbar_c * hbar_c; // MeV^2 * fm^2
  // Electron mass
  constexpr double m_e = 0.510998928; // MeV
  // Constant to use when converting from mb to MeV^(-2)
  constexpr double mb = 1/3.89379338e5; // MeV^(-2) mb^(-1)
  // Square of the elementary charge
  constexpr double e2 = hbar_c * alpha; // MeV*fm
  // Constant to use when approximating nuclear radii via
  // r = r0 * A^(1/3), where A is the nucleus's mass number.
  // See, for example, Introductory Nuclear Physics by Kenneth S. Krane.
  constexpr double r0 = 1.2; // fm
  // Handy constants for the fractions 1/2 and 1/3
  constexpr double ONE_HALF = 1.0/2.0;
  constexpr double ONE_THIRD = 1.0/3.0;

  // Strings to use for latex table output of DecayScheme objects
  extern std::string latex_table_1, latex_table_2, latex_table_3, latex_table_4;

  // Create an ENSDF nucid string given a nuclide's atomic number Z
  // and mass number A
  std::string nuc_id(int Z, int A);

  // Return the PDG particle ID that corresponds to a ground-state
  // nucleus with atomic number Z and mass number A
  inline int get_nucleus_pid(int Z, int A) {
    if (Z == 0 && A == 1) return NEUTRON;
    else return 10000*Z + 10*A + 1000000000;
  }

  inline int get_particle_Z(int pid) {
    if (pid == marley_utils::PROTON) return 1;
    else if (pid == marley_utils::NEUTRON) return 0;
    // nuclear fragment
    else if (pid > 1000000000) return (pid % 10000000)/10000;
    // other particle
    else return 0;
  }

  inline int get_particle_A(int pid) {
    if (pid == marley_utils::PROTON) return 1;
    else if (pid == marley_utils::NEUTRON) return 1;
    // nuclear fragment
    else if (pid > 1000000000) return (pid % 10000)/10;
    // other particle
    else return 0;
  }

  // Take the square root of a number. Assume that a negative argument is
  // due to roundoff error and return zero in such cases rather than NaN.
  double real_sqrt(double num);

  // Compute the complex gamma function using the Lanczos approximation
  std::complex<double> gamma(std::complex<double> z);

  // Numerically integrate a 1D function using the composite trapezoid rule
  double num_integrate(const std::function<double(double)> &f,
    double a, double b, int n);

  // Numerically minimize or maximize a function of one variable using
  // Brent's method (see http://en.wikipedia.org/wiki/Brent%27s_method)
  double minimize(const std::function<double(double)> f, double leftEnd,
    double rightEnd, double epsilon, double& minLoc);

  double maximize(const std::function<double(double)> f, double leftEnd,
    double rightEnd, double epsilon, double& maxLoc);

  // Find both solutions of a quadratic equation while attempting
  // to avoid floating-point arithmetic issues
  void solve_quadratic_equation(double A, double B,
    double C, double &solPlus, double &solMinus);

  // Efficiently read in an entire file as a std::string
  std::string get_file_contents(std::string filename);

  // Advance to the next line of an ifstream that either matches (match == true)
  // or does not match (match == false) a given regular expression
  std::string get_next_line(std::ifstream &file_in, const std::regex &rx,
    bool match);
  // Do the same, but store the number of lines used in num_lines
  std::string get_next_line(std::ifstream &file_in, const std::regex &rx,
    bool match, int& num_lines);

  // String containing all of the characters that will be
  // considered whitespace by default in the string
  // manipulation functions below
  const std::string whitespace = " \f\n\r\t\v";

  // This version of std::stod will return 0 if it encounters
  // an empty string or an all-whitespace string.
  inline double str_to_double(const std::string& s) {
    size_t endpos = s.find_last_not_of(whitespace);
    if (endpos == std::string::npos) {
      return 0.0; // string was all whitespace
    }
    else {
      return std::stod(s);
    }
  }

  // Function that creates a copy of a std::string object
  // that has been converted to all lowercase
  inline std::string to_lowercase(const std::string& s) {
    std::string new_s = s;
    std::transform(new_s.begin(), new_s.end(), new_s.begin(), ::tolower);
    return new_s;
  }

  // Function that converts a std::string object to
  // all lowercase in place and returns a reference to
  // it afterwards
  inline std::string& to_lowercase_inplace(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
  }

  // Function that converts a std::string object to
  // all uppercase in place and returns a reference to
  // it afterwards
  inline std::string& to_uppercase_inplace(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
  }

  // Functions for padding std::string objects in place.  They all return
  // references to the string afterwards.  These functions are based on
  // http://stackoverflow.com/a/667219/4081973
  inline std::string& pad_left_inplace(std::string &str,
    const size_t len, const char pad_char = ' ')
  {
    if(len > str.size())
    str.insert(0, len - str.size(), pad_char);
    return str;
  }

  inline std::string& pad_right_inplace(std::string &str,
    const size_t len, const char pad_char = ' ')
  {
    if(len > str.size())
    str.append(len - str.size(), pad_char);
    return str;
  }

  // These std::string trimming functions were taken from code
  // presented here: http://www.cplusplus.com/faq/sequences/strings/trim/
  // and here: http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
  // The first three (with the suffix copy) return a trimmed copy of
  // the string without modifying the original.
  inline std::string trim_right_copy(const std::string& s,
    const std::string& delimiters = whitespace)
  {
    size_t endpos = s.find_last_not_of(delimiters);
    return (endpos == std::string::npos) ? "" : s.substr(0, endpos + 1);
  }

  inline std::string trim_left_copy(const std::string& s,
    const std::string& delimiters = whitespace)
  {
    size_t startpos = s.find_first_not_of(delimiters);
    return (startpos == std::string::npos) ? "" : s.substr(startpos);
  }

  inline std::string trim_copy(const std::string& s,
    const std::string& delimiters = whitespace)
  {
    return trim_left_copy(trim_right_copy(s, delimiters), delimiters);
  }

  // The second three alter the original string, returning a
  // reference to it after it has been trimmed.
  inline std::string& trim_right_inplace(std::string& s,
    const std::string& delimiters = whitespace)
  {
    size_t endpos = s.find_last_not_of(delimiters);
    if (endpos == std::string::npos) {
      s.clear();
    }
    else {
      s.erase(endpos + 1);
    }
    return s;
  }

  inline std::string& trim_left_inplace(std::string& s,
    const std::string& delimiters = whitespace)
  {
    size_t startpos = s.find_first_not_of(delimiters);
    if (startpos == std::string::npos) {
      s.clear();
    }
    else {
      s.erase(0, startpos);
    }
    return s;
  }

  inline std::string& trim_inplace(std::string& s,
    const std::string& delimiters = whitespace)
  {
    return trim_left_inplace(trim_right_inplace(s,delimiters), delimiters);
  }

  // Function that takes a number of bytes and returns a string
  // representing the amount of memory in more readable units
  std::string num_bytes_to_string(double bytes, unsigned precision = 3);

  // Trim an ENSDF nucid string and make two-letter element symbols have a
  // lowercase last letter. Currently, no checking is done to see if the
  // string is a valid nucid.
  std::string nucid_to_symbol(std::string nucid);

  // Similar to nucid_to_symbol, but returns the atomic number as an integer
  // instead
  int nucid_to_Z(std::string nucid);

  // Generalized std::chrono::duration helper types
  template <typename repType> using
    seconds = std::chrono::duration<repType>;
  template <typename repType> using
    minutes = std::chrono::duration<repType, std::ratio<60>>;
  template <typename repType> using
    hours = std::chrono::duration<repType, std::ratio<3600>>;
  template <typename repType> using
    days = std::chrono::duration<repType, std::ratio<86400>>;

  // This function is a generalized version of code taken from the accepted answer at
  // http://stackoverflow.com/questions/15957805/extract-year-month-day-etc-from-stdchronotime-point-in-c
  //
  // Function that returns a string representation (in the
  // format days hours:minutes:seconds) of a std::chrono::duration object
  template <typename repType, typename periodType = std::ratio<1>> std::string duration_to_string(
    std::chrono::duration<repType, periodType> duration)
  {
    int day_count = static_cast<int>(std::chrono::duration_cast
      <marley_utils::days<repType>>(duration) / (marley_utils::days<repType>(1)));
    duration -= marley_utils::days<repType>(day_count);

    int hour_count = static_cast<int>(std::chrono::duration_cast
      <marley_utils::hours<repType>>(duration) / (marley_utils::hours<repType>(1)));
    duration -= marley_utils::hours<repType>(hour_count);

    int minute_count = static_cast<int>(std::chrono::duration_cast
      <marley_utils::minutes<repType>>(duration) / (marley_utils::minutes<repType>(1)));
    duration -= marley_utils::minutes<repType>(minute_count);

    int second_count = static_cast<int>(std::chrono::duration_cast
      <marley_utils::seconds<repType>>(duration) / (marley_utils::seconds<repType>(1)));
    duration -= marley_utils::seconds<repType>(second_count);

    std::ostringstream out;
    if (day_count > 1) {
      out << day_count << " days ";
    }
    if (day_count == 1) {
      out << day_count << " day ";
    }
    if (hour_count < 10) out << "0";
    out << hour_count << ":";
    if (minute_count < 10) out << "0";
    out << minute_count << ":";
    if (second_count < 10) out << "0";
    out << second_count;

    return out.str();
  }

  template <typename durationType> std::string duration_to_string(
    durationType duration)
  {
    return duration_to_string<typename durationType::rep,
      typename durationType::period>(duration);
  }

  // Function that takes two std::system_clock::time_point objects and returns
  // a string (in the same format as marley_utils::duration_to_string)
  // representing the time between them
  std::string elapsed_time_string(
    std::chrono::system_clock::time_point &start_time,
    std::chrono::system_clock::time_point &end_time);

  // Lookup table for particle symbols (keys are PDG particle IDs,
  // values are symbols).
  const std::unordered_map<int, std::string> particle_symbols = {
    { 22, "g"},//"\u03B3"},
    { 11, "e"},
    { 12, "nu_e"},//"\u03BD"},
    { 2112, "n"},
    { 2212, "p"},
    { 1000010020, "d"},
    { 1000010030, "t"},
    { 1000020030, "h"},
    { 1000020040, "a"},//"\u03B1"},
  };

  // Lookup table for particle electric charges (keys are PDG particle IDs,
  // values are charges expressed as integer multiples of the proton charge).
  const std::unordered_map<int, int> particle_electric_charges = {
    { 11, -1},
    { 12, 0},
    { 13, -1},
    { 14, 0},
    { 15, -1},
    { 16, 0},
    { 22, 0},
    { 2112, 0},
    { 2212, 1},
    //{ 1000010020, 1}, // bare deuteron
    //{ 1000010030, 1}, // bare triton
    //{ 1000020030, 2}, // bare helion
    //{ 1000020040, 2}, // bare alpha
  };

  // Looks up the electric charge of a particle based on its PDG particle ID
  inline int get_particle_charge(int pid) {
    // If a nuclear particle ID is supplied to this function, assume
    // that it is a bare nucleus, and return its atomic number Z.
    if (pid > 1000000000) return (pid % 10000000)/10000;
    // Otherwise, use the lookup table to determine the charge
    else return particle_electric_charges.at(pid);
  }

  // Lookup table for element symbols (keys are atomic numbers Z,
  // values are symbols on the periodic table). The symbol "Nn" is
  // used for a neutron to match the ENSDF convention.
  extern const std::unordered_map<int, std::string> element_symbols;

  // Lookup table for atomic numbers (keys are symbols on the periodic table,
  // values are atomic numbers Z). The symbol "Nn" is used for a neutron to
  // match the ENSDF convention.
  extern const std::unordered_map<std::string, int> atomic_numbers;

  extern const std::string marley_logo;

  extern const std::string marley_pic;
}
