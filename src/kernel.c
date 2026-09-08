/*
 *	Copyright (c) 2024-2026, Signaloid.
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */

#include <stddef.h>
#include <stdbool.h>
#include <math.h>
#include <uxhw.h>
#include "kernel.h"
#include "arithmetic-brownian-motion-uxhw.h"
#include "arithmetic-brownian-motion-monte-carlo.h"
#include "utilities.h"

double
arithmeticBrownianMotionSinglePath(
	size_t  numberOfSteps,
	double  periodicVolatility,
	double  maturityTime,
	double  periodicMeanReturn,
	double  initialPortfolioValue)
{
	double  dt;
	double  rDt;
	double  sqrtDt;
	double  Z;
	double  X;

	/*
	 *	Guard against a zero step count, which would otherwise produce a
	 *	non-finite `dt` and propagate inf/NaN through the path.
	 */
	if (numberOfSteps == 0)
	{
		numberOfSteps = 1;
	}

	dt      = maturityTime / (double) numberOfSteps;
	sqrtDt  = sqrt(dt);
	rDt     = periodicMeanReturn * dt;
	X       = initialPortfolioValue;

	for (size_t ii = 0 ; ii < numberOfSteps ; ii++)
	{
		/*
		 *	Standard normal white noise for Brownian motion increment.
		 *	The function UxHwDoubleGaussDist() returns a `double` that also has
		 *	associated with it a complete probability distribution.
		 */
		Z   = UxHwDoubleGaussDist(0.0, 1.0);
		X   = X + (rDt + periodicVolatility * sqrtDt * Z);
	}

	return X;
}

/*
 *	Translate the user-selected reporting frequency into a count of simulation
 *	time steps over the given maturity time.
 */
static size_t
numberOfStepsForFrequency(CommandLineArguments * arguments)
{
	double  stepsPerYear;
	double  steps;

	if (arguments->frequencyIndex == kArithmeticBrownianMotionConfigFrequencyIndexDays)
	{
		stepsPerYear = (double) kArithmeticBrownianMotionConfigFrequencyDays;
	}
	else if (arguments->frequencyIndex == kArithmeticBrownianMotionConfigFrequencyIndexMonths)
	{
		stepsPerYear = (double) kArithmeticBrownianMotionConfigFrequencyMonths;
	}
	else
	{
		stepsPerYear = (double) kArithmeticBrownianMotionConfigFrequencyYears;
	}

	/*
	 *	`maturityTime` is in years and is routinely fractional (the no-OS
	 *	build uses 2/252). Convert to an integer only after multiplying by
	 *	the step frequency, otherwise every sub-year maturity truncates to
	 *	zero steps. Round rather than truncate the product as well: a whole
	 *	number of steps can still land just below the integer in binary
	 *	floating point, so truncation would silently drop a step. For
	 *	example, a 4037-day maturity computes as 4036.9999999999995.
	 */
	steps = round(arguments->maturityTime * stepsPerYear);

	/*
	 *	Written as a negated `>=` so that a NaN maturity time also falls
	 *	through to the single-step floor below.
	 */
	if (!(steps >= 1.0))
	{
		steps = 1.0;
	}

	return (size_t) steps;
}

double
calculateOutputUxHw(
	CommandLineArguments *  arguments,
	double *                outputVariables,
	double *                monteCarloOutputSamples)
{
	double  result = 0.0;
	double  stockPriceAtMaturity;
	double  simulatedReturns;
	double  valueAtRisk;
	size_t  numberOfSteps;
	bool    calculateAllOutputs;

	calculateAllOutputs = (arguments->common.outputSelect == kOutputVariableIndexMax);
	numberOfSteps       = numberOfStepsForFrequency(arguments);

	stockPriceAtMaturity = arithmeticBrownianMotionUxHw(
		numberOfSteps,
		arguments->periodicVolatility,
		arguments->maturityTime,
		arguments->periodicMeanReturn,
		arguments->initialPortfolioValue
	);
	monteCarloOutputSamples[0] = stockPriceAtMaturity;

	if (calculateAllOutputs || arguments->common.outputSelect == kOutputVariableIndexStockPriceAtMaturity)
	{
		result = outputVariables[kOutputVariableIndexStockPriceAtMaturity] = stockPriceAtMaturity;
	}

	if (calculateAllOutputs || arguments->common.outputSelect == kOutputVariableIndexCallOptionPrice)
	{
		double payoff = callOptionPayoffUxHw(stockPriceAtMaturity, arguments->strikePrice);

		if (arguments->common.outputSelect == kOutputVariableIndexCallOptionPrice)
		{
			monteCarloOutputSamples[0] = payoff;
		}

		result = outputVariables[kOutputVariableIndexCallOptionPrice] = payoff;
	}

	if (calculateAllOutputs || arguments->common.outputSelect == kOutputVariableIndexPutOptionPrice)
	{
		double payoff = putOptionPayoffUxHw(stockPriceAtMaturity, arguments->strikePrice);

		if (arguments->common.outputSelect == kOutputVariableIndexPutOptionPrice)
		{
			monteCarloOutputSamples[0] = payoff;
		}

		result = outputVariables[kOutputVariableIndexPutOptionPrice] = payoff;
	}

	if (calculateAllOutputs
	    || arguments->common.outputSelect == kOutputVariableIndexSimulatedReturns
	    || arguments->common.outputSelect == kOutputVariableIndexValueAtRisk)
	{
		simulatedReturns            = computeSimulatedReturnsUxHw(stockPriceAtMaturity, arguments->initialPortfolioValue);
		monteCarloOutputSamples[0]  = simulatedReturns;

		if (calculateAllOutputs || arguments->common.outputSelect == kOutputVariableIndexSimulatedReturns)
		{
			result = outputVariables[kOutputVariableIndexSimulatedReturns] = simulatedReturns;
		}

		if (calculateAllOutputs || arguments->common.outputSelect == kOutputVariableIndexValueAtRisk)
		{
			valueAtRisk                 = computeValueAtRiskUxHw(simulatedReturns, arguments->quantileProbability);
			monteCarloOutputSamples[0]  = valueAtRisk;
			result = outputVariables[kOutputVariableIndexValueAtRisk] = valueAtRisk;
		}
	}

	return result;
}

