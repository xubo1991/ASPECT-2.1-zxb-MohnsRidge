/*
  Copyright (C) 2013 - 2018 by the authors of the ASPECT code.

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

#ifndef _aspect_material_model_latent_heat_h
#define _aspect_material_model_latent_heat_h

#include <aspect/material_model/interface.h>
#include <aspect/simulator_access.h>

namespace aspect
{
  namespace MaterialModel
  {
    using namespace dealii;

    /**
     * A material model that implements a standard approximation of the latent
     * heat terms following Christensen \& Yuen, 1986. The change of entropy
     * is calculated as $Delta S = \gamma \frac{\Delta\rho}{\rho^2}$ with the
     * Clapeyron slope $\gamma$ and the density change $\Delta\rho$ of the
     * phase transition being input parameters. This model employs an analytic
     * phase function in the form $X=\frac{1}{2} \left( 1 + \tanh \left( \frac{\Delta
     * p}{\Delta p_0} \right) \right)$ with $\Delta p = p - p_transition -
     * \gamma \left( T - T_transition \right)$ and $\Delta p_0$ being the
     * pressure difference over the width of the phase transition (specified
     * as input parameter).
     *
     * @ingroup MaterialModels
     */
    template <int dim>
    class LatentHeat : public MaterialModel::Interface<dim>, public ::aspect::SimulatorAccess<dim>
    {
      public:

        /**
         * Evaluate material properties.
         */
        virtual void evaluate(const MaterialModelInputs<dim> &in,
                              MaterialModelOutputs<dim> &out) const;

        /**
         * @name Qualitative properties one can ask a material model
         * @{
         */

        /**
         * Return whether the model is compressible or not.  Incompressibility
         * does not necessarily imply that the density is constant; rather, it
         * may still depend on temperature or pressure. In the current
         * context, compressibility means whether we should solve the continuity
         * equation as $\nabla \cdot (\rho \mathbf u)=0$ (compressible Stokes)
         * or as $\nabla \cdot \mathbf{u}=0$ (incompressible Stokes).
         */
        virtual bool is_compressible () const;
        /**
         * @}
         */

        /**
         * @name Reference quantities
         * @{
         */
        virtual double reference_viscosity () const;
        /**
         * @}
         */


        /**
         * @name Functions used in dealing with run-time parameters
         * @{
         */
        /**
         * Declare the parameters this class takes through input files.
         */
        static
        void
        declare_parameters (ParameterHandler &prm);

        /**
         * Read the parameters this class declares from the parameter file.
         */
        virtual
        void
        parse_parameters (ParameterHandler &prm);
        /**
         * @}
         */

      private:
        bool use_depth;
        double reference_rho;
        double reference_T;
        double eta;
        double composition_viscosity_prefactor;
        double thermal_viscosity_exponent;
        double thermal_alpha;
        double reference_specific_heat;
        double reference_compressibility;
        double max_viscosity;
        double min_viscosity;

        /**
         * The thermal conductivity.
         */
        double k_value;

        double compositional_delta_rho;

        /**
         * Percentage of material that has already undergone the phase
         * transition to the higher-pressure material (this is done
         * individually for each transition and summed up in the end)
         */
        virtual
        double
        phase_function (const Point<dim> &position,
                        const double temperature,
                        const double pressure,
                        const int phase) const;

        /**
         * Derivative of the phase function (argument is the pressure
         * deviation).
         */
        virtual
        double
        phase_function_derivative (const Point<dim> &position,
                                   const double temperature,
                                   const double pressure,
                                   const int phase) const;

        // list of depth (or pressure), width and Clapeyron slopes
        // for the different phase transitions
        std::vector<double> transition_depths;
        std::vector<double> transition_pressures;
        std::vector<double> transition_temperatures;
        std::vector<double> transition_widths;
        std::vector<double> transition_pressure_widths;
        std::vector<double> transition_slopes;
        std::vector<double> density_jumps;
        std::vector<int> transition_phases;
        std::vector<double> phase_prefactors;
    };

  }
}

#endif
