/*
 *	Copyright (c) 2026, Signaloid.
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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <uxhw.h>
#include "utilities.h"
#include "kernel.h"


int
main(int argc, char *  argv[])
{
	CommandLineArguments    arguments               = (CommandLineArguments) { 0 };
	const char *            applicationDescription  =
		"Propagates input uncertainty through energy and nuclear models "
		"derived from OpenTURNS examples.";
	double          inputVariables[kInputDistributionIndexMax];
	const char *    expectedInputHeaders[kInputDistributionIndexMax] = {
		"lineCurrent",
		"lineVoltage",
		"decayHeatShort",
		"decayHeatLong",
		"neutronFlux",
		"crossSection",
		"criticalHeatFlux",
		"localHeatFlux",
		"baseGeneration",
		"gridAvailability",
		"solarPhase",
		"floodFlowRate",
		"stricklerCoeff",
		"riverDownstream",
		"riverUpstream",
		"diffusionMultiplier",
		"crackingMultiplier"
	};
	double          outputVariables[kOutputDistributionIndexMax];
	const char *    outputVariableNames[kOutputDistributionIndexMax] = {
		"TransmissionPower",
		"TotalDecayHeat",
		"ReactionRate",
		"DNBR",
		"GridPower",
		"FloodOverflow",
		"FissionGasRelease"
	};
	const char *    outputVariableDescriptions[kOutputDistributionIndexMax] = {
		"P = V*I + R*I^2 [MW]",
		"Q = Qshort + Qlong [MW]",
		"R = Phi*Sigma [reaction rate]",
		"DNBR = qcrit/qlocal",
		"P = G*A + S*sin(phase) + k*G^2 [GW]",
		"S = Zv + H - Zb - Hd [flood overflow, m]",
		"f = fd*xdiff + fc*xcrack [FGR fraction]"
	};
	double *        monteCarloOutputSamples = NULL;
	clock_t         start                   = 0;
	clock_t         end                     = 0;
	double          cpuTimeUsedInSeconds    = 0.0;

	/*
	 *	Get command line arguments.
	 */
	if (getCommandLineArguments(argc, argv, &arguments) != kCommonConstantReturnTypeSuccess)
	{
		return EXIT_FAILURE;
	}

	/*
	 *	Read input distributions from CSV if input from file is enabled.
	 */
	if (arguments.common.isInputFromFileEnabled)
	{
		if (readInputDoubleDistributionsFromCSV(
				arguments.common.inputFilePath,
				expectedInputHeaders,
				inputVariables,
				kInputDistributionIndexMax
		))
		{
			fprintf(stderr, "Error: Could not read from input CSV file \"%s\".\n", arguments.common.inputFilePath);

			return EXIT_FAILURE;
		}
	}

	/*
	 *	Allocate for `monteCarloOutputSamples` if in Monte Carlo mode.
	 */
	if (arguments.common.isMonteCarloMode)
	{
		monteCarloOutputSamples = (double *) checkedMalloc(
			arguments.common.numberOfMonteCarloIterations * sizeof(double),
			__FILE__,
			__LINE__
		);
	}

	/*
	 *	Start timing if timing is enabled.
	 */
	if (arguments.common.isTimingEnabled)
	{
		start = clock();
	}

	/*
	 *	Dispatch to the mode-specific kernel.
	 */
	if (arguments.common.isMonteCarloMode)
	{
		calculateOutputMonteCarlo(&arguments, inputVariables, outputVariables, monteCarloOutputSamples);
	}
	else
	{
		calculateOutputUxHw(&arguments, inputVariables, outputVariables);
	}

	/*
	 *	Stop timing if timing is enabled.
	 */
	if (arguments.common.isTimingEnabled)
	{
		end = clock();
		cpuTimeUsedInSeconds = ((double) (end - start)) / CLOCKS_PER_SEC;
	}

	/*
	 *	Save Monte Carlo data to "data.out" if in Monte Carlo mode. Every
	 *	output is a distribution, so the whole sample array is saved.
	 */
	if (arguments.common.isMonteCarloMode)
	{
		saveMonteCarloDoubleDataToDataDotOutFile(
			monteCarloOutputSamples,
			(uint64_t) (cpuTimeUsedInSeconds * 1000000),
			arguments.common.numberOfMonteCarloIterations
		);
	}

	/*
	 *	Print json outputs if in JSON output mode.
	 */
	if (arguments.common.isOutputJSONMode)
	{
		printJSONFormattedOutput(
			&arguments.common,
			monteCarloOutputSamples,
			outputVariables,
			outputVariableNames,
			kOutputDistributionIndexMax,
			applicationDescription
		);
	}
	/*
	 *	Print human-consumable output if not in JSON output mode.
	 */
	else
	{
		printHumanConsumableOutput(
			&arguments.common,
			kOutputDistributionIndexMax,
			outputVariables,
			outputVariableNames,
			outputVariableDescriptions,
			monteCarloOutputSamples
		);
	}

	/*
	 *	Print timing if timing is enabled.
	 */
	if (arguments.common.isTimingEnabled)
	{
		printf("\nCPU time used: %" SignaloidParticleModifier "lf seconds\n", cpuTimeUsedInSeconds);
	}

	/*
	 *	Save outputs to file if write to file is enabled.
	 */
	if (arguments.common.isWriteToFileEnabled)
	{
		if (writeOutputDoubleDistributionsToCSV(
				arguments.common.outputFilePath,
				outputVariables,
				outputVariableNames,
				kOutputDistributionIndexMax
		))
		{
			fprintf(stderr, "Error: Could not write to output CSV file \"%s\".\n", arguments.common.outputFilePath);

			return EXIT_FAILURE;
		}
	}

	/*
	 *	Free allocations.
	 */
	if (arguments.common.isMonteCarloMode)
	{
		free(monteCarloOutputSamples);
	}

	return EXIT_SUCCESS;
}
