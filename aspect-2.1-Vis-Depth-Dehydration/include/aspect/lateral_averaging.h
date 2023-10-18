/*
  Copyright (C) 2011 - 2018 by the authors of the ASPECT code.

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


#ifndef _aspect_lateral_averaging_h
#define _aspect_lateral_averaging_h

#include <aspect/simulator_access.h>

#include <deal.II/fe/fe_values.h>

namespace aspect
{
  using namespace dealii;

  namespace internal
  {
    /**
     * This is the base class for all the functors implemented. Each is
     * used to compute one of the properties that will be laterally
     * averaged. The point of the base class is to allow handing over
     * a variable of type
     * <code> std::vector<std::unique_ptr<FunctorBase<dim> > > </code> to the
     * LateralAveraging::get_averages() function.
     */
    template <int dim>
    class FunctorBase
    {
      public:
        /**
         * operator() will have @p in and @p out filled out if @p true. By default
         * returns false.
         */
        virtual
        bool
        need_material_properties() const;

        /**
         * If this functor needs additional material model outputs create them
         * in here. By default, this does nothing.
         */
        virtual
        void
        create_additional_material_model_outputs (const unsigned int n_points,
                                                  MaterialModel::MaterialModelOutputs<dim> &outputs) const;

        /**
         * Called once at the beginning of compute_lateral_averages() to setup
         * internal data structures with the number of quadrature points.
         */
        virtual
        void
        setup(const unsigned int q_points);

        /**
         * This takes @p in material model inputs and @p out outputs (which are filled
         * if need_material_properties() == true), an initialized FEValues
         * object for a cell, and the current solution vector as inputs.
         * Functions in derived classes should then evaluate the desired quantity
         * and return the results in the output vector, which is q_points long.
         */
        virtual
        void
        operator()(const MaterialModel::MaterialModelInputs<dim> &in,
                   const MaterialModel::MaterialModelOutputs<dim> &out,
                   const FEValues<dim> &fe_values,
                   const LinearAlgebra::BlockVector &solution,
                   std::vector<double> &output) = 0;

        /**
         * Provide an (empty) virtual destructor.
         */
        virtual
        ~FunctorBase();
    };
  }

  /**
   * LateralAveraging is a class that performs various averaging operations
   * on the solution.  The functions of this class take a vector as an argument.
   * The model is divided into depth slices where the number of slices is the
   * length of the output vector. Each function averages a specific quantity
   * (as specified by their name), and that quantity is averaged laterally
   * for each depth slice.

   * Plugins may access the LateralAveraging plugin through the SimulatorAccess
   * function get_lateral_averaging(), and then query that for the desired
   * averaged quantity.
   *
   * @ingroup Simulator
   */
  template <int dim>
  class LateralAveraging : public SimulatorAccess<dim>
  {
    public:
      /**
       * Fill the @p values with a set of lateral averages of the selected
       * @p property_names. See the implementation of this function for
       * a range of accepted names. This function is more efficient than
       * calling multiple of the other functions that compute one property
       * each.
       *
       * @param n_slices The number of depth slices to perform the averaging in.
       * @return The output vector of laterally averaged values. Each vector
       * has the same size of @p n_slices, and there are
       * as many vectors returned as names in @p property_names.
       * @param property_names Names of the available quantities to average.
       * Check the implementation of this function for available names.
       */
      std::vector<std::vector<double> >
      get_averages(const unsigned int n_slices,
                   const std::vector<std::string> &property_names) const;

      /**
       * Fill the argument with a set of lateral averages of the current
       * temperature field. The function fills a vector that contains average
       * field values over slices of the domain of same depth.
       *
       * @param values The output vector of laterally averaged values. The
       * function takes the pre-existing size of this vector as the number of
       * depth slices.
       */
      void
      get_temperature_averages(std::vector<double> &values) const;

      /**
       * Fill the argument with a set of lateral averages of the current
       * compositional fields.
       *
       * @param composition_index The index of the compositional field whose
       * matrix we want to assemble (0 <= composition_index < number of
       * compositional fields in this problem).
       *
       * @param values The output vector of laterally averaged values. The
       * function takes the pre-existing size of this vector as the number of
       * depth slices.
       */
      void
      get_composition_averages(const unsigned int composition_index,
                               std::vector<double> &values) const;

      /**
       * Compute a lateral average of the current viscosity.
       *
       * @param values The output vector of laterally averaged values. The
       * function takes the pre-existing size of this vector as the number of
       * depth slices.
       */
      void
      get_viscosity_averages(std::vector<double> &values) const;

      /**
       * Compute a lateral average of the current velocity magnitude.
       *
       * @param values The output vector of laterally averaged values. The
       * function takes the pre-existing size of this vector as the number of
       * depth slices.
       */
      void
      get_velocity_magnitude_averages(std::vector<double> &values) const;

      /**
       * Compute a lateral average of the current sinking velocity.
       *
       * @param values The output vector of laterally averaged values. The
       * function takes the pre-existing size of this vector as the number of
       * depth slices.
       */
      void
      get_sinking_velocity_averages(std::vector<double> &values) const;

      /**
       * Compute a lateral average of the seismic shear wave speed: Vs.
       *
       * @param values The output vector of laterally averaged values. The
       * function takes the pre-existing size of this vector as the number of
       * depth slices.
       */
      void
      get_Vs_averages(std::vector<double> &values) const;

      /**
       * Compute a lateral average of the seismic pressure wave speed: Vp.
       *
       * @param values The output vector of laterally averaged values. The
       * function takes the pre-existing size of this vector as the number of
       * depth slices.
       */
      void
      get_Vp_averages(std::vector<double> &values) const;

      /**
       * Compute a lateral average of the heat flux, with the sign
       * convention of positive heat flux when it flows upwards.
       *
       * @param values The output vector of laterally averaged values. The
       * function takes the pre-existing size of this vector as the number of
       * depth slices.
       */
      void
      get_vertical_heat_flux_averages(std::vector<double> &values) const;

    private:
      /**
       * Internal routine to compute the depth averages of several quantities.
       * All of the public functions that compute a single field also call this
       * function. The vector of functors @p functors must contain one or more
       * objects of classes that are derived from FunctorBase and are used to
       * fill the values vectors.
       *
       * @param n_slices Number of depth slices to be computed.
       * @param functors Instances of a class derived from FunctorBase
       * that are used to compute the averaged properties.
       * @return The output vectors of depth averaged values. The
       * function returns one vector of doubles per property and uses
       * @p n_slices as the number of depth slices.
       * Each returned vector has the same size.
       */
      std::vector<std::vector<double> >
      compute_lateral_averages(const unsigned int n_slices,
                               std::vector<std::unique_ptr<internal::FunctorBase<dim> > > &functors) const;
  };
}


#endif
