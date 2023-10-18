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


#include <aspect/postprocess/visualization/shear_stress.h>



namespace aspect
{
  namespace Postprocess
  {
    namespace VisualizationPostprocessors
    {
      template <int dim>
      void
      ShearStress<dim>::
      evaluate_vector_field(const DataPostprocessorInputs::Vector<dim> &input_data,
                            std::vector<Vector<double> > &computed_quantities) const
      {
        const unsigned int n_quadrature_points = input_data.solution_values.size();
        Assert (computed_quantities.size() == n_quadrature_points,    ExcInternalError());
        Assert ((computed_quantities[0].size() == SymmetricTensor<2,dim>::n_independent_components),
                ExcInternalError());
        Assert (input_data.solution_values[0].size() == this->introspection().n_components,   ExcInternalError());
        Assert (input_data.solution_gradients[0].size() == this->introspection().n_components,  ExcInternalError());

        MaterialModel::MaterialModelInputs<dim> in(input_data,
                                                   this->introspection());
        MaterialModel::MaterialModelOutputs<dim> out(n_quadrature_points,
                                                     this->n_compositional_fields());

        // Compute the viscosity...
        this->get_material_model().evaluate(in, out);

        // ...and use it to compute the stresses
        for (unsigned int q=0; q<n_quadrature_points; ++q)
          {
            const SymmetricTensor<2,dim> strain_rate = in.strain_rate[q];
            const SymmetricTensor<2,dim> compressible_strain_rate
              = (this->get_material_model().is_compressible()
                 ?
                 strain_rate - 1./3 * trace(strain_rate) * unit_symmetric_tensor<dim>()
                 :
                 strain_rate);

            const double eta = out.viscosities[q];

            const SymmetricTensor<2,dim> shear_stress = 2*eta*compressible_strain_rate;
            for (unsigned int i=0; i<SymmetricTensor<2,dim>::n_independent_components; ++i)
              computed_quantities[q](i) = shear_stress[shear_stress.unrolled_to_component_indices(i)];
          }
      }


      template <int dim>
      std::vector<std::string>
      ShearStress<dim>::get_names () const
      {
        std::vector<std::string> names;
        switch (dim)
          {
            case 2:
              names.emplace_back("shear_stress_xx");
              names.emplace_back("shear_stress_yy");
              names.emplace_back("shear_stress_xy");
              break;

            case 3:
              names.emplace_back("shear_stress_xx");
              names.emplace_back("shear_stress_yy");
              names.emplace_back("shear_stress_zz");
              names.emplace_back("shear_stress_xy");
              names.emplace_back("shear_stress_xz");
              names.emplace_back("shear_stress_yz");
              break;

            default:
              Assert (false, ExcNotImplemented());
          }

        return names;
      }


      template <int dim>
      std::vector<DataComponentInterpretation::DataComponentInterpretation>
      ShearStress<dim>::get_data_component_interpretation () const
      {
        return
          std::vector<DataComponentInterpretation::DataComponentInterpretation>
          (SymmetricTensor<2,dim>::n_independent_components,
           DataComponentInterpretation::component_is_scalar);
      }



      template <int dim>
      UpdateFlags
      ShearStress<dim>::get_needed_update_flags () const
      {
        return update_gradients | update_values | update_quadrature_points;
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
      ASPECT_REGISTER_VISUALIZATION_POSTPROCESSOR(ShearStress,
                                                  "shear stress",
                                                  "A visualization output object that generates output "
                                                  "for the 3 (in 2d) or 6 (in 3d) components of the shear stress "
                                                  "tensor, i.e., for the components of the tensor "
                                                  "$2\\eta\\varepsilon(\\mathbf u)$ "
                                                  "in the incompressible case and "
                                                  "$2\\eta\\left[\\varepsilon(\\mathbf u)-"
                                                  "\\tfrac 13(\\textrm{tr}\\;\\varepsilon(\\mathbf u))\\mathbf I\\right]$ "
                                                  "in the compressible case. The shear "
                                                  "stress differs from the full stress tensor "
                                                  "by the absence of the pressure.")
    }
  }
}
