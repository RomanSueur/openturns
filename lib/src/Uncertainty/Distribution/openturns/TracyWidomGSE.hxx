//                                               -*- C++ -*-
/**
 *  @brief The TracyWidomGSE distribution
 *
 *  Copyright 2005-2017 Airbus-EDF-IMACS-Phimeca
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef OPENTURNS_TRACYWIDOMGSE_HXX
#define OPENTURNS_TRACYWIDOMGSE_HXX

#include "openturns/ContinuousDistribution.hxx"

BEGIN_NAMESPACE_OPENTURNS

/**
 * @class TracyWidomGSE
 *
 * The TracyWidomGSE distribution.
 */
class OT_API TracyWidomGSE
  : public ContinuousDistribution
{
  CLASSNAME
public:

  /** Default constructor */
  TracyWidomGSE();

  /** Comparison operator */
  Bool operator ==(const TracyWidomGSE & other) const;
protected:
  Bool equals(const DistributionImplementation & other) const;
public:

  /** String converter */
  String __repr__() const;



  /* Interface inherited from Distribution */

  /** Virtual constructor */
  virtual TracyWidomGSE * clone() const;

  /** Get the DDF of the distribution */
  using ContinuousDistribution::computeDDF;
  Point computeDDF(const Point & point) const;

  /** Get the PDF of the distribution */
  using ContinuousDistribution::computePDF;
  Scalar computePDF(const Point & point) const;

  /** Get the CDF of the distribution */
  using ContinuousDistribution::computeCDF;
  Scalar computeCDF(const Point & point) const;

  /** Get the PDFGradient of the distribution */
  Point computePDFGradient(const Point & point) const;

  /** Get the CDFGradient of the distribution */
  Point computeCDFGradient(const Point & point) const;

  /** Get the roughness, i.e. the L2-norm of the PDF */
  Scalar getRoughness() const;

  /** Get the standard deviation of the distribution */
  Point getStandardDeviation() const;

  /** Get the skewness of the distribution */
  Point getSkewness() const;

  /** Get the kurtosis of the distribution */
  Point getKurtosis() const;

  /** Parameters value and description accessor */
  PointWithDescriptionCollection getParametersCollection() const;

  /** Method save() stores the object through the StorageManager */
  void save(Advocate & adv) const;

  /** Method load() reloads the object from the StorageManager */
  void load(Advocate & adv);


protected:


private:

  /** Compute the mean of the distribution */
  void computeMean() const;

  /** Compute the covariance of the distribution */
  void computeCovariance() const;

  /** Get the quantile of the distribution */
  Scalar computeScalarQuantile(const Scalar prob,
                               const Bool tail = false) const;

}; /* class TracyWidomGSE */


END_NAMESPACE_OPENTURNS

#endif /* OPENTURNS_TRACYWIDOMGSE_HXX */