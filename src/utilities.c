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

#include <stdio.h>
#include <stdlib.h>
#include "utilities.h"

void
printUsage(void)
{
	fprintf(stderr, "Example: Arithmetic Brownian Motion Application Use Case - Signaloid version\n");
	fprintf(stderr, "Usage: Valid command-line arguments are:\n");
	fprintf(
		stderr,
		"\t[-o, --output <Path to output CSV file : str>] (Specify the output file.)\n"
		"\t[-S, --select-output <output : int (Default: %d)>] (Compute 0-indexed output. Calculate all possible outputs if equal to %d.)\n"
		"\t[-M, --multiple-executions <Number of executions : int (Default: 1)>] (Repeated execute kernel for benchmarking.)\n"
		"\t[-T, --time] (Timing mode: Times and prints the timing of the kernel execution.)\n"
		"\t[-b, --benchmarking] (Benchmarking mode: Generate outputs in format for benchmarking.)\n"
		"\t[-j, --json] (Print output in JSON format.)\n"
		"\t[-h, --help] (Display this help message.)\n"
		"\t[--mean-return, --periodic-mean-return <mean return : double (Default: %lf)>] (Periodic mean return.)\n"
		"\t[--frequency, --frequency-index <frequency : int (Default: %d)>] (Time step frequency: 0=daily, 1=monthly, 2=yearly.)\n"
		"\t[--initial-value, --initial-portfolio-value <initial value : double (Default: %lf)>] (Initial portfolio value.)\n"
		"\t[--volatility, --periodic-volatility <volatility : double (Default: %lf)>] (Periodic volatility.)\n"
		"\t[--strike, --strike-price <strike price : double (Default: %lf)>] (Strike price for options.)\n"
		"\t[--quantile, --quantile-probability <quantile : double (Default: %lf)>] (Quantile probability for Value at Risk.)\n"
		"\t[--maturity-time, --maturity-time-years <maturity time : double (Default: %lf)>] (Simulation time horizon in years.)\n",
		kOutputVariableIndexMax,
		kOutputVariableIndexMax,
		kArithmeticBrownianMotionConfigPeriodicMeanReturn,
		kArithmeticBrownianMotionConfigFrequencyIndexDays,
		kArithmeticBrownianMotionConfigInitialPortfolioValue,
		kArithmeticBrownianMotionConfigPeriodicVolatility,
		kArithmeticBrownianMotionConfigDefaultStrikePrice,
		kArithmeticBrownianMotionConfigDefaultQuantileProbability,
		kArithmeticBrownianMotionConfigDefaultMaturityTime
	);
	fprintf(stderr, "\n");

	return;
}

/**
 *	@brief	Set the default values for the command-line arguments.
 *
 *	@param	arguments	: command-line arguments pointer.
 */
