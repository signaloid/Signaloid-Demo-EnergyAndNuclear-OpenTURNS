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

#include "utilities.h"
#include "models-config.h"

/**
 *	@brief	Monte Carlo calculation kernel. On each of
 *		`arguments->common.numberOfMonteCarloIterations` iterations it
 *		draws a fresh sample of every input, clamps the inputs that
 *		OpenTURNS truncates, evaluates the selected model, and records
 *		the selected output in `monteCarloOutputSamples`.
 *
 *	@param	arguments		: Pointer to command line arguments struct.
 *	@param	inputVariables		: The input variables.
 *	@param	outputVariables		: The output variables.
 *	@param	monteCarloOutputSamples	: One sample of the selected output per iteration.
 */
void
calculateOutputMonteCarlo(
	CommandLineArguments *  arguments,
	double *                inputVariables,
	double *                outputVariables,
	double *                monteCarloOutputSamples);

/**
 *	@brief	UxHw calculation kernel. Sets every input to its full
 *		distribution, unless the inputs were read from a CSV file, limits
 *		the support of the inputs that OpenTURNS truncates, and evaluates
 *		the selected model(s) once so that each output carries its full
 *		distribution.
 *
 *	@param	arguments	: Pointer to command line arguments struct.
 *	@param	inputVariables	: The input variables.
 *	@param	outputVariables	: The output variables.
 */
void
calculateOutputUxHw(
	CommandLineArguments *  arguments,
	double *                inputVariables,
	double *                outputVariables);
