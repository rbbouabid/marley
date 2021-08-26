// Use this example job configuration file as a starting point for your own
// files.
{
  seed: 123456, // Random number seed (omit to use time since Unix epoch)

  // Pure 76Ge target
  target: {
    nuclides: [ 1000320760 ], //76Ge
    //nuclides: [ 1000541310 ],
    atom_fractions: [ 1.0 ],
  },

  // Simulate CC dm scattering on 76Ge
  reactions: [ "dm.react" ],
  log: [ { file: "stdout", level: "info" } ],

  // Neutrino source specification
    source: {
      type: "monoDM",
      neutrino: "dm",
      energy: 10000.0,        // Neutrino energy (MeV)
      mass: 1.1, // dark matter particle mass (MeV)
      velocity: 0.001, // dark matter particle velocity (m/s (i think) )
      LAMBDA: 1.2, // UV cutoff parameter ( I need to think more carefully about this param ) 
    },

  // Incident neutrino direction 3-vector
  direction: { x: 0.0, y: 0.0, z: 1.0 },

  // Settings for marley command-line executable
  executable_settings: {

    // The number of events to generate
    events: 1,

    // Event output configuration
    output: [ { file: "events.ascii", format: "ascii", mode: "overwrite" } ],

  },
}
