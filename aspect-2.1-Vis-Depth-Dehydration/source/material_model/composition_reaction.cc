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


#include <aspect/material_model/composition_reaction.h>
#include <aspect/geometry_model/interface.h>


namespace aspect
{
  namespace MaterialModel
  {

    template <int dim>
    void
    CompositionReaction<dim>::
    evaluate(const MaterialModelInputs<dim> &in,
             MaterialModelOutputs<dim> &out) const
    {
      ReactionRateOutputs<dim> *reaction_rate_out = out.template get_additional_output<ReactionRateOutputs<dim> >();

      for (unsigned int i=0; i < in.position.size(); ++i)
        {
          const double temperature = in.temperature[i];
          const std::vector<double> &composition = in.composition[i];
          const double delta_temp = temperature-reference_T;
          double temperature_dependence = std::max(std::min(std::exp(-thermal_viscosity_exponent*delta_temp/reference_T),1e2),1e-2);

          if (std::isnan(temperature_dependence))
            temperature_dependence = 1.0;

          switch (composition.size())
            {
              case 0:
                out.viscosities[i] = temperature_dependence * eta;
                break;
              case 1:
                // geometric interpolation
                out.viscosities[i] = (pow(10, ((1-composition[0]) * log10(eta*temperature_dependence)
                                               + composition[0] * log10(eta*composition_viscosity_prefactor_1*temperature_dependence))));
                break;
              default:
                out.viscosities[i] = (pow(10, ((1 - 0.5*composition[0] - 0.5*composition[1]) * log10(eta*temperature_dependence)
                                               + 0.5 * composition[0] * log10(eta*composition_viscosity_prefactor_1*temperature_dependence)
                                               + 0.5 * composition[1] * log10(eta*composition_viscosity_prefactor_2*temperature_dependence))));
                break;
            }

          const double c1 = composition.size()>0?
                            std::max(0.0, composition[0])
                            :
                            0.0;
          const double c2 = composition.size()>1?
                            std::max(0.0, composition[1])
                            :
                            0.0;
          out.densities[i] = reference_rho * (1 - thermal_alpha * (temperature - reference_T))
                             + compositional_delta_rho_1 * c1 + compositional_delta_rho_2 * c2;


          const double depth = this->get_geometry_model().depth(in.position[i]);
          for (unsigned int c=0; c<this->n_compositional_fields(); ++c)
            {
              double delta_C = 0.0;
              switch (c)
                {
                  case 0:
                    if (depth < reaction_depth) delta_C = -composition[0];
                    break;
                  case 1:
                    if (depth < reaction_depth) delta_C = composition[0];
                    break;
                  default:
                    delta_C = 0.0;
                    break;
                }
              out.reaction_terms[i][c] = delta_C;

              // Fill reaction rate outputs instead of the reaction terms if we use operator splitting
              // (and then set the latter to zero).
              if (this->get_parameters().use_operator_splitting)
                {
                  if (reaction_rate_out != nullptr)
                    reaction_rate_out->reaction_rates[i][c] = (this->get_timestep_number() > 0
                                                               ?
                                                               out.reaction_terms[i][c] / this->get_timestep()
                                                               :
                                                               0.0);
                  out.reaction_terms[i][c] = 0.0;
                }
            }

          out.specific_heat[i] = reference_specific_heat;
          out.thermal_conductivities[i] = k_value;
          out.thermal_expansion_coefficients[i] = thermal_alpha;
          out.compressibilities[i] = 0.0;
        }
    }

    template <int dim>
    double
    CompositionReaction<dim>::
    reference_viscosity () const
    {
      return eta;
    }



    template <int dim>
    bool
    CompositionReaction<dim>::
    is_compressible () const
    {
      return false;
    }



