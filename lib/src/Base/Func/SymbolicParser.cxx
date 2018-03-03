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

#include "openturns/SymbolicParser.hxx"
#include "openturns/SymbolicParserExprTk.hxx"
#ifdef OPENTURNS_HAVE_MUPARSER
#include "openturns/SymbolicParserMuParser.hxx"
#endif
#include "openturns/ResourceMap.hxx"

BEGIN_NAMESPACE_OPENTURNS


CLASSNAMEINIT(SymbolicParser)

/* Default constructor */
SymbolicParser::SymbolicParser()
  : TypedInterfaceObject<SymbolicParserImplementation>()
{
  String name = ResourceMap::Get("SymbolicParser-Backend");
  if (name == "ExprTk")
    p_implementation_ = new SymbolicParserExprTk();
#ifdef OPENTURNS_HAVE_MUPARSER
  else if (name == "MuParser")
    p_implementation_ = new SymbolicParserMuParser();
#endif
  else
    throw InvalidArgumentException(HERE) << "Error: invalid value for symbolic parser: " << name;
}

/* Constructor with parameter */
SymbolicParser::SymbolicParser(const Description & outputVariablesNames)
  : TypedInterfaceObject<SymbolicParserImplementation>()
{
  String name = ResourceMap::Get("SymbolicParser-Backend");
  if (name == "ExprTk")
    p_implementation_ = new SymbolicParserExprTk(outputVariablesNames);
#ifdef OPENTURNS_HAVE_MUPARSER
  else if (name == "MuParser")
    p_implementation_ = new SymbolicParserMuParser(outputVariablesNames);
#endif
  else
    throw InvalidArgumentException(HERE) << "Error: invalid value for symbolic parser: " << name;
}

/* Constructor with parameters */
SymbolicParser::SymbolicParser(const SymbolicParserImplementation & implementation)
  : TypedInterfaceObject<SymbolicParserImplementation>(implementation.clone())
{
  // Nothing to do
}

/* Constructor with parameters */
SymbolicParser::SymbolicParser(Implementation & p_implementation)
  : TypedInterfaceObject<SymbolicParserImplementation>(p_implementation)
  {
  // Nothing to do
}

Description SymbolicParser::getVariables() const
{
  return getImplementation()->getVariables();
}

void SymbolicParser::setVariables(const Description & inputVariablesNames)
{
  getImplementation()->setVariables(inputVariablesNames);
}

void SymbolicParser::setImplicitOutputVariables(const Bool implicitOutputVariables)
{
  getImplementation()->setImplicitOutputVariables(implicitOutputVariables);
}

Description SymbolicParser::getFormulas() const
{
  return getImplementation()->getFormulas();
}

void SymbolicParser::setFormulas(const Description & formulas)
{
  getImplementation()->setFormulas(formulas);
}

Point SymbolicParser::operator() (const Point & inP) const
{
  return getImplementation()->operator()(inP);
}

Sample SymbolicParser::operator() (const Sample & inS) const
{
  return getImplementation()->operator()(inS);
}


END_NAMESPACE_OPENTURNS
