#pragma once
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>

#include "TMarleyDecayScheme.hh"

class TMarleyConfigFile {
  public:

    // Type used to organize nuclear structure file
    // load requests within this configuration file
    struct StructureRecord {
      std::string filename;
      TMarleyDecayScheme::FileFormat format;
      std::unordered_set<std::string> nucids;
    };

    TMarleyConfigFile();
    TMarleyConfigFile(const std::string file_name);
    inline uint_fast64_t get_seed() const { return seed; }
    inline void set_seed(uint_fast64_t s) { seed = s; };
    inline const std::unordered_set<std::string>&
      get_reaction_filenames() const
    {
      return reaction_filenames;
    }
    inline void add_reaction_filename(std::string rfile) {
      reaction_filenames.insert(rfile);
    }
    inline void remove_reaction_filename(std::string rfile) {
      reaction_filenames.erase(rfile);
    }
    inline void clear_reaction_filenames() {
      reaction_filenames.clear();
    }
    inline const std::vector<StructureRecord>& get_structure_records() const {
      return structure_records;
    }
    #ifdef USE_ROOT
    inline std::string get_root_filename() const { return root_filename; }
    inline bool check_write_root() const { return writeroot; }
    inline bool check_overwrite_root() const {
      return check_before_root_file_overwrite;
    }
    #endif
    void print_summary(std::ostream& os = std::cout);

    inline double get_contbin_width() const { return contbin_width; }
    inline size_t get_contbin_num_subs() const { return contbin_num_subs; }

    // Default number of subintervals to use when integrating Hauser-Feshbach
    // fragment and gamma decay widths over a bin in the energy continuum.
    // By default, We'll approximate the decay width using a single trapezoid
    // since the underlying approximation of using continuum bins is that the
    // decay width varies slowly over a bin.
    static constexpr int DEFAULT_CONTINUUM_BIN_SUBINTERVALS = 1;

    // Default energy resolution to use when binning the continuum of final
    // nuclear excitation energies for Hauser-Feshbach model calculations.
    static constexpr double DEFAULT_CONTINUUM_BIN_RESOLUTION = 0.1; // MeV

  private:

    std::string filename;
    uint_fast64_t seed;
    std::unordered_set<std::string> reaction_filenames;

    std::vector<StructureRecord> structure_records;

    double contbin_width;
    size_t contbin_num_subs;

    #ifdef USE_ROOT
    std::string root_filename;
    bool writeroot;
    bool check_before_root_file_overwrite;
    #endif

    // Matches comment lines and empty lines
    static const std::regex rx_comment_or_empty;
    // Matches non-negative integers
    static const std::regex rx_nonneg_int;
    // Matches all numbers, including floats
    static const std::regex rx_num;
    // Matches trimmed ENSDF-style nucids
    static const std::regex rx_nucid;

    // Convert a lowercase, trimmed string file format
    // description for the structure data to a
    // TMarleyDecayScheme::FileFormat
    TMarleyDecayScheme::FileFormat string_to_format(
      const std::string& string);

    // Convert a TMarleyDecayScheme::FileFormat value
    // to a std::string
    std::string format_to_string(const TMarleyDecayScheme::FileFormat ff);

    // Get the next word from a parsed line. If errors occur, complain.
    // The last argument determines whether the next word should be
    // converted to all lowercase or left as is.
    bool next_word_from_line(std::istringstream& iss, std::string& word,
      const std::string& keyword, const int line_number,
      bool enable_exceptions = true, bool make_lowercase = true);
};
