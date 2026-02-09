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

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>
#include <math.h>
#include <float.h>
#include <uxhw.h>
#include "utilities.h"
#include "arithmetic-brownian-motion.h"

/*
 *	Kernel for arithmetic Brownian motion, exploiting UxHw.
 */
double
arithmeticBrownianMotion(
	size_t		numberOfMonteCarloIterations,
	size_t		numberOfSteps,
	double		periodicVolatility,
	double		maturityTime,
	double		periodicMeanReturn,
	double		initialPortfolioValue,
	double *	benchmarkingDataSamples)
{
	/*
	 *	When running on UxHw-enabled platforms, all `float` and `double` values
	 *	have an associated distribution, in addition to their default semantics.
	 */
	double		dt;
	double		rDt;
	double		sqrtDt;
	double		Z;
	double		X;

	dt	= maturityTime / (double)numberOfSteps;
	sqrtDt	= sqrt(dt);
	rDt	= periodicMeanReturn * dt;

	/*
	 *	Monte Carlo simulation loop.
	 *	Standard Monte Carlo: Multiple independent paths (numberOfMonteCarloIterations > 1).
	 *	UxHw mode: Single path with distributional arithmetic (numberOfMonteCarloIterations == 1).
	 */
	for (size_t jj = 0; jj < numberOfMonteCarloIterations; jj++)
	{
		X = initialPortfolioValue;
		for (size_t ii = 0; ii < numberOfSteps; ii++)
		{
			/*
			 *	Standard normal white noise for Brownian motion increment.
			 *	The function UxHwDoubleGaussDist() returns a `double` that also has
			 *	associated with it a complete probability distribution.
			 */
			Z = UxHwDoubleGaussDist(0.0, 1.0);
			X = X + rDt + periodicVolatility * sqrtDt * Z;
		}
		benchmarkingDataSamples[jj] = X;
	}

	/*
	 *	Return the last sample. For UxHw execution
	 *	where `numberOfMonteCarloIterations == 1`, returns the single
	 *	distributional value.
	 */
	return benchmarkingDataSamples[numberOfMonteCarloIterations - 1];
}

double
callOptionPayoff(double stockPriceAtMaturity, double strikePrice)
{
	double	difference;
	double	truncated;
	double	strikeFraction;

	/*
	 *	We need to do two things here:
	 *	1.	Capture the part of the distribution that satisfies the strike price.
	 *		We achieve this by taking the part of the distribution with positive
	 *		support.
	 *	2.	Capture the fact that only a fraction of the paths satisfy the strike
	 *		price. We compute strikeFraction = UxHwDoubleProbabilityGT(difference, 0)
	 *		and either use that to make a mixture of strike-exceeding payoff and zero
	 *		payoff, or multiply the mean of the truncated distribution by that.
	 *		Note: Because of the current semantics of UxHwDoubleProbabilityGT(),
	 *		we have to make sure the distribution value is in the first argument.
	 */
	difference = stockPriceAtMaturity - strikePrice;
	truncated = UxHwDoubleLimitDistributionSupport(difference, 0, DBL_MAX);
	strikeFraction = UxHwDoubleProbabilityGT(difference, 0);

	return UxHwDoubleMixture(truncated, 0, strikeFraction);
}

double
putOptionPayoff(double stockPriceAtMaturity, double strikePrice)
{
	double	difference;
	double	truncated;
	double	strikeFraction;

	/*
	 *	We need to do two things here:
	 *	1.	Capture the part of the distribution that satisfies the strike price.
	 *		We achieve this by taking the part of the distribution with positive
	 *		support.
	 *	2.	Capture the fact that only a fraction of the paths satisfy the strike
	 *		price. We compute strikeFraction = UxHwDoubleProbabilityGT(difference, 0)
	 *		and either use that to make a mixture of strike-exceeding payoff and zero
	 *		payoff, or multiply the mean of the truncated distribution by that.
	 *		Note: Because of the current semantics of UxHwDoubleProbabilityGT(),
	 *		we have to make sure the distribution value is in the first argument.
	 */
	difference = strikePrice - stockPriceAtMaturity;
	truncated = UxHwDoubleLimitDistributionSupport(difference, 0, DBL_MAX);
	strikeFraction = UxHwDoubleProbabilityGT(difference, 0);

	return UxHwDoubleMixture(truncated, 0, strikeFraction);
}

double
computeSimulatedReturns(double stockPriceAtMaturity, double initialPortfolioValue)
{
	/*
	 *	Compute the simulated returns distribution by subtracting the initial value
	 *	from stockPriceAtMaturity.
	 */
	return stockPriceAtMaturity - initialPortfolioValue;
}

double
computeValueAtRisk(double simulatedReturns, double quantileProbability)
{
	/*
	 *	Capture the quantileProbability value at risk from the simulated returns.
	 */
	return UxHwDoubleQuantile(simulatedReturns, quantileProbability);
}
