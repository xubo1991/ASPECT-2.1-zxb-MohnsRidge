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


#include <aspect/geometry_model/sphere.h>
#include <aspect/geometry_model/initial_topography_model/zero_topography.h>

#include <deal.II/grid/grid_generator.h>

#if !DEAL_II_VERSION_GTE(9,0,0)
#include <deal.II/grid/tria_boundary_lib.h>
#endif

namespace aspect
{
  namespace GeometryModel
  {
    /*
      intel 18 incorrectly complains:

      warning #411: class template "aspect::GeometryModel::Sphere<dim>" defines no constructor to initialize the following:
      const member "aspect::GeometryModel::Sphere<dim>::spherical_manifold"

      even though SphericalManifold's constructor has only one argument with a default.
    */
    template <int dim>
    Sphere<dim>::Sphere()
#if DEAL_II_VERSION_GTE(9,0,0)
      :
      spherical_manifold()
#endif
    {}



    template <int dim>
    void
    Sphere<dim>::
    create_coarse_mesh (parallel::distributed::Triangulation<dim> &coarse_grid) const
    {
      GridGenerator::hyper_ball (coarse_grid,
                                 Point<dim>(),
                                 R);

#if DEAL_II_VERSION_GTE(9,0,0)
      coarse_grid.set_manifold(0,spherical_manifold);
      coarse_grid.set_all_manifold_ids_on_boundary(0);
#else
      static const HyperBallBoundary<dim> boundary_ball(Point<dim>(), R);
      coarse_grid.set_boundary (0, boundary_ball);
#endif
    }


    template <int dim>
    std::set<types::boundary_id>
    Sphere<dim>::
    get_used_boundary_indicators () const
    {
      const types::boundary_id s[] = { 0 };
      return std::set<types::boundary_id>(&s[0],
                                          &s[sizeof(s)/sizeof(s[0])]);
    }


    template <int dim>
    std::map<std::string,types::boundary_id>
    Sphere<dim>::
    get_symbolic_boundary_names_map () const
    {
      static const std::pair<std::string,types::boundary_id> mapping("top", 0);
      return std::map<std::string,types::boundary_id> (&mapping,
                                                       &mapping+1);
    }


    template <int dim>
    double
    Sphere<dim>::
    length_scale () const
    {
      // as described in the first ASPECT paper, a length scale of
      // 10km = 1e4m works well for the pressure scaling for earth
      // sized spherical shells. use a length scale that
      // yields this value for the R0,R1 corresponding to earth
      // but otherwise scales like (R1-R0)
      return 1e4 * maximal_depth() / (6336000.-3481000.);
    }



    template <int dim>
    double
    Sphere<dim>::depth(const Point<dim> &position) const
    {
      return std::min (std::max (R-position.norm(), 0.), maximal_depth());
    }

    template <int dim>
    double
    Sphere<dim>::height_above_reference_surface(const Point<dim> &position) const
    {
      return position.norm()-radius();
    }


    template <int dim>
    Point<dim>
    Sphere<dim>::representative_point(const double depth) const
    {
      Point<dim> p;
      p(dim-1) = std::min (std::max(R - depth, 0.), maximal_depth());
      return p;
    }



    template <int dim>
    double
    Sphere<dim>::maximal_depth() const
    {
      return R;
    }

    template <int dim>
    double Sphere<dim>::radius () const
    {
      return R;
    }

    template <int dim>
    bool
    Sphere<dim>::has_curved_elements () const
    {
      return true;
    }



    template <int dim>
    bool
    Sphere<dim>::point_is_in_domain(const Point<dim> &point) const
    {
      AssertThrow(this->get_free_surface_boundary_indicators().size() == 0 ||
                  this->get_timestep_number() == 0,
                  ExcMessage("After displacement of the free surface, this function can no longer be used to determine whether a point lies in the domain or not."));

      AssertThrow(dynamic_cast<const InitialTopographyModel::ZeroTopography<dim>*>(&this->get_initial_topography_model()) != nullptr,
                  ExcMessage("After adding topography, this function can no longer be used to determine whether a point lies in the domain or not."));

      const double radius = point.norm();

      if (radius > R+std::numeric_limits<double>::epsilon()*R)
        return false;

      return true;
    }


    template <int dim>
    aspect::Utilities::Coordinates::CoordinateSystem
    Sphere<dim>::natural_coordinate_system() const
    {
      return aspect::Utilities::Coordinates::CoordinateSystem::spherical;
    }



    template <int dim>
    void
    Sphere<dim>::declare_parameters (ParameterHandler &prm)
    {
      prm.enter_subsection("Geometry model");
      {
        prm.enter_subsection("Sphere");
        {
          prm.declare_entry ("Radius", "6371000",
                             Patterns::Double (0),
                             "Radius of the sphere. Units: m.");
        }
        prm.leave_subsection();
      }
      prm.leave_subsection();
    }



    template <int dim>
    void
    Sphere<dim>::parse_parameters (ParameterHandler &prm)
    {
      prm.enter_subsection("Geometry model");
      {
        prm.enter_subsection("Sphere");
        {
          R            = prm.get_double ("Radius");
        }
        prm.leave_subsection();
      }
      prm.leave_subsection();
    }
  }
}

// explicit instantiations
namespace aspect
{
  namespace GeometryModel
  {
    ASPECT_REGISTER_GEOMETRY_MODEL(Sphere,
                                   "sphere",
                                   "A geometry model for a sphere with a user specified "
                                   "radius. This geometry has only a single boundary, so "
                                   "the only valid boundary indicator to "
                                   "specify in input files is ``0''. It can also be "
                                   "referenced by the symbolic name ``surface'' in "
                                   "input files."
                                   "\n\n"
                                   "Despite the name, this geometry does not imply the use of "
                                   "a spherical coordinate system when used in 2d. Indeed, "
                                   "in 2d the geometry is simply a circle in a Cartesian "
                                   "coordinate system and consequently would correspond to "
                                   "a cross section of the fluid filled interior of an "
                                   "infinite cylinder where one has made the assumption that "
                                   "the velocity in direction of the cylinder axes is zero. "
                                   "This is consistent with the definition of what we consider "
                                   "the two-dimension case given in "
                                   "Section~\\ref{sec:meaning-of-2d}.")
  }
}
