/*
  Copyright (C) 2011 - 2019 by the authors of the ASPECT code.

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


#include <aspect/postprocess/visualization/thermal_diffusivity.h>
#include <aspect/adiabatic_conditions/interface.h>



namespace aspect
{
  namespace Postprocess
  {
    namespace VisualizationPostprocessors
    {
      template <int dim>
      ThermalDiffusivity<dim>::
      ThermalDiffusivity ()
        :
        DataPostprocessorScalar<dim> ("thermal_diffusivity",
                                      update_values | update_quadrature_points )
      {}



      template <int dim>
      void
      ThermalDiffusivity<dim>::
      evaluate_vector_field(const DataPostprocessorInputs::Vector<dim> &input_data,
                            std::vector<Vector<double> > &computed_quantities) const
      {
        const unsigned int n_quadrature_points = input_data.solution_values.size();
        Assert (computed_quantities.size() == n_quadrature_points,    ExcInternalError());
        Assert (computed_quantities[0].size() == 1,                   ExcInternalError());
        Assert (input_data.solution_values[0].size() == this->introspection().n_components,           ExcInternalError());

        // Set use_strain_rates to false since we have no need for viscosity.
        MaterialModel::MaterialModelInputs<dim> in(input_data,
                                                   this->introspection(), false);
        MaterialModel::MaterialModelOutputs<dim> out(n_quadrature_points,
                                                     this->n_compositional_fields());

        this->get_material_model().evaluate(in, out);

        for (unsigned int q=0; q<n_quadrature_points; ++q)

          if (this->get_parameters().formulation_temperature_equation ==
              Parameters<dim>::Formulation::TemperatureEquation::reference_density_profile)
            computed_quantities[q](0) = out.thermal_conductivities[q] /
                                        (this->get_adiabatic_conditions().density(in.position[q]) * out.specific_heat[q]);
          else
            computed_quantities[q](0) = out.thermal_conductivities[q] / (out.densities[q] * out.specific_heat[q]);

      }
    }
  }
}


// explicit instantiations
namespace aspect
{
  namespace Postprocess
  {
    namespace VisualizationPostprocessors
    {
      ASPECT_REGISTER_VISUALIZATION_POSTPROCESSOR(ThermalDiffusivity,
                                                  "thermal diffusivity",
                                                  "A visualization output object that generates output "
                                                  "for the thermal diffusivity $\\kappa$=$\\frac{k}{\\rho C_p}$, "
                                                  "with $k$ the thermal conductivity.")
    }
  }
}
