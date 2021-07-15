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

  // Neutrino source specification
    source: {
      type: "monoenergetic",
      neutrino: "dm",
      energy: 10000.0,        // Neutrino energy (MeV)
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
