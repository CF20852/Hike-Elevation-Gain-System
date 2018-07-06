Autodesk Eagle 9.0.1 (free version) files for a Feather daughterboard for an Adafruit Feather M0 Adalogger.

There is at least one error in this design:  the GPIOSDA and GPIOSCL lines coming from the Adafruit Feather do not have pullup resistors, and because these lines are open-drain, the daughterboard must provide the pullup resistors.
