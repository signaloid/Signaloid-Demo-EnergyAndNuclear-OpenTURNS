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

#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include "models-config.h"

/**
 *	@brief	Kernel inputs. The command-line parser sets the defaults and
 *		fills it in, and `main()` reads the CSV inputs into it, so the
 *		kernels do not depend on the command-line parsing.
 */
typedef struct
{
	/*
	 *	True if `inputVariables` holds the inputs read from a CSV file,
	 *	in which case the UxHw kernel does not draw them.
	 */
	bool isInputFromFileEnabled;

	/*
	 *	The inputs read from the CSV file, indexed by `InputDistributionIndex`.
	 *	Unused unless `isInputFromFileEnabled`.
	 */
	double inputVariables[kInputDistributionIndexMax];
} KernelParameters;

/**
 *	@brief	Monte Carlo calculation kernel. On each of
 *		`numberOfMonteCarloIterations` iterations it
 *		draws a fresh sample of every input, clamps the inputs that
 *		OpenTURNS truncates, evaluates the selected model, and records
 *		the selected output in `monteCarloOutputSamples`.
 *
 *	@param	parameters			: The kernel inputs.
 *	@param	outputSelect			: Index of the output to calculate, or
 *						  `kOutputDistributionIndexMax` for all outputs.
 *	@param	numberOfMonteCarloIterations	: Number of Monte Carlo iterations to run.
 *	@param	outputVariables			: The output variables.
 *	@param	monteCarloOutputSamples		: One sample of the selected output per iteration.
 */
void
calculateOutputMonteCarlo(
	const KernelParameters *    parameters,
	size_t                      outputSelect,
	size_t                      numberOfMonteCarloIterations,
	double *                    outputVariables,
	double *                    monteCarloOutputSamples);

/**
 *	@brief	UxHw calculation kernel. Sets every input to its full
 *		distribution, unless the inputs were read from a CSV file, limits
 *		the support of the inputs that OpenTURNS truncates, and evaluates
 *		the selected model(s) once so that each output carries its full
 *		distribution.
 *
 *	@param	parameters	: The kernel inputs, including the inputs read
 *				  from a CSV file if `isInputFromFileEnabled`.
 *	@param	outputSelect	: Index of the output to calculate, or
 *				  `kOutputDistributionIndexMax` for all outputs.
 *	@param	outputVariables	: The output variables.
 */
void
calculateOutputUxHw(
	const KernelParameters *    parameters,
	size_t                      outputSelect,
	double *                    outputVariables);
