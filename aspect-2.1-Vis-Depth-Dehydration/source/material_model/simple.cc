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


#include <aspect/material_model/simple.h>


namespace aspect
{
  namespace MaterialModel
  {
    template <int dim>
    void
    Simple<dim>::
    evaluate(const MaterialModel::MaterialModelInputs<dim> &in,
             MaterialModel::MaterialModelOutputs<dim> &out) const
    {
      for (unsigned int i=0; i < in.position.size(); ++i)
        {
          //const double delta_temp = in.temperature[i]-reference_T;
          const double temperature_dependence
            = ((in.pressure[i]<=3300*9.8*80000) || (in.temperature[i]>=1120.7+273+132.9*in.pressure[i]/1e9-5.1*(in.pressure[i]/1e9)*(in.pressure[i]/1e9))
               ?
               std::max(std::min(200*std::exp((activation_energy_zxb+in.pressure[i]*activation_volume_zxb)/(gas_constant_zxb*in.temperature[i])-(activation_energy_zxb+reference_rho*gravity_cons_zxb*model_height_zxb*activation_volume_zxb)/(gas_constant_zxb*model_bottom_tem_zxb)),
                                 maximum_thermal_prefactor),
                        minimum_thermal_prefactor)
               :
               std::max(std::min(1*std::exp((activation_energy_zxb+in.pressure[i]*activation_volume_zxb)/(gas_constant_zxb*in.temperature[i])-(activation_energy_zxb+reference_rho*gravity_cons_zxb*model_height_zxb*activation_volume_zxb)/(gas_constant_zxb*model_bottom_tem_zxb)),
                                 maximum_thermal_prefactor),
                        minimum_thermal_prefactor));

          out.viscosities[i] = ((composition_viscosity_prefactor != 1.0) && (in.composition[i].size()>0))
                               ?
                               // Geometric interpolation
                               std::pow(10.0, ((1-in.composition[i][0]) * std::log10(eta *
                                                                                     temperature_dependence)
                                               + in.composition[i][0] * std::log10(eta *
                                                                                   composition_viscosity_prefactor *
                                                                                   temperature_dependence)))
                               :
                               temperature_dependence * eta;

          const double c = (in.composition[i].size()>0)
                           ?
                           std::max(0.0, in.composition[i][0])
                           :
                           0.0;

          out.densities[i] = reference_rho * (1 - thermal_alpha * (in.temperature[i] - reference_T))
                             + compositional_delta_rho * c;

          out.thermal_expansion_coefficients[i] = thermal_alpha;
          out.specific_heat[i] = reference_specific_heat;
          out.thermal_conductivities[i] = k_value;
          out.compressibilities[i] = 0.0;
          // Pressure derivative of entropy at the given positions.
          out.entropy_derivative_pressure[i] = 0.0;
          // Temperature derivative of entropy at the given positions.
          out.entropy_derivative_temperature[i] = 0.0;
          // Change in composition due to chemical reactions at the
          // given positions. The term reaction_terms[i][c] is the
          // change in compositional field c at point i.
          for (unsigned int c=0; c<in.composition[i].size(); ++c)
            out.reaction_terms[i][c] = 0.0;
        }
    }


    template <int dim>
    double
    Simple<dim>::
    reference_viscosity () const
    {
      return eta;
    }



    template <int dim>
    bool
    Simple<dim>::
    is_compressible () const
    {
      return false;
    }