double
calculateOutputMonteCarlo(
	CommandLineArguments *  arguments,
	double *                outputVariables,
	double *                monteCarloOutputSamples)
{
	double  result = 0.0;
	double  stockPriceAtMaturity;
	double  simulatedReturns;
	double  valueAtRisk;
	size_t  numberOfSteps;
	size_t  numberOfMonteCarloIterations;
	bool    calculateAllOutputs;

	calculateAllOutputs             = (arguments->common.outputSelect == kOutputVariableIndexMax);
	numberOfMonteCarloIterations    = arguments->common.numberOfMonteCarloIterations;
	numberOfSteps                   = numberOfStepsForFrequency(arguments);

	stockPriceAtMaturity = arithmeticBrownianMotionMonteCarlo(
		numberOfMonteCarloIterations,
		numberOfSteps,
		arguments->periodicVolatility,
		arguments->maturityTime,
		arguments->periodicMeanReturn,
		arguments->initialPortfolioValue,
		monteCarloOutputSamples
	);

	if (calculateAllOutputs || arguments->common.outputSelect == kOutputVariableIndexStockPriceAtMaturity)
	{
		result = outputVariables[kOutputVariableIndexStockPriceAtMaturity] = stockPriceAtMaturity;
	}

	if (calculateAllOutputs || arguments->common.outputSelect == kOutputVariableIndexCallOptionPrice)
	{
		/*
		 *	Overwrite samples with payoffs only when this is the selected
		 *	output; otherwise preserve the stock prices for the put and
		 *	returns sections.
		 */
		double * payoffsOut = (arguments->common.outputSelect == kOutputVariableIndexCallOptionPrice)
		                ? monteCarloOutputSamples
		                : NULL;
		double payoff = callOptionPayoffMonteCarlo(
			payoffsOut,
			monteCarloOutputSamples,
			numberOfMonteCarloIterations,
			arguments->strikePrice
		);

		result = outputVariables[kOutputVariableIndexCallOptionPrice] = payoff;
	}

	if (calculateAllOutputs || arguments->common.outputSelect == kOutputVariableIndexPutOptionPrice)
	{
		double * payoffsOut = (arguments->common.outputSelect == kOutputVariableIndexPutOptionPrice)
		                ? monteCarloOutputSamples
		                : NULL;
		double payoff = putOptionPayoffMonteCarlo(
			payoffsOut,
			monteCarloOutputSamples,
			numberOfMonteCarloIterations,
			arguments->strikePrice
		);

		result = outputVariables[kOutputVariableIndexPutOptionPrice] = payoff;
	}

	if (calculateAllOutputs
	    || arguments->common.outputSelect == kOutputVariableIndexSimulatedReturns
	    || arguments->common.outputSelect == kOutputVariableIndexValueAtRisk)
	{
		simulatedReturns = computeSimulatedReturnsMonteCarlo(
			monteCarloOutputSamples,
			monteCarloOutputSamples,
			numberOfMonteCarloIterations,
			arguments->initialPortfolioValue
		);

		if (calculateAllOutputs || arguments->common.outputSelect == kOutputVariableIndexSimulatedReturns)
		{
			result = outputVariables[kOutputVariableIndexSimulatedReturns] = simulatedReturns;
		}

		if (calculateAllOutputs || arguments->common.outputSelect == kOutputVariableIndexValueAtRisk)
		{
			valueAtRisk = computeValueAtRiskMonteCarlo(
				monteCarloOutputSamples,
				numberOfMonteCarloIterations,
				arguments->quantileProbability
			);

			monteCarloOutputSamples[0] = valueAtRisk;

			result = outputVariables[kOutputVariableIndexValueAtRisk] = valueAtRisk;
		}
	}

	return result;
}
