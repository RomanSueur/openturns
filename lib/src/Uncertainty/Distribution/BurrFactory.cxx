//                                               -*- C++ -*-
/**
 *  @brief Factory for Burr distribution
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
#include "openturns/BurrFactory.hxx"
#include "openturns/MethodBoundEvaluation.hxx"
#include "openturns/SpecFunc.hxx"
#include "openturns/Brent.hxx"
#include "openturns/Point.hxx"
#include "openturns/PersistentObjectFactory.hxx"

BEGIN_NAMESPACE_OPENTURNS

CLASSNAMEINIT(BurrFactory);

static const Factory<BurrFactory> Factory_BurrFactory;

/* Default constructor */
BurrFactory::BurrFactory()
  : DistributionFactoryImplementation()
{
  // Nothing to do
}

/* Virtual constructor */
BurrFactory * BurrFactory::clone() const
{
  return new BurrFactory(*this);
}

struct BurrFactoryParameterConstraint
{
  /** Constructor from a sample and a derivative factor estimate */
  BurrFactoryParameterConstraint(const Sample & sample):
    sample_(sample)
  {
    // Nothing to do
  };

  Point computeConstraint(const Point & parameter) const
  {
    const NumericalScalar c = parameter[0];
    if (!(c > 0.0)) throw InvalidArgumentException(HERE) << "Error: the c parameter must be positive.";
    const UnsignedInteger size = sample_.getSize();
    /* \sum_{i=1}^N \log(1 + x_i^c) */
    NumericalScalar sumLogXC = 0.0;
    /* \sum_{i=1}^N \frac{\log(x_i)}{1+x_i^c} */
    NumericalScalar sumRatio = 0.0;
    /* \sum_{i=1}^N \frac{x_i^c\log(x_i)}{1+x_i^c} */
    NumericalScalar sumScaledRatio = 0.0;
    for (UnsignedInteger i = 0; i < size; ++i)
    {
      const NumericalScalar x = sample_[i][0];
      const NumericalScalar xC = std::pow(x , c);
      const NumericalScalar logX = std::log(x);
      const NumericalScalar ratio = logX / (1.0 + xC);
      sumRatio += ratio;
      sumScaledRatio += xC * ratio;
      sumLogXC += log1p(xC);
    }
    /* MLE second parameter */
    const NumericalScalar k = size / sumLogXC;
    if (!SpecFunc::IsNormal(k)) throw InvalidArgumentException(HERE) << "Error: cannot estimate the k parameter";

    /* MLE equation first parameter */
    const NumericalScalar relation = 1.0 + (c / size) * (sumRatio - k * sumScaledRatio);
    return Point(1, relation);
  }

  const Sample & sample_;
};


/* Here is the interface that all derived class must implement */

BurrFactory::Implementation BurrFactory::build(const Sample & sample) const
{
  return buildAsBurr(sample).clone();
}

BurrFactory::Implementation BurrFactory::build(const Point & parameters) const
{
  return buildAsBurr(parameters).clone();
}

BurrFactory::Implementation BurrFactory::build() const
{
  return buildAsBurr().clone();
}

Burr BurrFactory::buildAsBurr(const Sample & sample) const
{
  const UnsignedInteger size = sample.getSize();
  if (size == 0) throw InvalidArgumentException(HERE) << "Error: cannot build a Burr distribution from an empty sample";
  if (sample.getDimension() != 1) throw InvalidArgumentException(HERE) << "Error: can build a Burr distribution only from a sample of dimension 1, here dimension=" << sample.getDimension();

  if (!(sample.getMin()[0] > 0.0)) throw InvalidArgumentException(HERE) << "Error: cannot build a Burr distribution based on a sample with nonpositive values.";
  BurrFactoryParameterConstraint constraint(sample);
  const Function f(bindMethod<BurrFactoryParameterConstraint, Point, Point>(constraint, &BurrFactoryParameterConstraint::computeConstraint, 1, 1));
  // Find a bracketing interval
  NumericalScalar a = 1.0;
  NumericalScalar b = 2.0;
  NumericalScalar fA = f(Point(1, a))[0];
  NumericalScalar fB = f(Point(1, b))[0];
  // While f has the same sign at the two bounds, update the interval
  while ((fA * fB > 0.0))
  {
    a = 0.5 * a;
    fA = f(Point(1, a))[0];
    if (fA * fB <= 0.0) break;
    b = 2.0 * b;
    fB = f(Point(1, b))[0];
  }
  // Solve the constraint equation
  Brent solver(ResourceMap::GetAsScalar( "BurrFactory-AbsolutePrecision" ), ResourceMap::GetAsScalar( "BurrFactory-RelativePrecision" ), ResourceMap::GetAsScalar( "BurrFactory-ResidualPrecision" ), ResourceMap::GetAsUnsignedInteger( "BurrFactory-MaximumIteration" ));
  // C estimate
  const NumericalScalar c = solver.solve(f, 0.0, a, b, fA, fB);
  // Corresponding k estimate
  NumericalScalar sumLogXC = 0.0;
  for (UnsignedInteger i = 0; i < size; ++i) sumLogXC += log1p(std::pow(sample[i][0], c));
  const NumericalScalar k = size / sumLogXC;
  Burr result(c, k);
  result.setDescription(sample.getDescription());
  return result;
}

Burr BurrFactory::buildAsBurr(const Point & parameters) const
{
  try
  {
    Burr distribution;
    distribution.setParameter(parameters);
    return distribution;
  }
  catch (InvalidArgumentException)
  {
    throw InvalidArgumentException(HERE) << "Error: cannot build a Burr distribution from the given parameters";
  }
}

Burr BurrFactory::buildAsBurr() const
{
  return Burr();
}

END_NAMESPACE_OPENTURNS
