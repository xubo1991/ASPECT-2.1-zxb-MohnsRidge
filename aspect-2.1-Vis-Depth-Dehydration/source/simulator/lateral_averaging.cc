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

#include <aspect/lateral_averaging.h>
#include <aspect/material_model/interface.h>
#include <aspect/gravity_model/interface.h>
#include <aspect/geometry_model/interface.h>

#include <deal.II/fe/fe_values.h>
#include <deal.II/base/quadrature_lib.h>



namespace aspect
{
  /**
   * This namespace contains all the implemented functors. They are used to
   * compute various properties of the solution that will be laterally averaged.
   */
  namespace
  {
    template <int dim>
    class FunctorDepthAverageField: public internal::FunctorBase<dim>
    {
      public:
        FunctorDepthAverageField(const FEValuesExtractors::Scalar &field)
          : field_(field)
        {}

        void operator()(const MaterialModel::MaterialModelInputs<dim> &,
                        const MaterialModel::MaterialModelOutputs<dim> &,
                        const FEValues<dim> &fe_values,
                        const LinearAlgebra::BlockVector &solution,
                        std::vector<double> &output)
        {
          fe_values[field_].get_function_values (solution, output);
        }

        const FEValuesExtractors::Scalar field_;
    };



    template <int dim>
    class FunctorDepthAverageViscosity: public internal::FunctorBase<dim>
    {
      public:
        bool need_material_properties() const
        {
          return true;
        }

        void operator()(const MaterialModel::MaterialModelInputs<dim> &,
                        const MaterialModel::MaterialModelOutputs<dim> &out,
                        const FEValues<dim> &,
                        const LinearAlgebra::BlockVector &,
                        std::vector<double> &output)
        {
          output = out.viscosities;
        }
    };



    template <int dim>
    class FunctorDepthAverageVelocityMagnitude: public internal::FunctorBase<dim>
    {
      public:
        FunctorDepthAverageVelocityMagnitude(const FEValuesExtractors::Vector &field,
                                             bool convert_to_years)
          : field_(field), convert_to_years_(convert_to_years)
        {}

        void setup(const unsigned int q_points)
        {
          velocity_values.resize(q_points);
        }

        void operator()(const MaterialModel::MaterialModelInputs<dim> &,
                        const MaterialModel::MaterialModelOutputs<dim> &,
                        const FEValues<dim> &fe_values,
                        const LinearAlgebra::BlockVector &solution,
                        std::vector<double> &output)
        {
          fe_values[field_].get_function_values (solution, velocity_values);
          for (unsigned int q=0; q<output.size(); ++q)
            output[q] = std::sqrt( velocity_values[q] * velocity_values[q] ) *
                        (convert_to_years_ ? year_in_seconds : 1.0);
        }

        std::vector<Tensor<1,dim> > velocity_values;
        const FEValuesExtractors::Vector field_;
        const bool convert_to_years_;
    };



    template <int dim>
    class FunctorDepthAverageSinkingVelocity: public internal::FunctorBase<dim>
    {
      public:
        FunctorDepthAverageSinkingVelocity(const FEValuesExtractors::Vector &field,
                                           const GravityModel::Interface<dim> *gravity,
                                           bool convert_to_years)
          : field_(field),
            gravity_(gravity),
            convert_to_years_(convert_to_years)
        {}

        bool need_material_properties() const
        {
          // this is needed because we want to access in.position in operator()
          return true;
        }

        void setup(const unsigned int q_points)
        {
          velocity_values.resize(q_points);
        }

        void operator()(const MaterialModel::MaterialModelInputs<dim> &in,
                        const MaterialModel::MaterialModelOutputs<dim> &,
                        const FEValues<dim> &fe_values,
                        const LinearAlgebra::BlockVector &solution,
                        std::vector<double> &output)
        {
          fe_values[field_].get_function_values (solution, velocity_values);
          for (unsigned int q=0; q<output.size(); ++q)
            {
              const Tensor<1,dim> g = gravity_->gravity_vector(in.position[q]);
              const Tensor<1,dim> vertical = (g.norm() > 0 ? g/g.norm() : Tensor<1,dim>());

              output[q] = std::fabs(std::min(0.0, velocity_values[q] * vertical))
                          * (convert_to_years_ ? year_in_seconds : 1.0);
            }
        }

        std::vector<Tensor<1,dim> > velocity_values;
        const FEValuesExtractors::Vector field_;
        const GravityModel::Interface<dim> *gravity_;
        const bool convert_to_years_;
    };



