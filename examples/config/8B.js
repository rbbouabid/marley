// Use this example job configuration file as a starting point for your own
// files.
{
  seed: 123456, // Random number seed (omit to use time since Unix epoch)

  // Pure 76Ge target
  target: {
    nuclides: [ 1000180400 ], //40Ar
    atom_fractions: [ 1.0 ],
  },

  // Simulate CC dm scattering on 40Ar 
  reactions: [ "ve40ArCC_Bhattacharya2009.react" ],
  log: [ { file: "stdout", level: "info" } ],

  // Neutrino source specification
    source: {
      type: "histogram",
      neutrino: "ve",
      E_bin_lefts: [ 0.0, 0.16, 0.32, 0.48, 0.64, 0.8, 0.96, 1.12, 1.28, 1.44, 1.6, 1.76, 1.92, 2.08, 2.24, 2.4, 2.56, 2.72, 2.88, 3.04, 3.2, 3.36, 3.52, 3.68, 3.84, 4.0, 4.16, 4.32, 4.48, 4.64, 4.8, 4.96, 5.12, 5.28, 5.44, 5.6, 5.76, 5.92, 6.08, 6.24, 6.4, 6.56, 6.72, 6.88, 7.04, 7.2, 7.36, 7.52, 7.68, 7.84, 8.0, 8.16, 8.32, 8.48, 8.64, 8.8, 8.96, 9.12, 9.28, 9.44, 9.6, 9.76, 9.92, 10.08, 10.24, 10.4, 10.56, 10.72, 10.88, 11.04, 11.2, 11.36, 11.52, 11.68, 11.84, 12.0, 12.16, 12.32, 12.48, 12.64, 12.8, 12.96, 13.12, 13.28, 13.44, 13.6, 13.76, 13.92, 14.08, 14.24, 14.4, 14.56, 14.72, 14.88, 15.04, 15.2, 15.36, 15.52, 15.68, 15.84, 16.0, 16.16, 16.32, 16.48 ],   // Low edges of energy bins (MeV)
      weights: [ 0.0, 0.0004912, 0.0016919, 0.0035058, 0.0059024, 0.0088044, 0.0121275, 0.0158284, 0.0198625, 0.0241649, 0.0286963, 0.033411, 0.0382645, 0.0432267, 0.0482606, 0.0533315, 0.0584143, 0.0634753, 0.0684884, 0.0734348, 0.0782819, 0.0830112, 0.0876036, 0.0920371, 0.0962999, 0.1003738, 0.1042435, 0.1079014, 0.1113299, 0.1145204, 0.1174686, 0.1201595, 0.1225902, 0.1247557, 0.1266475, 0.1282649, 0.1296032, 0.1306601, 0.1314402, 0.1319392, 0.1321578, 0.132104, 0.1317746, 0.1311762, 0.1303166, 0.1291959, 0.1278246, 0.1262088, 0.1243531, 0.1222707, 0.1199684, 0.1174543, 0.1147438, 0.1118432, 0.1087639, 0.105525, 0.1021309, 0.0985961, 0.094941, 0.0911702, 0.0873032, 0.0833563, 0.0793387, 0.0752712, 0.0711678, 0.0670409, 0.0629135, 0.0587968, 0.0547056, 0.0506645, 0.0466814, 0.0427724, 0.0389637, 0.035259, 0.0316751, 0.0282403, 0.0249523, 0.0218312, 0.018899, 0.016153, 0.0136139, 0.0112952, 0.0091913, 0.0073246, 0.0056968, 0.0042962, 0.0031466, 0.002226, 0.0015101, 0.0010006, 0.0006401, 0.0003948, 0.0002433, 0.0001444, 8.22e-05, 4.66e-05, 2.46e-05, 1.21e-05, 5.8e-06, 2.4e-06, 9e-07, 3e-07, 1e-07, 0.0 ],       // Bin weights (dimensionless)
      Emax: 40.,                        // Upper edge of the final bin (MeV)
    },

  // Incident neutrino direction 3-vector
  direction: { x: 0.0, y: 0.0, z: 1.0 },

  // Settings for marley command-line executable
  executable_settings: {

    // The number of events to generate
    events: 1,

    // Event output configuration
     output: [ { file: "scripted/solarnu.ascii", format: "ascii", mode: "overwrite" } ],

  },
}
