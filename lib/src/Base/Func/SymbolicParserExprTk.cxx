//                                               -*- C++ -*-
/**
 *  @brief A math expression parser
 *
 *  Copyright 2005-2018 Airbus-EDF-IMACS-Phimeca
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

#include "openturns/SymbolicParserExprTk.hxx"
#include "openturns/PersistentObjectFactory.hxx"
#include "openturns/SpecFunc.hxx"
#include "openturns/exprtk.hpp"

typedef exprtk::results_context<OT::Scalar> results_context_t;
typedef typename results_context_t::type_store_t type_t;

BEGIN_NAMESPACE_OPENTURNS

CLASSNAMEINIT(SymbolicParserExprTk);

static const Factory<SymbolicParserExprTk> Factory_SymbolicParserExprTk;

/* Default constructor */
SymbolicParserExprTk::SymbolicParserExprTk()
  : SymbolicParserImplementation()
{
  // Nothing to do
}

/* Constructor with parameter */
SymbolicParserExprTk::SymbolicParserExprTk(const Description & outputVariablesNames)
  : SymbolicParserImplementation(outputVariablesNames)
{
  // Nothing to do
}

/* Virtual constructor */
SymbolicParserExprTk * SymbolicParserExprTk::clone() const
{
  return new SymbolicParserExprTk(*this);
}

/* Evaluation operator */
Point SymbolicParserExprTk::operator()(const Point & inP) const
{
  const UnsignedInteger inputDimension(inputVariablesNames_.getSize());
  if (inP.getDimension() != inputDimension)
    throw InvalidArgumentException(HERE) << "Error: invalid input dimension (" << inP.getDimension() << ") expected " << inputDimension;
  const UnsignedInteger outputDimension(outputVariablesNames_.getSize() > 0 ? outputVariablesNames_.getSize() : formulas_.getSize());
  if (outputDimension == 0) return Point();
  initialize();
  std::copy(inP.begin(), inP.end(), inputStack_.begin());
  Point result(outputDimension);
  if (outputVariablesNames_.getSize() == 0)
  {
    // One formula by marginal
    for (UnsignedInteger outputIndex = 0; outputIndex < result.getDimension(); ++ outputIndex)
    {
      const Scalar value = expressions_[outputIndex]->value();
      // ExprTk does not throw on domain/division errors
      if (!SpecFunc::IsNormal(value))
        throw InternalException(HERE) << "Cannot evaluate " << formulas_[outputIndex] << " at " << inputVariablesNames_.__str__() << "=" << inP.__str__();
      result[outputIndex] = value;
    }
  }
  else
  {
    // Single formula, evaluate expression
    (void) expressions_[0]->value();

    if (implicitOutputVariables_)
    {
      const results_context_t & results = expressions_[0]->results();
      if (!(results.count() == outputVariablesNames_.getSize()))
        throw InternalException(HERE) << "Expected " << outputVariablesNames_.getSize() << " outputs, got " << results.count() << " at " << inputVariablesNames_.__str__() << "=" << inP.__str__();
      for (UnsignedInteger outputIndex = 0; outputIndex < results.count(); ++ outputIndex)
      {
        if (!(results[outputIndex].type == type_t::e_scalar))
          throw InternalException(HERE) << "Expected scalar output, got type " << results[outputIndex].type;
        const Scalar value = type_t::scalar_view(results[outputIndex])();
        // ExprTk does not throw on domain/division errors
        if (!SpecFunc::IsNormal(value))
          throw InternalException(HERE) << "Cannot evaluate " << formulas_[0] << " at " << inputVariablesNames_.__str__() << "=" << inP.__str__();
        result[outputIndex] = value;
      }
    }
    else
    {
      std::copy(inputStack_.begin() + inputDimension, inputStack_.end(), result.begin());
      for (UnsignedInteger outputIndex = 0; outputIndex < outputDimension; ++ outputIndex)
      {
        if (!SpecFunc::IsNormal(result[outputIndex]))
          throw InternalException(HERE) << "Cannot evaluate " << formulas_[0] << " at " << inputVariablesNames_.__str__() << "=" << inP.__str__();
      }
    }
  }
  return result;
}