    template <int dim>
    class FunctorDepthAverageVsVp: public internal::FunctorBase<dim>
    {
      public:
        FunctorDepthAverageVsVp(bool vs)
          : vs_(vs)
        {}

        bool need_material_properties() const
        {
          return true;
        }

        void
        create_additional_material_model_outputs (const unsigned int n_points,
                                                  MaterialModel::MaterialModelOutputs<dim> &outputs) const
        {
          outputs.additional_outputs.push_back(
            std::make_shared<MaterialModel::SeismicAdditionalOutputs<dim>> (n_points));
        }

        void operator()(const MaterialModel::MaterialModelInputs<dim> &,
                        const MaterialModel::MaterialModelOutputs<dim> &out,
                        const FEValues<dim> &,
                        const LinearAlgebra::BlockVector &,
                        std::vector<double> &output)
        {
          const MaterialModel::SeismicAdditionalOutputs<dim> *seismic_outputs
            = out.template get_additional_output<const MaterialModel::SeismicAdditionalOutputs<dim> >();

          Assert(seismic_outputs != nullptr,ExcInternalError());

          if (vs_)
            for (unsigned int q=0; q<output.size(); ++q)
              output[q] = seismic_outputs->vs[q];
          else
            for (unsigned int q=0; q<output.size(); ++q)
              output[q] = seismic_outputs->vp[q];
        }

        bool vs_;
    };



    template <int dim>
    class FunctorDepthAverageVerticalHeatFlux: public internal::FunctorBase<dim>
    {
      public:
        FunctorDepthAverageVerticalHeatFlux(const FEValuesExtractors::Vector &velocity_field,
                                            const FEValuesExtractors::Scalar &temperature_field,
                                            const GravityModel::Interface<dim> *gm)
          : velocity_field_(velocity_field),
            temperature_field_(temperature_field),
            gravity_model(gm)
        {}

        bool need_material_properties() const
        {
          return true;
        }

        void setup(const unsigned int q_points)
        {
          velocity_values.resize(q_points);
          temperature_values.resize(q_points);
          temperature_gradients.resize(q_points);
        }

        void operator()(const MaterialModel::MaterialModelInputs<dim> &in,
                        const MaterialModel::MaterialModelOutputs<dim> &out,
                        const FEValues<dim> &fe_values,
                        const LinearAlgebra::BlockVector &solution,
                        std::vector<double> &output)
        {
          fe_values[velocity_field_].get_function_values (solution, velocity_values);
          fe_values[temperature_field_].get_function_values (solution, temperature_values);
          fe_values[temperature_field_].get_function_gradients (solution, temperature_gradients);

          for (unsigned int q=0; q<output.size(); ++q)
            {
              const Tensor<1,dim> gravity = gravity_model->gravity_vector(in.position[q]);
              const Tensor<1,dim> vertical = -gravity/( gravity.norm() != 0.0 ?
                                                        gravity.norm() : 1.0 );
              const double advective_flux = (velocity_values[q] * vertical) * in.temperature[q] *
                                            out.densities[q]*out.specific_heat[q];
              const double conductive_flux = -(temperature_gradients[q]*vertical) *
                                             out.thermal_conductivities[q];
              output[q] = advective_flux + conductive_flux;
            }
        }

        const FEValuesExtractors::Vector velocity_field_;
        const FEValuesExtractors::Scalar temperature_field_;
        const GravityModel::Interface<dim> *gravity_model;
        std::vector<Tensor<1,dim> > velocity_values;
        std::vector<Tensor<1,dim> > temperature_gradients;
        std::vector<double> temperature_values;
    };
  }

  namespace internal
  {
    template <int dim>
    bool
    FunctorBase<dim>::need_material_properties() const
    {
      return false;
    }



    template <int dim>
    FunctorBase<dim>::~FunctorBase()
    {}



    template <int dim>
    void
    FunctorBase<dim>::create_additional_material_model_outputs (const unsigned int /*n_points*/,
                                                                MaterialModel::MaterialModelOutputs<dim> &/*outputs*/) const
    {}



    template <int dim>
    void
    FunctorBase<dim>::setup(const unsigned int /*q_points*/)
    {}
  }



