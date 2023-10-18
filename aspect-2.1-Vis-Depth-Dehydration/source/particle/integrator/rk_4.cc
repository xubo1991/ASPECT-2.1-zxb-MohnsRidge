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

#include <aspect/particle/integrator/rk_4.h>

namespace aspect
{
  namespace Particle
  {
    namespace Integrator
    {
      template <int dim>
      RK4<dim>::RK4()
        :
        integrator_substep(0)
      {}

      template <int dim>
      void
      RK4<dim>::local_integrate_step(const typename ParticleHandler<dim>::particle_iterator &begin_particle,
                                     const typename ParticleHandler<dim>::particle_iterator &end_particle,
                                     const std::vector<Tensor<1,dim> > &old_velocities,
                                     const std::vector<Tensor<1,dim> > &velocities,
                                     const double dt)
      {
        Assert(static_cast<unsigned int> (std::distance(begin_particle, end_particle)) == old_velocities.size(),
               ExcMessage("The particle integrator expects the old velocity vector to be of equal size "
                          "to the number of particles to advect. For some unknown reason they are different, "
                          "most likely something went wrong in the calling function."));

        Assert(old_velocities.size() == velocities.size(),
               ExcMessage("The particle integrator expects the velocity vector to be of equal size "
                          "to the number of particles to advect. For some unknown reason they are different, "
                          "most likely something went wrong in the calling function."));

        // TODO: currently old_velocity is not used in this scheme, but it should,
        // to make it at least second-order accurate in time.
        typename std::vector<Tensor<1,dim> >::const_iterator old_velocity = old_velocities.begin();
        typename std::vector<Tensor<1,dim> >::const_iterator velocity = velocities.begin();

        for (typename ParticleHandler<dim>::particle_iterator it = begin_particle;
             it != end_particle; ++it, ++velocity, ++old_velocity)
          {
            const types::particle_index particle_id = it->get_id();
            if (integrator_substep == 0)
              {
                loc0[particle_id] = it->get_location();
                k1[particle_id] = dt * (*old_velocity);
                it->set_location(it->get_location() + 0.5*k1[particle_id]);
              }
            else if (integrator_substep == 1)
              {
                k2[particle_id] = dt * (*old_velocity + *velocity) / 2.0;
                it->set_location(loc0[particle_id] + 0.5*k2[particle_id]);
              }
            else if (integrator_substep == 2)
              {
                k3[particle_id] = dt * (*old_velocity + *velocity) / 2.0;
                it->set_location(loc0[particle_id] + k3[particle_id]);
              }
            else if (integrator_substep == 3)
              {
                const Tensor<1,dim> k4 = dt * (*velocity);
                it->set_location(loc0[particle_id] + (k1[particle_id] + 2.0*k2[particle_id] + 2.0*k3[particle_id] + k4)/6.0);
              }
            else
              {
                Assert(false,
                       ExcMessage("The RK4 integrator should never continue after four integration steps."));
              }
          }
      }

      template <int dim>
      bool
      RK4<dim>::new_integration_step()
      {
        if (integrator_substep == 3)
          {
            loc0.clear();
            k1.clear();
            k2.clear();
            k3.clear();
          }

        integrator_substep = (integrator_substep+1)%4;

        // Continue until we're at the last step
        return (integrator_substep != 0);
      }

      template <int dim>
      std::size_t
      RK4<dim>::get_data_size() const
      {
        // If integration is finished, we do not need to transfer integrator
        // data to other processors, because it will be deleted soon anyway.
        // Skip the MPI transfer in this case.
        if (integrator_substep == 3)
          return 0;

        return 4*dim*sizeof(double);
      }

      template <int dim>
      const void *
      RK4<dim>::read_data(const typename ParticleHandler<dim>::particle_iterator &particle,
                          const void *data)
      {
        // If integration is finished, we do not need to transfer integrator
        // data to other processors, because it will be deleted soon anyway.
        // Skip the MPI transfer in this case.
        if (integrator_substep == 3)
          return data;

        const double *integrator_data = static_cast<const double *> (data);

        // Read location data
        for (unsigned int i=0; i<dim; ++i)
          loc0[particle->get_id()](i) = *integrator_data++;

        // Read k1, k2 and k3
        for (unsigned int i=0; i<dim; ++i)
          k1[particle->get_id()][i] = *integrator_data++;

        for (unsigned int i=0; i<dim; ++i)
          k2[particle->get_id()][i] = *integrator_data++;

        for (unsigned int i=0; i<dim; ++i)
          k3[particle->get_id()][i] = *integrator_data++;

        return static_cast<const void *> (integrator_data);
      }

      template <int dim>
      void *
      RK4<dim>::write_data(const typename ParticleHandler<dim>::particle_iterator &particle,
                           void *data) const
      {
        // If integration is finished, we do not need to transfer integrator
        // data to other processors, because it will be deleted soon anyway.
        // Skip the MPI transfer in this case.
        if (integrator_substep == 3)
          return data;

        double *integrator_data = static_cast<double *> (data);

        // Write location data
        typename std::map<types::particle_index, Point<dim> >::const_iterator it = loc0.find(particle->get_id());
        for (unsigned int i=0; i<dim; ++i,++integrator_data)
          *integrator_data = it->second(i);

        // Write k1, k2 and k3
        typename std::map<types::particle_index, Tensor<1,dim> >::const_iterator it_k = k1.find(particle->get_id());
        for (unsigned int i=0; i<dim; ++i,++integrator_data)
          *integrator_data = it_k->second[i];

        it_k = k2.find(particle->get_id());
        for (unsigned int i=0; i<dim; ++i,++integrator_data)
          *integrator_data = it_k->second[i];

        it_k = k3.find(particle->get_id());
        for (unsigned int i=0; i<dim; ++i,++integrator_data)
          *integrator_data = it_k->second[i];

        return static_cast<void *> (integrator_data);
      }
    }
  }
}


// explicit instantiations
namespace aspect
{
  namespace Particle
  {
    namespace Integrator
    {
      ASPECT_REGISTER_PARTICLE_INTEGRATOR(RK4,
                                          "rk4",
                                          "Runge Kutta fourth order integrator, where "
                                          "$y_{n+1} = y_n + \\frac{1}{6} k_1 + \\frac{1}{3} k_2 "
                                          "+ \\frac{1}{3} k_3 + \\frac{1}{6} k_4$ "
                                          "and $k_1$, $k_2$, $k_3$, $k_4$ are defined as usual.")
    }
  }
}
