/*
 Copyright (C) 2015 - 2018 by the authors of the ASPECT code.

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

#ifndef _aspect_particle_integrator_rk_4_h
#define _aspect_particle_integrator_rk_4_h

#include <aspect/particle/integrator/interface.h>

namespace aspect
{
  namespace Particle
  {
    namespace Integrator
    {
      /**
       * Runge Kutta fourth order integrator. This scheme requires
       * storing the original location and intermediate k1, k2, k3, k4
       * values, so the read/write_data functions reflect this.
       *
       * @ingroup ParticleIntegrators
       */
      template <int dim>
      class RK4 : public Interface<dim>
      {
        public:
          RK4();

          /**
           * Perform an integration step of moving the particles of one cell
           * by the specified timestep dt. This class implements a Runge-
           * Kutta integration scheme that is fourth order accurate
           * in space.
           *
           * @param [in] begin_particle An iterator to the first particle to be moved.
           * @param [in] end_particle An iterator to the last particle to be moved.
           * @param [in] old_velocities The velocities at t_n, i.e. before the
           * particle movement, for all particles between @p begin_particle
           * and @p end_particle at their current position.
           * @param [in] velocities The velocities at the particle positions
           * at t_{n+1}, i.e. after the particle movement. Note that this is
           * the velocity at the old positions, but at the new time. It is the
           * responsibility of this function to compute the new location of
           * the particles.
           * @param [in] dt The length of the integration timestep.
           */
          virtual
          void
          local_integrate_step(const typename ParticleHandler<dim>::particle_iterator &begin_particle,
                               const typename ParticleHandler<dim>::particle_iterator &end_particle,
                               const std::vector<Tensor<1,dim> > &old_velocities,
                               const std::vector<Tensor<1,dim> > &velocities,
                               const double dt);

          /**
           * This function is called at the end of every integration step.
           * For the current class it will either increment the step variable
           * and return true to request another integration step, or reset the
           * step variable to 0 and return false if we are already in step 4.
           *
           * @return This function returns true if the integrator requires
           * another integration step. The particle integration will continue
           * to start new integration steps until this function returns false.
           */
          virtual bool new_integration_step();

          /**
           * Return data length of the integration related data required for
           * communication in terms of number of bytes. When data about
           * particles is transported from one processor to another, or stored
           * on disk for snapshots, integrators get the chance to store
           * whatever information they need with each particle. This function
           * returns how many pieces of additional information a concrete
           * integrator class needs to store for each particle.
           *
           * @return The number of bytes required to store the relevant
           * integrator data for one particle.
           */
          virtual std::size_t get_data_size() const;

          /**
           * @copydoc Interface::read_data()
           */
          virtual
          const void *
          read_data(const typename ParticleHandler<dim>::particle_iterator &particle,
                    const void *data);

          /**
           * @copydoc Interface::write_data()
           */
          virtual
          void *
          write_data(const typename ParticleHandler<dim>::particle_iterator &particle,
                     void *data) const;

        private:
          /**
           * The current integration step, i.e. for RK4 a number between 0
           * and 3.
           */
          unsigned int integrator_substep;

          /**
           * The particle location before the first integration step. This is
           * used in the following steps and transferred to another process if
           * the particle leaves the domain during one of the steps.
           */
          std::map<types::particle_index, Point<dim> > loc0;

          /**
           * The intermediate values of the RK4 scheme. These are
           * used in the following steps and transferred to another process if
           * the particle leaves the domain during one of the steps.
           */
          std::map<types::particle_index, Tensor<1,dim> > k1, k2, k3;

      };
    }
  }
}

#endif