  template <int dim>
  std::vector<std::vector<double> >
  LateralAveraging<dim>::compute_lateral_averages(const unsigned int n_slices,
                                                  std::vector<std::unique_ptr<internal::FunctorBase<dim> > > &functors) const
  {
    Assert (functors.size() > 0,
            ExcMessage ("To call this function, you need to request a positive "
                        "number of properties to compute."));
    Assert (n_slices > 0,
            ExcMessage ("To call this function, you need to request a positive "
                        "number of depth slices."));

    const unsigned int n_properties = functors.size();

    std::vector<std::vector<double> > values(n_properties,
                                             std::vector<double>(n_slices,0.0));
    std::vector<double> volume(n_slices,0.0);

    // this yields 10^dim quadrature points evenly distributed in the interior of the cell.
    // We avoid points on the faces, as they would be counted more than once.
    const QIterated<dim> quadrature_formula (QMidpoint<1>(),
                                             10);
    const unsigned int n_q_points = quadrature_formula.size();
    const double max_depth = this->get_geometry_model().maximal_depth();

    FEValues<dim> fe_values (this->get_mapping(),
                             this->get_fe(),
                             quadrature_formula,
                             update_values | update_gradients | update_quadrature_points | update_JxW_values);

    std::vector<std::vector<double> > composition_values (this->n_compositional_fields(),
                                                          std::vector<double> (n_q_points));
    std::vector<std::vector<double> > output_values(n_properties,
                                                    std::vector<double>(quadrature_formula.size()));

    MaterialModel::MaterialModelInputs<dim> in(n_q_points,
                                               this->n_compositional_fields());
    MaterialModel::MaterialModelOutputs<dim> out(n_q_points,
                                                 this->n_compositional_fields());

    bool functors_need_material_output = false;
    for (unsigned int i=0; i<n_properties; ++i)
      {
        functors[i]->setup(quadrature_formula.size());
        if (functors[i]->need_material_properties())
          functors_need_material_output = true;

        functors[i]->create_additional_material_model_outputs(n_q_points,out);
      }

    typename DoFHandler<dim>::active_cell_iterator
    cell = this->get_dof_handler().begin_active(),
    endc = this->get_dof_handler().end();
    for (; cell!=endc; ++cell)
      if (cell->is_locally_owned())
        {
          fe_values.reinit (cell);

          if (functors_need_material_output)
            {
              // get the material properties at each quadrature point if necessary
              in.reinit(fe_values,
                        cell,
                        this->introspection(),
                        this->get_solution());
              this->get_material_model().evaluate(in, out);
            }

          for (unsigned int i = 0; i < n_properties; ++i)
            (*functors[i])(in, out, fe_values, this->get_solution(), output_values[i]);

          for (unsigned int q = 0; q < n_q_points; ++q)
            {
              const double depth = this->get_geometry_model().depth(fe_values.quadrature_point(q));
              // make sure we are rounding down and never end up with idx==num_slices:
              const double magic = 1.0-2.0*std::numeric_limits<double>::epsilon();
              const unsigned int idx = static_cast<unsigned int>(std::floor((depth*n_slices)/max_depth*magic));

              Assert(idx<n_slices, ExcInternalError());

              for (unsigned int i = 0; i < n_properties; ++i)
                values[i][idx] += output_values[i][q] * fe_values.JxW(q);

              volume[idx] += fe_values.JxW(q);
            }
        }

    std::vector<double> volume_all(n_slices);
    Utilities::MPI::sum(volume, this->get_mpi_communicator(), volume_all);

    bool print_under_res_warning=false;
    for (unsigned int property=0; property<n_properties; ++property)
      {
        std::vector<double> values_all(n_slices);
        Utilities::MPI::sum(values[property], this->get_mpi_communicator(), values_all);

        for (unsigned int i=0; i<n_slices; ++i)
          {
            if (volume_all[i] > 0.0)
              {
                values[property][i] = values_all[i] / (static_cast<double>(volume_all[i]));
              }
            else
              {
                print_under_res_warning = true;
                // Output nan if no quadrature points in depth block
                values[property][i] = std::numeric_limits<double>::quiet_NaN();
              }
          }
      }

    if (print_under_res_warning)
      {
        this->get_pcout() << std::endl
                          << "**** Warning: When computing depth averages, there is at least one depth band"
                          << std::endl
                          << "     that does not have any quadrature points in it."
                          << std::endl
                          << "     Consider reducing the number of depth layers for averaging."
                          << std::endl << std::endl;
      }

    return values;
  }



  template <int dim>
  void LateralAveraging<dim>::get_temperature_averages(std::vector<double> &values) const
  {
    values = get_averages(values.size(),
                          std::vector<std::string>(1,"temperature"))[0];
  }



