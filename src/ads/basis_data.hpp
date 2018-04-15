#ifndef ADS_BASIS_DATA_HPP_
#define ADS_BASIS_DATA_HPP_

#include <vector>
#include <boost/range/counting_range.hpp>

#include "ads/bspline/bspline.hpp"

namespace ads {

typedef int element_id;

struct basis_data {
    using dof_range_type = decltype(boost::counting_range(0, 0));

    std::vector<int> first_dofs;
    int degree;
    int elements;
    int quad_order;
    int elem_division;
    std::vector<double> points;
    double**** b;
    double** x;
    const double* w;
    double* J;

    basis_data(const bspline::basis& basis, int derivatives)
    : basis_data{ std::move(basis), derivatives, basis.degree + 1, 1 }
    { }

    basis_data(const bspline::basis& basis, int derivatives, int quad_order, int elem_division);

    int first_dof(element_id e) const {
        return first_dofs[e / elem_division];
    }

    int last_dof(element_id e) const {
        return first_dof(e) + dofs_per_element() - 1;
    }

    int dofs_per_element() const {
        return degree + 1;
    }

    dof_range_type dof_range(element_id e) const {
        return boost::counting_range(first_dof(e), last_dof(e) + 1);
    }

};

}


#endif /* ADS_BASIS_DATA_HPP_ */