    template <int dim>
    void
    Simple<dim>::declare_parameters (ParameterHandler &prm)
    {
      prm.enter_subsection("Material model");
      {
        prm.enter_subsection("Simple model");
        {
          prm.declare_entry ("Activation energy", "2.5e5",
                             Patterns::Double (0),
                             "Activation energy $\\activation_energy_zxb$. Units: $J/mol$.");

          prm.declare_entry ("Activation volume", "4e-6",
                             Patterns::Double (0),
                             "Activation volume $\\activation_volume_zxb$. Units: $m^3/mol$.");

          prm.declare_entry ("Gas constant", "8.314",
                             Patterns::Double (0),
                             "Gas constant $\\gas_constant_zxb$. Units: $J/mol/K$.");

          prm.declare_entry ("Bottom temperature of the model", "1573",
                             Patterns::Double (0),
                             "Bottom temperature of the model $\\model_bottom_tem_zxb$. Units: $K$.");

          prm.declare_entry ("Height of the model", "400000",
                             Patterns::Double (0),
                             "Height of the model $\\model_height_zxb$. Units: $m$.");

          prm.declare_entry ("Gravitational acceleration", "10",
                             Patterns::Double (0),
                             "Gravitational acceleration $\\gravity_cons_zxb$. Units: $m/s^2$.");

          prm.declare_entry ("Reference density", "3300",
                             Patterns::Double (0),
                             "Reference density $\\rho_0$. Units: $kg/m^3$.");
          prm.declare_entry ("Reference temperature", "293",
                             Patterns::Double (0),
                             "The reference temperature $T_0$. The reference temperature is used "
                             "in both the density and viscosity formulas. Units: $K$.");
          prm.declare_entry ("Viscosity", "5e24",
                             Patterns::Double (0),
                             "The value of the constant viscosity $\\eta_0$. This viscosity may be "
                             "modified by both temperature and compositional dependencies. Units: $kg/m/s$.");
          prm.declare_entry ("Composition viscosity prefactor", "1.0",
                             Patterns::Double (0),
                             "A linear dependency of viscosity on the first compositional field. "
                             "Dimensionless prefactor. With a value of 1.0 (the default) the "
                             "viscosity does not depend on the composition. See the general documentation "
                             "of this model for a formula that states the dependence of the "
                             "viscosity on this factor, which is called $\\xi$ there.");
          prm.declare_entry ("Thermal viscosity exponent", "0.0",
                             Patterns::Double (0),
                             "The temperature dependence of viscosity. Dimensionless exponent. "
                             "See the general documentation "
                             "of this model for a formula that states the dependence of the "
                             "viscosity on this factor, which is called $\\beta$ there.");
          prm.declare_entry("Maximum thermal prefactor","1.0e2",
                            Patterns::Double (0),
                            "The maximum value of the viscosity prefactor associated with temperature "
                            "dependence.");
          prm.declare_entry("Minimum thermal prefactor","1.0e-2",
                            Patterns::Double (0),
                            "The minimum value of the viscosity prefactor associated with temperature "
                            "dependence.");
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
                             "The value of the thermal expansion coefficient $\\alpha$. "
                             "Units: $1/K$.");
          prm.declare_entry ("Density differential for compositional field 1", "0",
                             Patterns::Double(),
                             "If compositional fields are used, then one would frequently want "
                             "to make the density depend on these fields. In this simple material "
                             "model, we make the following assumptions: if no compositional fields "
                             "are used in the current simulation, then the density is simply the usual "
                             "one with its linear dependence on the temperature. If there are compositional "
                             "fields, then the density only depends on the first one in such a way that "
                             "the density has an additional term of the kind $+\\Delta \\rho \\; c_1(\\mathbf x)$. "
                             "This parameter describes the value of $\\Delta \\rho$. Units: $kg/m^3/\\textrm{unit "
                             "change in composition}$.");
        }
        prm.leave_subsection();
      }
      prm.leave_subsection();
    }



    template <int dim>
    void
    Simple<dim>::parse_parameters (ParameterHandler &prm)
    {
      prm.enter_subsection("Material model");
      {
        prm.enter_subsection("Simple model");
        {
          activation_energy_zxb          = prm.get_double ("Activation energy");
          activation_volume_zxb          = prm.get_double ("Activation volume");
          gas_constant_zxb               = prm.get_double ("Gas constant");
          model_bottom_tem_zxb           = prm.get_double ("Bottom temperature of the model");
          model_height_zxb               = prm.get_double ("Height of the model");
          gravity_cons_zxb               = prm.get_double ("Gravitational acceleration");

          reference_rho              = prm.get_double ("Reference density");
          reference_T                = prm.get_double ("Reference temperature");
          eta                        = prm.get_double ("Viscosity");
          composition_viscosity_prefactor = prm.get_double ("Composition viscosity prefactor");
          thermal_viscosity_exponent = prm.get_double ("Thermal viscosity exponent");
          maximum_thermal_prefactor       = prm.get_double ("Maximum thermal prefactor");
          minimum_thermal_prefactor       = prm.get_double ("Minimum thermal prefactor");
          if ( maximum_thermal_prefactor == 0.0 ) maximum_thermal_prefactor = std::numeric_limits<double>::max();
          if ( minimum_thermal_prefactor == 0.0 ) minimum_thermal_prefactor = std::numeric_limits<double>::min();

          k_value                    = prm.get_double ("Thermal conductivity");
          reference_specific_heat    = prm.get_double ("Reference specific heat");
          thermal_alpha              = prm.get_double ("Thermal expansion coefficient");
          compositional_delta_rho    = prm.get_double ("Density differential for compositional field 1");

          if (thermal_viscosity_exponent!=0.0 && reference_T == 0.0)
            AssertThrow(false, ExcMessage("Error: Material model simple with Thermal viscosity exponent can not have reference_T=0."));
        }
        prm.leave_subsection();
      }
      prm.leave_subsection();

      // Declare dependencies on solution variables
      this->model_dependence.compressibility = NonlinearDependence::none;
      this->model_dependence.specific_heat = NonlinearDependence::none;
      this->model_dependence.thermal_conductivity = NonlinearDependence::none;
      this->model_dependence.viscosity = NonlinearDependence::none;
      this->model_dependence.density = NonlinearDependence::none;

      if (thermal_viscosity_exponent != 0)
        this->model_dependence.viscosity |= NonlinearDependence::temperature;
      if (composition_viscosity_prefactor != 1.0)
        this->model_dependence.viscosity |= NonlinearDependence::compositional_fields;

      if (thermal_alpha != 0)
        this->model_dependence.density |=NonlinearDependence::temperature;
      if (compositional_delta_rho != 0)
        this->model_dependence.density |=NonlinearDependence::compositional_fields;
    }
  }
}