  template <int dim>
  void LateralAveraging<dim>::get_composition_averages(const unsigned int c,
                                                       std::vector<double> &values) const
  {
    values = get_averages(values.size(),
                          std::vector<std::string>(1,"C_" + Utilities::int_to_string(c)))[0];
  }



  template <int dim>
  void LateralAveraging<dim>::get_viscosity_averages(std::vector<double> &values) const
  {
    values = get_averages(values.size(),
                          std::vector<std::string>(1,"viscosity"))[0];
  }



  template <int dim>
  void LateralAveraging<dim>::get_velocity_magnitude_averages(std::vector<double> &values) const
  {
    values = get_averages(values.size(),
                          std::vector<std::string>(1,"velocity_magnitude"))[0];
  }



  template <int dim>
  void LateralAveraging<dim>::get_sinking_velocity_averages(std::vector<double> &values) const
  {
    values = get_averages(values.size(),
                          std::vector<std::string>(1,"sinking_velocity"))[0];
  }



  template <int dim>
  void LateralAveraging<dim>::get_Vs_averages(std::vector<double> &values) const
  {
    values = get_averages(values.size(),
                          std::vector<std::string>(1,"Vs"))[0];
  }



  template <int dim>
  void LateralAveraging<dim>::get_Vp_averages(std::vector<double> &values) const
  {
    values = get_averages(values.size(),
                          std::vector<std::string>(1,"Vp"))[0];
  }



  template <int dim>
  void LateralAveraging<dim>::get_vertical_heat_flux_averages(std::vector<double> &values) const
  {
    values = get_averages(values.size(),
                          std::vector<std::string>(1,"vertical_heat_flux"))[0];
  }



  template <int dim>
  std::vector<std::vector<double> >
  LateralAveraging<dim>::get_averages(const unsigned int n_slices,
                                      const std::vector<std::string> &property_names) const
  {
    std::vector<std::unique_ptr<internal::FunctorBase<dim> > > functors;
    for (unsigned int property_index=0; property_index<property_names.size(); ++property_index)
      {
        if (property_names[property_index] == "temperature")
          {
            functors.push_back(std_cxx14::make_unique<FunctorDepthAverageField<dim>>
                               (this->introspection().extractors.temperature));
          }
        else if (property_names[property_index].substr(0,2) == "C_")
          {
            const unsigned int c =
              Utilities::string_to_int(property_names[property_index].substr(2,std::string::npos));

            functors.push_back(std_cxx14::make_unique<FunctorDepthAverageField<dim>> (
                                 this->introspection().extractors.compositional_fields[c]));
          }
        else if (property_names[property_index] == "velocity_magnitude")
          {
            functors.push_back(std_cxx14::make_unique<FunctorDepthAverageVelocityMagnitude<dim>>
                               (this->introspection().extractors.velocities,
                                this->convert_output_to_years()));
          }
        else if (property_names[property_index] == "sinking_velocity")
          {
            functors.push_back(std_cxx14::make_unique<FunctorDepthAverageSinkingVelocity<dim>>
                               (this->introspection().extractors.velocities,
                                &this->get_gravity_model(),
                                this->convert_output_to_years()));
          }
        else if (property_names[property_index] == "Vs")
          {
            functors.push_back(std_cxx14::make_unique<FunctorDepthAverageVsVp<dim>> (true /* Vs */));
          }
        else if (property_names[property_index] == "Vp")
          {
            functors.push_back(std_cxx14::make_unique<FunctorDepthAverageVsVp<dim>> (false /* Vp */));
          }
        else if (property_names[property_index] == "viscosity")
          {
            functors.push_back(std_cxx14::make_unique<FunctorDepthAverageViscosity<dim>>());
          }
        else if (property_names[property_index] == "vertical_heat_flux")
          {
            functors.push_back(std_cxx14::make_unique<FunctorDepthAverageVerticalHeatFlux<dim>>
                               (this->introspection().extractors.velocities,
                                this->introspection().extractors.temperature,
                                &this->get_gravity_model()));
          }
        else
          {
            AssertThrow(false,
                        ExcMessage("The lateral averaging scheme was asked to average the property "
                                   "named <" + property_names[property_index] + ">, but it does not know how "
                                   "to do that. There is no functor implemented that computes this property."));
          }
      }

    // Now compute values for all selected properties.
    return compute_lateral_averages(n_slices, functors);
  }
}

namespace aspect
{
#define INSTANTIATE(dim) \
  template class LateralAveraging<dim>;
  ASPECT_INSTANTIATE(INSTANTIATE)
}
