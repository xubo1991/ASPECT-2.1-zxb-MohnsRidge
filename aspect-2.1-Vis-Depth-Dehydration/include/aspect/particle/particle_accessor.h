/*
 Copyright (C) 2017 - 2019 by the authors of the ASPECT code.

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

#ifndef _aspect_particle_particle_accessor_h
#define _aspect_particle_particle_accessor_h

#include <aspect/global.h>
#include <aspect/particle/particle.h>

#include <deal.II/base/array_view.h>
#include <deal.II/distributed/tria.h>

#if !DEAL_II_VERSION_GTE(9,0,0)

namespace aspect
{
  namespace Particle
  {
    using namespace dealii;

    template <int, int> class ParticleIterator;
    template <int, int> class ParticleHandler;

    template<int dim, int spacedim=dim>
    class ParticleAccessor
    {
      public:
        /**
         * Write particle data into a data array. The array is expected
         * to be large enough to take the data, and the void pointer should
         * point to the first element in which the data should be written. This
         * function is meant for serializing all particle properties and
         * afterwards de-serializing the properties by calling the appropriate
         * constructor Particle(void *&data, PropertyPool *property_pool = nullptr);
         *
         * @param [in,out] data The memory location to write particle data
         * into. This pointer points to the begin of the memory, in which the
         * data will be written and it will be advanced by the serialized size
         * of this particle.
         */
        void
        write_data(void *&data) const;

        /**
         * Set the location of this particle. Note that this does not check
         * whether this is a valid location in the simulation domain.
         *
         * @param [in] new_location The new location for this particle.
         */
        void
        set_location (const Point<spacedim> &new_location);

        /**
         * Get the location of this particle.
         *
         * @return The location of this particle.
         */
        const Point<spacedim> &
        get_location () const;

        /**
         * Set the reference location of this particle.
         *
         * @param [in] new_reference_location The new reference location for
         * this particle.
         */
        void
        set_reference_location (const Point<dim> &new_reference_location);

        /**
         * Return the reference location of this particle in its current cell.
         */
        const Point<dim> &
        get_reference_location () const;

        /**
         * Return the ID number of this particle.
         */
        types::particle_index
        get_id () const;

        /**
         * Tell the particle where to store its properties (even if it does not
         * own properties). Usually this is only done once per particle, but
         * since the particle generator does not know about the properties
         * we want to do it not at construction time. Another use for this
         * function is after particle transfer to a new process.
         */
        void
        set_property_pool(PropertyPool &property_pool);

        /**
         * Returns whether this particle has a valid property pool and a valid
         * handle to properties.
         */
        bool
        has_properties () const;

        /**
         * Set the properties of this particle.
         *
         * @param [in] new_properties A vector containing the new properties
         * for this particle.
         */
        void
        set_properties (const std::vector<double> &new_properties);

        /**
         * Get write-access to properties of this particle.
         *
         * @return An ArrayView of the properties of this particle.
         */
        const ArrayView<double>
        get_properties ();

        /**
         * Get read-access to properties of this particle.
         *
         * @return An ArrayView of the properties of this particle.
         */
        const ArrayView<const double>
        get_properties () const;

        /**
         * Returns the size in bytes this particle occupies if all of its data is
         * serialized (i.e. the number of bytes that is written by the write_data
         * function of this class).
         */
        std::size_t
        serialized_size_in_bytes() const;

        /**
         * Get a cell iterator to the cell surrounding the current particle.
         * As particles are organized in the structure of a triangulation,
         * but the triangulation itself is not stored in the particle this
         * operation requires a reference to the triangulation.
         */
        typename parallel::distributed::Triangulation<dim,spacedim>::cell_iterator
        get_surrounding_cell (const parallel::distributed::Triangulation<dim,spacedim> &triangulation) const;

        /**
         * Serialize the contents of this class.
         */
        template <class Archive>
        void serialize (Archive &ar, const unsigned int version);

        /**
         * Advance the ParticleAccessor to the next particle.
         */
        void next ();

        /**
         * Move the ParticleAccessor to the previous particle.
         */
        void prev ();

        /**
         * Inequality operator.
         */
        bool operator != (const ParticleAccessor<dim,spacedim> &other) const;

        /**
         * Equality operator.
         */
        bool operator == (const ParticleAccessor<dim,spacedim> &other) const;

      protected:
        /**
         * Construct an invalid accessor. Such an object is not usable.
         */
        ParticleAccessor ();

        /**
         * Construct an accessor from a reference to a map and an iterator to the map.
         * This constructor is protected so that it can only be accessed by friend
         * classes.
         */
        ParticleAccessor (const std::multimap<Particles::internal::LevelInd, Particle<dim,spacedim> > &map,
                          const typename std::multimap<Particles::internal::LevelInd, Particle<dim,spacedim> >::iterator &particle);

      private:
        /**
         * A pointer to the container that stores the particles. Obviously,
         * this accessor is invalidated if the container changes.
         */
        std::multimap<Particles::internal::LevelInd, Particle<dim,spacedim> >            *map;

        /**
         * An iterator into the container of particles. Obviously, this
         * accessor is invalidated if the container changes.
         */
        typename std::multimap<Particles::internal::LevelInd, Particle<dim,spacedim> >::iterator  particle;

        /**
         * Make ParticleIterator a friend to allow it constructing ParticleAccessors.
         */
        template <int, int> friend class ParticleIterator;
        template <int, int> friend class ParticleHandler;
    };
  }
}

#endif
#endif
