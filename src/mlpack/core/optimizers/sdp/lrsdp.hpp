/**
 * @file lrsdp.hpp
 * @author Ryan Curtin
 *
 * An implementation of Monteiro and Burer's formulation of low-rank
 * semidefinite programs (LR-SDP).
 *
 * This file is part of mlpack 2.0.0.
 *
 * mlpack is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * mlpack is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details (LICENSE.txt).
 *
 * You should have received a copy of the GNU General Public License along with
 * mlpack.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __MLPACK_CORE_OPTIMIZERS_SDP_LRSDP_HPP
#define __MLPACK_CORE_OPTIMIZERS_SDP_LRSDP_HPP

#include <mlpack/core.hpp>
#include <mlpack/core/optimizers/aug_lagrangian/aug_lagrangian.hpp>

#include "lrsdp_function.hpp"

namespace mlpack {
namespace optimization {

/**
 * LRSDP is the implementation of Monteiro and Burer's formulation of low-rank
 * semidefinite programs (LR-SDP).  This solver uses the augmented Lagrangian
 * optimizer to solve low-rank semidefinite programs.
 */
template <typename SDPType>
class LRSDP
{
 public:
  /**
   * Create an LRSDP to be optimized.  The solution will end up being a matrix
   * of size (rows) x (rank).  To construct each constraint and the objective
   * function, use the function SDP() in order to access the SDPType object
   * associated with this optimizer.
   *
   * @param numConstraints Number of constraints in the problem.
   * @param initialPoint Initial point of the optimization.
   */
  LRSDP(const size_t numSparseConstraints,
        const size_t numDenseConstraints,
        const arma::mat& initialPoint);

  /**
   * Create an LRSDP object with the given SDP problem to be solved, and the
   * given initial point.  Note that the SDP may be modified later by calling
   * SDP() to access the object.
   *
   * @param sdp SDP to be solved.
   * @param initialPoint Initial point of the optimization.
   */
  LRSDP(const SDPType& sdp,
        const arma::mat& initialPoint);

  /**
   * Optimize the LRSDP and return the final objective value.  The given
   * coordinates will be modified to contain the final solution.
   *
   * @param coordinates Starting coordinates for the optimization.
   */
  double Optimize(arma::mat& coordinates);

  //! Return the SDP that will be solved.
  const SDPType& SDP() const { return function.SDP(); }
  //! Modify the SDP that will be solved.
  SDPType& SDP() { return function.SDP(); }

  //! Return the function to be optimized.
  const LRSDPFunction<SDPType>& Function() const { return function; }
  //! Modify the function to be optimized.
  LRSDPFunction<SDPType>& Function() { return function; }

  //! Return the augmented Lagrangian object.
  const AugLagrangian<LRSDPFunction<SDPType>>& AugLag() const { return augLag; }
  //! Modify the augmented Lagrangian object.
  AugLagrangian<LRSDPFunction<SDPType>>& AugLag() { return augLag; }

 private:
  //! Function to optimize, which the AugLagrangian object holds.
  LRSDPFunction<SDPType> function;

  //! The AugLagrangian object which will be used for optimization.
  AugLagrangian<LRSDPFunction<SDPType>> augLag;
};

} // namespace optimization
} // namespace mlpack

// Include implementation
#include "lrsdp_impl.hpp"

#endif
