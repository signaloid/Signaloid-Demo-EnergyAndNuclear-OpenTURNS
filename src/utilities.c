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

#include <math.h>
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <assert.h>
#include "utilities.h"
#include "common.h"

void
printUsage(void)
{
	fprintf(stderr, "OpenTURNS Energy & Nuclear Uncertainty Propagation Models on Signaloid SCCE\n");
	fprintf(stderr, "\n");
	printCommonUsage();
	fprintf(stderr, "\n");
	fprintf(stderr, "Output selection (-S):\n");
	fprintf(stderr, "	-S 0  Transmission Power:  P = V*I + R*I^2        (I~Normal(1,0.1)kA, V~Uniform(220,240)kV)\n");
	fprintf(stderr, "	-S 1  Total Decay Heat:    Q = Qshort + Qlong     (Qshort~Normal(5,0.5)MW, Qlong~LogNormal(0,0.3)MW)\n");
	fprintf(stderr, "	-S 2  Reaction Rate:       R = Phi*Sigma          (Phi~Normal(5,0.5), Sigma~LogNormal(0,0.3))\n");
	fprintf(stderr, "	-S 3  DNBR Safety Margin:  qcrit/qlocal           (qcrit~Normal(5,0.5), qlocal~LogNormal(0,0.3))\n");
	fprintf(stderr, "	-S 4  Grid Power:          G*A+S*sin(phase)+k*G^2 (G~Normal(1,0.1)GW, A~Uniform(0.8,1), phase~Exp(1))\n");
	fprintf(stderr, "	-S 5  Flood Overflow:      Zv+H-Zb-Hd             (Q~Gumbel(1013,558), Ks~Normal(30,7.5), Zv,Zm~Uniform)\n");
	fprintf(stderr, "	-S 6  Fission Gas Release: fd*xdiff + fc*xcrack   (xdiff~LogNormal(0,0.2), xcrack~LogNormal(0,0.3))\n");
	fprintf(stderr, "	-S 7  All outputs\n");
	fprintf(stderr, "\n");

	return;
}

CommonConstantReturnType
setDefaultCommandLineArguments(CommandLineArguments * arguments)
{
	if (arguments == NULL)
	{
		fprintf(stderr, "Error: The provided pointer to arguments is NULL.\n");

		return kCommonConstantReturnTypeError;
	}

	/*
	 *	Older GCC versions have a bug which gives a spurious warning for the C universal zero
	 *	initializer `{0}`. Any workaround makes the code less portable or prevents the common code
	 *	from adding new fields to the `CommonCommandLineArguments` struct. Therefore, we surpress
	 *	this warning.
	 *
	 *	See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53119.
	 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
	*arguments = (CommandLineArguments) {
		.common             = (CommonCommandLineArguments) { 0 },
		.kernelParameters   = (KernelParameters) {
			.isInputFromFileEnabled = false,
		},
	};
#pragma GCC diagnostic pop

	return kCommonConstantReturnTypeSuccess;
}

#ifdef NO_OS_AVAILABLE
CommonConstantReturnType
setNoOSCommandLineArguments(CommandLineArguments * arguments)
{
	if (arguments == NULL)
	{
		fputs("Error: The provided pointer to arguments is NULL.\n", stderr);

		return kCommonConstantReturnTypeError;
	}

	/*
	 *	Start from the defaults so that every demo-specific field is
	 *	initialized, then override the ones the no-OS build fixes. The
	 *	`common` sub-struct is zeroed by this call, so it is set explicitly
	 *	below.
	 */
	if (setDefaultCommandLineArguments(arguments) != kCommonConstantReturnTypeSuccess)
	{
		return kCommonConstantReturnTypeError;
	}

	arguments->common.numberOfMonteCarloIterations  = 1;
	arguments->common.outputSelect                  = kOutputDistributionIndexMax;
	arguments->common.isTimingEnabled               = false;
	arguments->common.isMonteCarloMode              = false;
	arguments->common.isOutputJSONMode              = false;
	arguments->common.isWriteToFileEnabled          = false;

	return kCommonConstantReturnTypeSuccess;
}
#endif

CommonConstantReturnType
getCommandLineArguments(int argc, char *  argv[], CommandLineArguments *  arguments)
{
#ifdef NO_OS_AVAILABLE
	/*
	 *	The no-OS build has no command line to parse, so ignore `argc` and
	 *	`argv` and use the hard-coded configuration instead.
	 */
	(void) argc;
	(void) argv;

	puts("Using hard coded command line arguments");

	return setNoOSCommandLineArguments(arguments);

#else

	if (arguments == NULL)
	{
		fprintf(stderr, "Error: The provided pointer to arguments is NULL.\n");

		return kCommonConstantReturnTypeError;
	}

	if (setDefaultCommandLineArguments(arguments) != kCommonConstantReturnTypeSuccess)
	{
		return kCommonConstantReturnTypeError;
	}

	DemoOption options[] = {
		{ 0 },
	};

	if (parseArgs(argc, argv, &arguments->common, options) != kCommonConstantReturnTypeSuccess)
	{
		fprintf(stderr, "Error: Parsing command line arguments failed.\n");
		printUsage();

		return kCommonConstantReturnTypeError;
	}

	if (arguments->common.isHelpEnabled)
	{
		printUsage();

		exit(EXIT_SUCCESS);
	}

	/*
	 *	If no output is selected, set `outputSelect` to `kOutputDistributionIndexMax`.
	 *	This triggers the demo to compute all outputs.
	 */
	if (!arguments->common.isOutputSelected)
	{
		arguments->common.outputSelect = kOutputDistributionIndexMax;
	}

	/*
	 *	When `outputSelect` is set to `kOutputDistributionIndexMax`, we cannot be
	 *	in Monte Carlo mode.
	 */
	if (arguments->common.outputSelect == kOutputDistributionIndexMax)
	{
		if (arguments->common.isMonteCarloMode)
		{
			fprintf(stderr, "Error: Please select a single output when in Monte Carlo mode.\n");

			return kCommonConstantReturnTypeError;
		}
	}
	/*
	 *	Selected output can never be greater than `kOutputDistributionIndexMax`.
	 */
	else if (arguments->common.outputSelect > kOutputDistributionIndexMax)
	{
		fprintf(stderr, "Error: Wrong output selection.\n");

		return kCommonConstantReturnTypeError;
	}

	/*
	 *	Monte Carlo mode does not support input from file.
	 */
	if ((arguments->common.isMonteCarloMode) && (arguments->common.isInputFromFileEnabled))
	{
		fprintf(stderr, "Error: Monte Carlo mode does not support input from file.\n");

		return kCommonConstantReturnTypeError;
	}

	arguments->kernelParameters.isInputFromFileEnabled = arguments->common.isInputFromFileEnabled;

	if (arguments->common.isVerbose)
	{
		fprintf(stderr, "Warning: Verbose mode not supported. Continuing in non-verbose mode.\n");
	}

	return kCommonConstantReturnTypeSuccess;

#endif
}