static void
setDefaultCommandLineArguments(CommandLineArguments *  arguments)
{
	/*
	 *	Older GCC versions have a bug which gives a spurious warning for
	 *	the C universal zero initializer `{0}`. Any workaround makes the
	 *	code less portable or prevents the common code from adding new
	 *	fields to the `CommonCommandLineArguments` struct. Therefore, we
	 *	suppress this warning.
	 *
	 *	See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53119.
	 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"

	*arguments = (CommandLineArguments)
	{
		.common			= (CommonCommandLineArguments) {0},
		.periodicMeanReturn	= kArithmeticBrownianMotionConfigPeriodicMeanReturn,
		.frequencyIndex		= kArithmeticBrownianMotionConfigFrequencyIndexDays,
		.initialPortfolioValue	= kArithmeticBrownianMotionConfigInitialPortfolioValue,
		.periodicVolatility	= kArithmeticBrownianMotionConfigPeriodicVolatility,
		.strikePrice		= kArithmeticBrownianMotionConfigDefaultStrikePrice,
		.quantileProbability	= kArithmeticBrownianMotionConfigDefaultQuantileProbability,
		.maturityTime		= kArithmeticBrownianMotionConfigDefaultMaturityTime,
	};
#pragma GCC diagnostic pop

	return;
}

CommonConstantReturnType
getCommandLineArguments(
	int			argc,
	char *			argv[],
	CommandLineArguments *	arguments)
{
	const char *	periodicMeanReturnArg		= NULL;
	const char * 	initialPortfolioValueArg	= NULL;
	const char *	periodicVolatilityArg		= NULL;
	const char *	strikePriceArg			= NULL;
	const char *	quantileProbabilityArg 		= NULL;
	const char *	maturityTimeArg			= NULL;
	const char *	frequencyIndexArg		= NULL;

	DemoOption demoSpecificOptions[] = {
		{ .opt = "mean-return",		.optAlternative = "periodic-mean-return",	.hasArg = true, .foundArg = &periodicMeanReturnArg,	.foundOpt = NULL },
		{ .opt = "initial-value", 	.optAlternative = "initial-portfolio-value", 	.hasArg = true, .foundArg = &initialPortfolioValueArg,	.foundOpt = NULL },
		{ .opt = "volatility",		.optAlternative = "periodic-volatility",	.hasArg = true, .foundArg = &periodicVolatilityArg,	.foundOpt = NULL },
		{ .opt = "strike",		.optAlternative = "strike-price",		.hasArg = true, .foundArg = &strikePriceArg,		.foundOpt = NULL },
		{ .opt = "quantile", 		.optAlternative = "quantile-probability",	.hasArg = true, .foundArg = &quantileProbabilityArg,	.foundOpt = NULL },
		{ .opt = "maturity-time",	.optAlternative = "maturity-time-years",	.hasArg = true, .foundArg = &maturityTimeArg,		.foundOpt = NULL },
		{ .opt = "frequency",		.optAlternative = "frequency-index",		.hasArg = true, .foundArg = &frequencyIndexArg,		.foundOpt = NULL },
		{0},
	};

	if (arguments == NULL)
	{
		fprintf(stderr, "Arguments pointer is NULL.\n");

		return kCommonConstantReturnTypeError;
	}

	setDefaultCommandLineArguments(arguments);

	/*
	 *	This application example has no application specific arguments.
	 */
	if (parseArgs(argc, argv, &arguments->common, demoSpecificOptions) != 0)
	{
		fprintf(stderr, "Parsing command-line arguments failed\n");
		printUsage();

		return kCommonConstantReturnTypeError;
	}

	/*
	 *	Process command-line arguments
	 */
	if (arguments->common.isHelpEnabled)
	{
		printUsage();

		exit(EXIT_SUCCESS);
	}

	if (arguments->common.isInputFromFileEnabled)
	{
		fprintf(stderr, "Reading inputs from CSV file is not currently supported\n");

		return kCommonConstantReturnTypeError;
	}

	/*
	 *	Write to output file is not supported in MonteCarlo Mode.
	 */
	if (arguments->common.isWriteToFileEnabled && arguments->common.isMonteCarloMode)
	{
		fprintf(stderr, "Writing to output file is not supported in MonteCarlo Mode.\n");

		return kCommonConstantReturnTypeError;
	}

	if (arguments->common.isVerbose)
	{
		fprintf(stderr, "Warning: Verbose mode not supported. Continuing in non-verbose mode.\n");
	}

	/*
	 *	If no output selected from CLA, set the print all value as default.
	 */
	if (!arguments->common.isOutputSelected)
	{
		arguments->common.outputSelect = kOutputVariableIndexMax;
	}

	/*
	 *	If a single output is selected, we must be in benchmarking mode or Monte Carlo mode.
	 */
	if (arguments->common.outputSelect > kOutputVariableIndexMax)
	{
		fprintf(
			stderr,
			"Output select value (-S option) is greater than the possible number of outputs: Provided %zd. Max: %d\n",
			arguments->common.outputSelect,
			kOutputVariableIndexMax);

		return kCommonConstantReturnTypeError;
	}
	/*
	 *	When all outputs are selected, we cannot be in benchmarking mode or Monte Carlo mode.
	 */
	else if (arguments->common.outputSelect == kOutputVariableIndexMax)
	{
		if ((arguments->common.isBenchmarkingMode) || (arguments->common.isMonteCarloMode))
		{
			fprintf(stderr, "Error: Please select a single output when in benchmarking mode or Monte Carlo mode.\n");
			return kCommonConstantReturnTypeError;
		}
	}

	if (periodicMeanReturnArg != NULL)
	{
		double periodicMeanReturn;
		if (parseDoubleChecked(periodicMeanReturnArg, &periodicMeanReturn) != kCommonConstantReturnTypeSuccess)
		{
			fprintf(stderr, "Error: The periodic mean return (--mean-return) must be a real number.\n");
			printUsage();
			return kCommonConstantReturnTypeError;
		}
		arguments->periodicMeanReturn = periodicMeanReturn;
	}

	if (initialPortfolioValueArg != NULL)
	{
		double initialPortfolioValue;
		if (parseDoubleChecked(initialPortfolioValueArg, &initialPortfolioValue) != kCommonConstantReturnTypeSuccess)
		{
			fprintf(stderr, "Error: The initial portfolio value (--initial-value) must be a real number.\n");
			printUsage();
			return kCommonConstantReturnTypeError;
		}
		arguments->initialPortfolioValue = initialPortfolioValue;
	}

	if (periodicVolatilityArg != NULL)
	{
		double periodicVolatility;
		if (parseDoubleChecked(periodicVolatilityArg, &periodicVolatility) != kCommonConstantReturnTypeSuccess)
		{
			fprintf(stderr, "Error: The periodic volatility (--volatility) must be a real number.\n");
			printUsage();
			return kCommonConstantReturnTypeError;
		}
		arguments->periodicVolatility = periodicVolatility;
	}

	if (strikePriceArg != NULL)
	{
		double strikePrice;
		if (parseDoubleChecked(strikePriceArg, &strikePrice) != kCommonConstantReturnTypeSuccess)
		{
			fprintf(stderr, "Error: The strike price (--strike) must be a real number.\n");
			printUsage();
			return kCommonConstantReturnTypeError;
		}
		arguments->strikePrice = strikePrice;
	}

	if (quantileProbabilityArg != NULL)
	{
		double quantileProbability;
		if (parseDoubleChecked(quantileProbabilityArg, &quantileProbability) != kCommonConstantReturnTypeSuccess)
		{
			fprintf(stderr, "Error: The quantile probability parameter (--quantile) must be a real number.\n");
			printUsage();
			return kCommonConstantReturnTypeError;
		}
		if (quantileProbability <= 0.0 || quantileProbability >= 1.0)
		{
			fprintf(stderr, "Error: The quantile probability parameter (--quantile) must be in the range (0, 1).\n");
			printUsage();
			return kCommonConstantReturnTypeError;
		}
		arguments->quantileProbability = quantileProbability;
	}

	if (maturityTimeArg != NULL)
	{
		double maturityTime;
		if (parseDoubleChecked(maturityTimeArg, &maturityTime) != kCommonConstantReturnTypeSuccess)
		{
			fprintf(stderr, "Error: The maturity time (--maturity-time) must be a real number.\n");
			printUsage();
			return kCommonConstantReturnTypeError;
		}
		if (maturityTime <= 0.0)
		{
			fprintf(stderr, "Error: The maturity time (--maturity-time) must be positive.\n");
			printUsage();
			return kCommonConstantReturnTypeError;
		}
		arguments->maturityTime = maturityTime;
	}

	if (frequencyIndexArg != NULL)
	{
		int frequencyIndex;
		if (parseIntChecked(frequencyIndexArg, &frequencyIndex) != kCommonConstantReturnTypeSuccess)
		{
			fprintf(stderr, "Error: The frequency index (--frequency) must be an integer.\n");
			printUsage();
			return kCommonConstantReturnTypeError;
		}
		if (frequencyIndex < 0 || frequencyIndex >= kArithmeticBrownianMotionConfigFrequencyIndexMax)
		{
			fprintf(stderr, "Error: The frequency index (--frequency) must be 0 (daily), 1 (monthly), or 2 (yearly).\n");
			printUsage();
			return kCommonConstantReturnTypeError;
		}
		arguments->frequencyIndex = (unsigned int)frequencyIndex;
	}

	return kCommonConstantReturnTypeSuccess;
}