Sample SymbolicParserExprTk::operator() (const Sample & inS) const
{
  const UnsignedInteger inputDimension = inputVariablesNames_.getSize();
  if (inS.getDimension() != inputDimension)
    throw InvalidArgumentException(HERE) << "Error: invalid input dimension (" << inS.getDimension() << ") expected " << inputDimension;
  const UnsignedInteger outputDimension(outputVariablesNames_.getSize() > 0 ? outputVariablesNames_.getSize() : formulas_.getSize());
  if (outputDimension == 0) return Sample(inS.getSize(), 0);
  initialize();
  const UnsignedInteger size = inS.getSize();
  Sample result(size, outputDimension);
  if (outputVariablesNames_.getSize() == 0)
  {
    // One formula by marginal
    for (UnsignedInteger i = 0; i < size; ++i)
    {
      std::copy(&inS(i, 0), &inS(i, inputDimension), inputStack_.begin());

      for (UnsignedInteger outputIndex = 0; outputIndex < outputDimension; ++ outputIndex)
      {
        const Scalar value = expressions_[outputIndex]->value();
        // ExprTk does not throw on domain/division errors
        if (!SpecFunc::IsNormal(value))
          throw InternalException(HERE) << "Cannot evaluate " << formulas_[outputIndex] << " at " << inputVariablesNames_.__str__() << "=" << Point(inS[i]).__str__();
        result(i, outputIndex) = value;
      }
    }
  }
  else
  {
    // Single formula
    for (UnsignedInteger i = 0; i < size; ++i)
    {
      std::copy(&inS(i, 0), &inS(i, inputDimension), inputStack_.begin());
      // Evaluate expression
      (void) expressions_[0]->value();

      if (implicitOutputVariables_)
      {
        const results_context_t & results = expressions_[0]->results();
        if (!(results.count() == outputVariablesNames_.getSize()))
          throw InternalException(HERE) << "Expected " << outputVariablesNames_.getSize() << " outputs, got " << results.count() << " at " << inputVariablesNames_.__str__() << "=" << Point(inS[i]).__str__();
        for (UnsignedInteger outputIndex = 0; outputIndex < results.count(); ++ outputIndex)
        {
          if (!(results[outputIndex].type == type_t::e_scalar))
            throw InternalException(HERE) << "Expected scalar output, got type " << results[outputIndex].type;
          const Scalar value = type_t::scalar_view(results[outputIndex])();
          // ExprTk does not throw on domain/division errors
          if (!SpecFunc::IsNormal(value))
            throw InternalException(HERE) << "Cannot evaluate " << formulas_[0] << " at " << inputVariablesNames_.__str__() << "=" << Point(inS[i]).__str__();
          result(i, outputIndex) = value;
        }
      }
      else
      {
        std::copy(inputStack_.begin() + inputDimension, inputStack_.end(), &result(i, 0));
        for (UnsignedInteger outputIndex = 0; outputIndex < outputDimension; ++ outputIndex)
        {
          if (!SpecFunc::IsNormal(result(i, outputIndex)))
            throw InternalException(HERE) << "Cannot evaluate " << formulas_[0] << " at " << inputVariablesNames_.__str__() << "=" << Point(inS[i]).__str__();
        }
      }
    }
  }
  return result;
}

namespace {
Scalar ExprTk_sign(Scalar v) {
  if (v > 0.0) return 1.0; else if (v < 0.0) return -1.0; else return 0.0;
}
Scalar ExprTk_rint(Scalar v) {
  return ((v < 0.0) ? std::ceil(v - 0.5) : std::floor(v + 0.5));
}
}

/* Method that instantiate the parsers */
void SymbolicParserExprTk::initialize() const
{
  const UnsignedInteger numberOfParsers(formulas_.getSize());
  if (expressions_.getSize() == numberOfParsers) return;
  expressions_ = Collection<Pointer<exprtk::expression<Scalar> > >(numberOfParsers);
  const UnsignedInteger inputDimension(inputVariablesNames_.getSize());
  // outputDimension is non-zero only if implicitOutputVariables_ is false
  const UnsignedInteger outputDimension(outputVariablesNames_.getSize());
  inputStack_ = Point(inputDimension + outputDimension);
  exprtk::symbol_table<Scalar> symbol_table;
  symbol_table.add_constants();
  symbol_table.add_constant("e_", 2.71828182845904523536028747135266249775724709369996);
  symbol_table.add_constant("pi_", 3.14159265358979323846264338327950288419716939937510);
  symbol_table.add_function("sign", ExprTk_sign);
  symbol_table.add_function("rint", ExprTk_rint);
  symbol_table.add_function("ln", std::log);
  symbol_table.add_function("lngamma", lgamma);
  symbol_table.add_function("gamma", SpecFunc::Gamma);
  symbol_table.add_function("cbrt", cbrt);
  symbol_table.add_function("besselJ0", j0);
  symbol_table.add_function("besselJ1", j1);
  symbol_table.add_function("besselY0", y0);
  symbol_table.add_function("besselY1", y1);
  for (UnsignedInteger inputIndex = 0; inputIndex < inputDimension; ++ inputIndex)
  {
    symbol_table.add_variable(inputVariablesNames_[inputIndex], inputStack_[inputIndex]);
  }
  if (!implicitOutputVariables_)
  {
    for (UnsignedInteger outputIndex = 0; outputIndex < outputDimension; ++ outputIndex)
    {
      symbol_table.add_variable(outputVariablesNames_[outputIndex], inputStack_[inputDimension + outputIndex]);
    }
  }
  exprtk::parser<Scalar> parser;
  // For each parser of a formula, do
  for (UnsignedInteger outputIndex = 0; outputIndex < numberOfParsers; ++ outputIndex)
  {
    exprtk::expression<Scalar> expression;
    expression.register_symbol_table(symbol_table);
    if (!parser.compile(formulas_[outputIndex], expression))
    {
      throw InvalidArgumentException(HERE) << "Errors found when parsing expression '" << formulas_[outputIndex] << "': " << parser.error();
    }
    expressions_[outputIndex] = new exprtk::expression<Scalar>(expression);
  }
}

END_NAMESPACE_OPENTURNS
