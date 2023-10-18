/*
 Copyright (C) 2015 - 2019 by the authors of the ASPECT code.

 This file is part of ASPECT.

 ASPECT is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
 any later version.

 ASPECT is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with ASPECT; see the file LICENSE.  If not see
 <http://www.gnu.org/licenses/>.
 */

#ifndef _aspect_particle_output_ascii_h
#define _aspect_particle_output_ascii_h

#include <aspect/particle/output/interface.h>
#include <aspect/simulator_access.h>

#if !DEAL_II_VERSION_GTE(9,0,0)

namespace aspect
{
  namespace Particle
  {
    namespace Output
    {
      /**
       * Class that outputs particles and their properties in a simple text
       * format.
       *
       * @ingroup ParticleOutput
       */
      template <int dim>
      class ASCIIOutput : public Interface<dim>,
        public SimulatorAccess<dim>
      {
        public:
          /**
           * Constructor.
           */
          ASCIIOutput();

          /**
           * Initialization function. This function is called once at the
           * beginning of the program after parse_parameters is run and after the
           * SimulatorAccess (if applicable) is initialized.
           */
          virtual
          void
          initialize ();

          /**
           * Write data about the particles specified in the first argument
           * to a file. If possible, encode the current simulation time
           * into this file using the data provided in the last argument.
           *
           * @param[in] particle_handler The particle handler that allows access
           * to the collection of particles.
           *
           * @param [in] property_information Information object containing names and number
           * of components of each property.
           *
           * @param[in] current_time Current time of the simulation, given as either
           * years or seconds, as selected in the input file. In other words,
           * output writers do not need to know the units in which time is
           * described.
           *
           * @return The name of the file that was written, or any other
           * information that describes what output was produced if for example
           * multiple files were created.
           */
          virtual
          std::string
          output_particle_data(const ParticleHandler<dim> &particle_handler,
                               const Property::ParticlePropertyInformation &property_information,
                               const double current_time);

          /**
           * Read or write the data of this object for serialization
           */
          template <class Archive>
          void serialize(Archive &ar, const unsigned int version);

          /**
           * Save the state of the object.
           */
          virtual
          void
          save (std::ostringstream &os) const;

          /**
           * Restore the state of the object.
           */
          virtual
          void
          load (std::istringstream &is);

        private:
          /**
           * Internal index of file output number.
           */
          unsigned int    file_index;
      };
    }
  }
}

#endif
#endif
