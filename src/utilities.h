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

#pragma once

#include "common.h"

typedef enum
{
	kOutputVariableIndexStockPriceAtMaturity		= 0,
	kOutputVariableIndexCallOptionPrice			= 1,
	kOutputVariableIndexPutOptionPrice			= 2,
	kOutputVariableIndexValueAtRisk				= 3,
	kOutputVariableIndexSimulatedReturns			= 4,
	kOutputVariableIndexMax,
} OutputVariableIndex;

typedef enum
{
	kArithmeticBrownianMotionConfigFrequencyIndexDays	= 0,
	kArithmeticBrownianMotionConfigFrequencyIndexMonths	= 1,
	kArithmeticBrownianMotionConfigFrequencyIndexYears	= 2,
	kArithmeticBrownianMotionConfigFrequencyIndexMax,
} ArithmeticBrownianMotionConfigFrequencyIndex;

typedef enum
{
	kArithmeticBrownianMotionConfigFrequencyDays		= 252,
	kArithmeticBrownianMotionConfigFrequencyMonths		= 12,
	kArithmeticBrownianMotionConfigFrequencyYears		= 1,
} ArithmeticBrownianMotionConfigFrequency;

typedef enum
{
#ifdef IS_SIMULATION
	kArithmeticBrownianMotionConfigDefaultNumberOfPaths	= 1,
	kArithmeticBrownianMotionConfigDefaultNumberOfSteps	= 1,
#else
	kArithmeticBrownianMotionConfigDefaultNumberOfPaths	= 25,
	kArithmeticBrownianMotionConfigDefaultNumberOfSteps	= 252,
#endif
} ArithmeticBrownianMotionConfigDefault;

#define kArithmeticBrownianMotionConfigPeriodicMeanReturn		(0.05)
#define kArithmeticBrownianMotionConfigInitialPortfolioValue		(5.0)
#define kArithmeticBrownianMotionConfigPeriodicVolatility		(0.4)
#define kArithmeticBrownianMotionConfigDefaultStrikePrice		(4.5)
#define kArithmeticBrownianMotionConfigDefaultRiskFreeInterestRate	(0.05)
#define kArithmeticBrownianMotionConfigDefaultStartDate			(0.0)
#define kArithmeticBrownianMotionConfigDefaultMaturityTime		(1.0)
#define kArithmeticBrownianMotionConfigDefaultQuantileProbability	(0.05)

static const char *	kApplicationDescription = "Oosterlee-Grzelak Book Arithmetic Brownian Motion";

typedef struct
{
	CommonCommandLineArguments	common;
	double				periodicMeanReturn;
	unsigned int			frequencyIndex;
	double				initialPortfolioValue;
	double				periodicVolatility;
	double				strikePrice;
	double				quantileProbability;
	double				maturityTime;
} CommandLineArguments;

/**
 *	@brief	Print out command-line usage.
 */
void	printUsage(void);

/**
 *	@brief	Get command-line arguments.
 *
 *	@param	argc		: argument count from `main()`.
 *	@param	argv		: argument vector from `main()`.
 *	@param	arguments	: Pointer to struct to store arguments.
 *	@return			: `kCommonConstantReturnTypeSuccess` if successful,
 *					else `kCommonConstantReturnTypeError`.
 */
CommonConstantReturnType getCommandLineArguments(int argc, char *  argv[], CommandLineArguments *  arguments);