// explicit instantiations
namespace aspect
{
  namespace MaterialModel
  {
    ASPECT_REGISTER_MATERIAL_MODEL(Simple,
                                   "simple",
                                   "A material model that has constant values "
                                   "for all coefficients but the density and viscosity. The defaults for all "
                                   "coefficients are chosen to be similar to what is believed to be correct "
                                   "for Earth's mantle. All of the values that define this model are read "
                                   "from a section ``Material model/Simple model'' in the input file, see "
                                   "Section~\\ref{parameters:Material_20model/Simple_20model}."
                                   "\n\n"
                                   "This model uses the following set of equations for the two coefficients that "
                                   "are non-constant: "
                                   "\\begin{align}"
                                   "  \\eta(p,T,\\mathfrak c) &= \\tau(T) \\zeta(\\mathfrak c) \\eta_0, \\\\"
                                   "  \\rho(p,T,\\mathfrak c) &= \\left(1-\\alpha (T-T_0)\\right)\\rho_0 + \\Delta\\rho \\; c_0,"
                                   "\\end{align}"
                                   "where $c_0$ is the first component of the compositional vector "
                                   "$\\mathfrak c$ if the model uses compositional fields, or zero otherwise. "
                                   "\n\n"
                                   "The temperature pre-factor for the viscosity formula above is "
                                   "defined as "
                                   "\\begin{align}"
                                   "  \\tau(T) &= H\\left(e^{-\\beta (T-T_0)/T_0}\\right),"
                                   "\\intertext{with} "
                                   "  \\qquad\\qquad H(x) &= \\begin{cases}"
                                   "                            \\tau_{\\text{min}} & \\text{if}\\; x<\\tau_{\\text{min}}, \\\\"
                                   "                            x & \\text{if}\\; 10^{-2}\\le x \\le 10^2, \\\\"
                                   "                            \\tau_{\\text{max}} & \\text{if}\\; x>\\tau_{\\text{max}}, \\\\"
                                   "                         \\end{cases}"
                                   "\\end{align} "
                                   "where $x=e^{-\\beta (T-T_0)/T_0}$, "
                                   "$\\beta$ corresponds to the input parameter ``Thermal viscosity exponent'', "
                                   "and $T_0$ to the parameter ``Reference temperature''. If you set $T_0=0$ "
                                   "in the input file, the thermal pre-factor $\\tau(T)=1$. The parameters $\\tau_{\\text{min}}$ "
                                   "and $\\tau_{\\text{max}}$ set the minimum and maximum values of the temperature pre-factor "
                                   "and are set using ``Maximum thermal prefactor'' and ``Minimum thermal prefactor''. "
                                   "Specifying a value of 0.0 for the minimum or maximum values will disable pre-factor limiting."
                                   "\n\n"
                                   "The compositional pre-factor for the viscosity is defined as "
                                   "\\begin{align}"
                                   "  \\zeta(\\mathfrak c) &= \\xi^{c_0}"
                                   "\\end{align} "
                                   "if the model has compositional fields and equals one otherwise. $\\xi$ "
                                   "corresponds to the parameter ``Composition viscosity prefactor'' in the "
                                   "input file."
                                   "\n\n"
                                   "Finally, in the formula for the density, $\\alpha$ corresponds to the "
                                   "``Thermal expansion coefficient'' and "
                                   "$\\Delta\\rho$ "
                                   "corresponds to the parameter ``Density differential for compositional field 1''."
                                   "\n\n"
                                   "Note that this model uses the formulation that assumes an incompressible "
                                   "medium despite the fact that the density follows the law "
                                   "$\\rho(T)=\\rho_0(1-\\alpha(T-T_{\\text{ref}}))$. "
                                   "\n\n"
                                   "\\note{Despite its name, this material model is not exactly ``simple'', "
                                   "as indicated by the formulas above. While it was originally intended "
                                   "to be simple, it has over time acquired all sorts of temperature "
                                   "and compositional dependencies that weren't initially intended. "
                                   "Consequently, there is now a ``simpler'' material model that now fills "
                                   "the role the current model was originally intended to fill.}")
  }
}
