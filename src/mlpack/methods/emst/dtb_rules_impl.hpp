/**
 * @file dtb_impl.hpp
 * @author Bill March (march@gatech.edu)
 *
 * Tree traverser rules for the DualTreeBoruvka algorithm.
 *
 * This file is part of MLPACK 1.0.8.
 *
 * MLPACK is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * MLPACK is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details (LICENSE.txt).
 *
 * You should have received a copy of the GNU General Public License along with
 * MLPACK.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __MLPACK_METHODS_EMST_DTB_RULES_IMPL_HPP
#define __MLPACK_METHODS_EMST_DTB_RULES_IMPL_HPP

namespace mlpack {
namespace emst {

template<typename MetricType, typename TreeType>
DTBRules<MetricType, TreeType>::
DTBRules(const arma::mat& dataSet,
         UnionFind& connections,
         arma::vec& neighborsDistances,
         arma::Col<size_t>& neighborsInComponent,
         arma::Col<size_t>& neighborsOutComponent,
         MetricType& metric)
:
  dataSet(dataSet),
  connections(connections),
  neighborsDistances(neighborsDistances),
  neighborsInComponent(neighborsInComponent),
  neighborsOutComponent(neighborsOutComponent),
  metric(metric)
{
  // Nothing else to do.
}

template<typename MetricType, typename TreeType>
inline force_inline
double DTBRules<MetricType, TreeType>::BaseCase(const size_t queryIndex,
                                                const size_t referenceIndex)
{
  // Check if the points are in the same component at this iteration.
  // If not, return the distance between them.  Also, store a better result as
  // the current neighbor, if necessary.
  double newUpperBound = -1.0;

  // Find the index of the component the query is in.
  size_t queryComponentIndex = connections.Find(queryIndex);

  size_t referenceComponentIndex = connections.Find(referenceIndex);

  if (queryComponentIndex != referenceComponentIndex)
  {
    double distance = metric.Evaluate(dataSet.col(queryIndex),
                                      dataSet.col(referenceIndex));

    if (distance < neighborsDistances[queryComponentIndex])
    {
      Log::Assert(queryIndex != referenceIndex);

      neighborsDistances[queryComponentIndex] = distance;
      neighborsInComponent[queryComponentIndex] = queryIndex;
      neighborsOutComponent[queryComponentIndex] = referenceIndex;
    }
  }

  if (newUpperBound < neighborsDistances[queryComponentIndex])
    newUpperBound = neighborsDistances[queryComponentIndex];

  Log::Assert(newUpperBound >= 0.0);

  return newUpperBound;
}

template<typename MetricType, typename TreeType>
double DTBRules<MetricType, TreeType>::Score(const size_t queryIndex,
                                             TreeType& referenceNode)
{
  size_t queryComponentIndex = connections.Find(queryIndex);

  // If the query belongs to the same component as all of the references,
  // then prune.  The cast is to stop a warning about comparing unsigned to
  // signed values.
  if (queryComponentIndex ==
      (size_t) referenceNode.Stat().ComponentMembership())
    return DBL_MAX;

  const arma::vec queryPoint = dataSet.unsafe_col(queryIndex);
  const double distance = referenceNode.MinDistance(queryPoint);

  // If all the points in the reference node are farther than the candidate
  // nearest neighbor for the query's component, we prune.
  return neighborsDistances[queryComponentIndex] < distance
      ? DBL_MAX : distance;
}

template<typename MetricType, typename TreeType>
double DTBRules<MetricType, TreeType>::Score(const size_t queryIndex,
                                             TreeType& referenceNode,
                                             const double baseCaseResult)
{
  // I don't really understand the last argument here
  // It just gets passed in the distance call, otherwise this function
  // is the same as the one above.
  size_t queryComponentIndex = connections.Find(queryIndex);

  // If the query belongs to the same component as all of the references,
  // then prune.
  if (queryComponentIndex == referenceNode.Stat().ComponentMembership())
    return DBL_MAX;

  const arma::vec queryPoint = dataSet.unsafe_col(queryIndex);
  const double distance = referenceNode.MinDistance(queryPoint,
                                                    baseCaseResult);

  // If all the points in the reference node are farther than the candidate
  // nearest neighbor for the query's component, we prune.
  return (neighborsDistances[queryComponentIndex] < distance) ? DBL_MAX :
      distance;
}

template<typename MetricType, typename TreeType>
double DTBRules<MetricType, TreeType>::Rescore(const size_t queryIndex,
                                               TreeType& referenceNode,
                                               const double oldScore)
{
  // We don't need to check component membership again, because it can't
  // change inside a single iteration.
  return (oldScore > neighborsDistances[connections.Find(queryIndex)])
      ? DBL_MAX : oldScore;
}

template<typename MetricType, typename TreeType>
double DTBRules<MetricType, TreeType>::Score(TreeType& queryNode,
                                             TreeType& referenceNode) const
{
  // If all the queries belong to the same component as all the references
  // then we prune.
  if ((queryNode.Stat().ComponentMembership() >= 0) &&
      (queryNode.Stat().ComponentMembership() ==
           referenceNode.Stat().ComponentMembership()))
    return DBL_MAX;

  const double distance = queryNode.MinDistance(&referenceNode);
  const double bound = CalculateBound(queryNode);

  // If all the points in the reference node are farther than the candidate
  // nearest neighbor for all queries in the node, we prune.
  return (bound < distance) ? DBL_MAX : distance;
}

template<typename MetricType, typename TreeType>
double DTBRules<MetricType, TreeType>::Score(TreeType& queryNode,
                                             TreeType& referenceNode,
                                             const double baseCaseResult) const
{
  // If all the queries belong to the same component as all the references
  // then we prune.
  if ((queryNode.Stat().ComponentMembership() >= 0) &&
      (queryNode.Stat().ComponentMembership() ==
           referenceNode.Stat().ComponentMembership()))
    return DBL_MAX;

  const double distance = queryNode.MinDistance(referenceNode, baseCaseResult);
  const double bound = CalculateBound(queryNode);

  // If all the points in the reference node are farther than the candidate
  // nearest neighbor for all queries in the node, we prune.
  return (bound < distance) ? DBL_MAX : distance;
}

template<typename MetricType, typename TreeType>
double DTBRules<MetricType, TreeType>::Rescore(TreeType& queryNode,
                                               TreeType& /* referenceNode */,
                                               const double oldScore) const
{
  const double bound = CalculateBound(queryNode);
  return (oldScore > bound) ? DBL_MAX : oldScore;
}