    template <int dim>
    void
    CompositionReaction<dim>::declare_parameters (ParameterHandler &prm)
    {
      prm.enter_subsection("Material model");
      {
        prm.enter_subsection("Composition reaction model");
        {
          prm.declare_entry ("Reference density", "3300",
                             Patterns::Double (0),
                             "Reference density $\\rho_0$. Units: $kg/m^3$.");
          prm.declare_entry ("Reference temperature", "293",
                             Patterns::Double (0),
                             "The reference temperature $T_0$. Units: $K$.");
          prm.declare_entry ("Viscosity", "5e24",
                             Patterns::Double (0),
                             "The value of the constant viscosity. Units: $kg/m/s$.");
          prm.declare_entry ("Composition viscosity prefactor 1", "1.0",
                             Patterns::Double (0),
                             "A linear dependency of viscosity on the first compositional field. "
                             "Dimensionless prefactor. With a value of 1.0 (the default) the "
                             "viscosity does not depend on the composition.");
          prm.declare_entry ("Composition viscosity prefactor 2", "1.0",
                             Patterns::Double (0),
                             "A linear dependency of viscosity on the second compositional field. "
                             "Dimensionless prefactor. With a value of 1.0 (the default) the "
                             "viscosity does not depend on the composition.");
          prm.declare_entry ("Thermal viscosity exponent", "0.0",
                             Patterns::Double (0),
                             "The temperature dependence of viscosity. Dimensionless exponent.");
          prm.declare_entry ("Thermal conductivity", "4.7",
                             Patterns::Double (0),
                             "The value of the thermal conductivity $k$. "
                             "Units: $W/m/K$.");
          prm.declare_entry ("Reference specific heat", "1250",
                             Patterns::Double (0),
                             "The value of the specific heat $C_p$. "
                             "Units: $J/kg/K$.");
          prm.declare_entry ("Thermal expansion coefficient", "2e-5",
                             Patterns::Double (0),
                             "The value of the thermal expansion coefficient $\\beta$. "
                             "Units: $1/K$.");
          prm.declare_entry ("Density differential for compositional field 1", "0",
                             Patterns::Double(),
                             "If compositional fields are used, then one would frequently want "
                             "to make the density depend on these fields. In this simple material "
                             "model, we make the following assumptions: if no compositional fields "
                             "are used in the current simulation, then the density is simply the usual "
                             "one with its linear dependence on the temperature. If there are compositional "
                             "fields, then the density only depends on the first and the second one in such a way that "
                             "the density has an additional term of the kind $+\\Delta \\rho \\; c_1(\\mathbf x)$. "
                             "This parameter describes the value of $\\Delta \\rho$ for the first field. "
                             "Units: $kg/m^3/\\textrm{unit change in composition}$.");
          prm.declare_entry ("Density differential for compositional field 2", "0",
                             Patterns::Double(),
                             "If compositional fields are used, then one would frequently want "
                             "to make the density depend on these fields. In this simple material "
                             "model, we make the following assumptions: if no compositional fields "
                             "are used in the current simulation, then the density is simply the usual "
                             "one with its linear dependence on the temperature. If there are compositional "
                             "fields, then the density only depends on the first and the second one in such a way that "
                             "the density has an additional term of the kind $+\\Delta \\rho \\; c_1(\\mathbf x)$. "
                             "This parameter describes the value of $\\Delta \\rho$ for the second field. "
                             "Units: $kg/m^3/\\textrm{unit change in composition}$.");
          prm.declare_entry ("Reaction depth", "0",
                             Patterns::Double (0),
                             "Above this depth the compositional fields react: "
                             "The first field gets converted to the second field. "
                             "Units: $m$.");
        }
        prm.leave_subsection();
      }
      prm.leave_subsection();
    }



    template <int dim>
    void
    CompositionReaction<dim>::parse_parameters (ParameterHandler &prm)
    {
      prm.enter_subsection("Material model");
      {
        prm.enter_subsection("Composition reaction model");
        {
          reference_rho              = prm.get_double ("Reference density");
          reference_T                = prm.get_double ("Reference temperature");
          eta                        = prm.get_double ("Viscosity");
          composition_viscosity_prefactor_1 = prm.get_double ("Composition viscosity prefactor 1");
          composition_viscosity_prefactor_2 = prm.get_double ("Composition viscosity prefactor 2");
          thermal_viscosity_exponent = prm.get_double ("Thermal viscosity exponent");
          k_value                    = prm.get_double ("Thermal conductivity");
          reference_specific_heat    = prm.get_double ("Reference specific heat");
          thermal_alpha              = prm.get_double ("Thermal expansion coefficient");
          compositional_delta_rho_1  = prm.get_double ("Density differential for compositional field 1");
          compositional_delta_rho_2  = prm.get_double ("Density differential for compositional field 2");
          reaction_depth             = prm.get_double ("Reaction depth");

          if (thermal_viscosity_exponent!=0.0 && reference_T == 0.0)
            AssertThrow(false, ExcMessage("Error: Material model composition reaction with Thermal viscosity exponent can not have reference_T=0."));
        }
        prm.leave_subsection();
      }
      prm.leave_subsection();

      // Declare dependencies on solution variables
      this->model_dependence.viscosity = NonlinearDependence::none;
      this->model_dependence.density = NonlinearDependence::none;
      this->model_dependence.compressibility = NonlinearDependence::none;
      this->model_dependence.specific_heat = NonlinearDependence::none;
      this->model_dependence.thermal_conductivity = NonlinearDependence::none;

      if (thermal_viscosity_exponent != 0)
        this->model_dependence.viscosity |= NonlinearDependence::temperature;
      if ((composition_viscosity_prefactor_1 != 1.0) ||
          (composition_viscosity_prefactor_2 != 1.0))
        this->model_dependence.viscosity |= NonlinearDependence::compositional_fields;

      if (thermal_alpha != 0)
        this->model_dependence.density |= NonlinearDependence::temperature;
      if ((compositional_delta_rho_1 != 0) ||
          (compositional_delta_rho_2 != 0))
        this->model_dependence.density |= NonlinearDependence::compositional_fields;
    }


    template <int dim>
    void
    CompositionReaction<dim>::create_additional_named_outputs (MaterialModel::MaterialModelOutputs<dim> &out) const
    {
      if (this->get_parameters().use_operator_splitting
          && out.template get_additional_output<ReactionRateOutputs<dim> >() == nullptr)
        {
          const unsigned int n_points = out.viscosities.size();
          out.additional_outputs.push_back(
            std::make_shared<MaterialModel::ReactionRateOutputs<dim>> (n_points,
                                                                       this->n_compositional_fields()));
        }
    }
  }
}

// explicit instantiations
namespace aspect
{
  namespace MaterialModel
  {
    ASPECT_REGISTER_MATERIAL_MODEL(CompositionReaction,
                                   "composition reaction",
                                   "A material model that behaves in the same way as "
                                   "the simple material model, but includes two compositional "
                                   "fields and a reaction between them. Above a depth given "
                                   "in the input file, the first fields gets converted to the "
                                   "second field. ")
  }
}