// Calculate the bound for a given query node in its current state and update
// it.
template<typename MetricType, typename TreeType>
inline double DTBRules<MetricType, TreeType>::CalculateBound(
    TreeType& queryNode) const
{
  double worstPointBound = -DBL_MAX;
  double bestPointBound = DBL_MAX;

  double worstChildBound = -DBL_MAX;
  double bestChildBound = DBL_MAX;

  // Now, find the best and worst point bounds.
  for (size_t i = 0; i < queryNode.NumPoints(); ++i)
  {
    const size_t pointComponent = connections.Find(queryNode.Point(i));
    const double bound = neighborsDistances[pointComponent];

    if (bound > worstPointBound)
      worstPointBound = bound;
    if (bound < bestPointBound)
      bestPointBound = bound;
  }

  // Find the best and worst child bounds.
  for (size_t i = 0; i < queryNode.NumChildren(); ++i)
  {
    const double maxBound = queryNode.Child(i).Stat().MaxNeighborDistance();
    if (maxBound > worstChildBound)
      worstChildBound = maxBound;

    const double minBound = queryNode.Child(i).Stat().MinNeighborDistance();
    if (minBound < bestChildBound)
      bestChildBound = minBound;
  }

  // Now calculate the actual bounds.
  const double worstBound = std::max(worstPointBound, worstChildBound);
  const double bestBound = std::min(bestPointBound, bestChildBound);
  // We must check that bestBound != DBL_MAX; otherwise, we risk overflow.
  const double bestAdjustedBound = (bestBound == DBL_MAX) ? DBL_MAX :
      bestBound + 2 * queryNode.FurthestDescendantDistance();

  // Update the relevant quantities in the node.
  queryNode.Stat().MaxNeighborDistance() = worstBound;
  queryNode.Stat().MinNeighborDistance() = bestBound;
  queryNode.Stat().Bound() = std::min(worstBound, bestAdjustedBound);

  return queryNode.Stat().Bound();
}

}; // namespace emst
}; // namespace mlpack



#endif

