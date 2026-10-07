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
#include <uxhw.h>
#include "kernel.h"
#include "utilities.h"
#include "models-config.h"

/*
 *	The seven models are closed-form expressions, so the Monte Carlo and
 *	UxHw kernels evaluate exactly the same arithmetic. They differ only in
 *	how the inputs are populated and how the OpenTURNS truncations are
 *	applied. The Monte Carlo kernel draws a fresh sample of every input on
 *	each iteration and records the selected output, while the UxHw kernel
 *	sets each input to its full distribution once and lets the
 *	distributional arithmetic carry the uncertainty through in a single
 *	pass.
 */

/**
 *	@brief	Set the input variables via UxHw distribution calls. On the
 *		Signaloid Cloud Compute Engine each call yields a full
 *		distribution. Natively the compatibility layer draws one sample
 *		per call.
 *
 *	@param	inputVariables	: The input variables.
 */
static void
setInputVariables(double * inputVariables)
{
	/*
	 *	Model 0 -- Transmission-line power.
	 */
	inputVariables[kInputIndexLineCurrent] = UxHwDoubleGaussDist(
		kParamLineCurrentMean,
		kParamLineCurrentStdDev
	);
	inputVariables[kInputIndexLineVoltage] = UxHwDoubleUniformDist(
		kParamLineVoltageMin,
		kParamLineVoltageMax
	);

	/*
	 *	Model 1 -- Total decay heat.
	 */
	inputVariables[kInputIndexDecayHeatShort] = UxHwDoubleGaussDist(
		kParamDecayHeatShortMean,
		kParamDecayHeatShortStdDev
	);
	inputVariables[kInputIndexDecayHeatLong] = UxHwDoubleLognormalDist(
		kParamDecayHeatLongLogMean,
		kParamDecayHeatLongLogStdDev
	);

	/*
	 *	Model 2 -- Neutron reaction rate.
	 */
	inputVariables[kInputIndexNeutronFlux] = UxHwDoubleGaussDist(
		kParamNeutronFluxMean,
		kParamNeutronFluxStdDev
	);
	inputVariables[kInputIndexCrossSection] = UxHwDoubleLognormalDist(
		kParamCrossSectionLogMean,
		kParamCrossSectionLogStdDev
	);

	/*
	 *	Model 3 -- DNBR safety margin.
	 */
	inputVariables[kInputIndexCriticalHeatFlux] = UxHwDoubleGaussDist(
		kParamCriticalHeatFluxMean,
		kParamCriticalHeatFluxStdDev
	);
	inputVariables[kInputIndexLocalHeatFlux] = UxHwDoubleLognormalDist(
		kParamLocalHeatFluxLogMean,
		kParamLocalHeatFluxLogStdDev
	);

	/*
	 *	Model 4 -- Grid power.
	 */
	inputVariables[kInputIndexBaseGeneration] = UxHwDoubleGaussDist(
		kParamBaseGenerationMean,
		kParamBaseGenerationStdDev
	);
	inputVariables[kInputIndexGridAvailability] = UxHwDoubleUniformDist(
		kParamGridAvailabilityMin,
		kParamGridAvailabilityMax
	);
	inputVariables[kInputIndexSolarPhase] = UxHwDoubleExponentialDist(
		kParamSolarPhaseRate
	);

	/*
	 *	Model 5 -- Flood overflow (OpenTURNS flood use case).
	 */
	inputVariables[kInputIndexFloodFlowRate] = UxHwDoubleGumbel1Dist(
		kParamFloodFlowRateMode,
		kParamFloodFlowRateScale
	);
	inputVariables[kInputIndexStricklerCoeff] = UxHwDoubleGaussDist(
		kParamStricklerMean,
		kParamStricklerStdDev
	);
	inputVariables[kInputIndexRiverDownstream] = UxHwDoubleUniformDist(
		kParamRiverDownstreamMin,
		kParamRiverDownstreamMax
	);
	inputVariables[kInputIndexRiverUpstream] = UxHwDoubleUniformDist(
		kParamRiverUpstreamMin,
		kParamRiverUpstreamMax
	);

	/*
	 *	Model 6 -- Fission gas release (OpenTURNS fission-gas use case).
	 */
	inputVariables[kInputIndexDiffusionMultiplier] = UxHwDoubleLognormalDist(
		kParamDiffusionMultiplierLogMean,
		kParamDiffusionMultiplierLogStdDev
	);
	inputVariables[kInputIndexCrackingMultiplier] = UxHwDoubleLognormalDist(
		kParamCrackingMultiplierLogMean,
		kParamCrackingMultiplierLogStdDev
	);

	return;
}

/**
 *	@brief	Model 0 -- Transmission-line power: P = V*I + R*I^2 [MW].
 *
 *	@param	inputVariables	: The input variables.
 *	@return			: The computed output value.
 */
static double
calculateTransmissionPowerOutput(double * inputVariables)
{
	/*
	 *	Transmission-line power: delivered power (V*I) plus resistive loss (R*I^2).
	 *	The line current I appears in both terms, so a distributional processor must
	 *	preserve the self-correlation of I to get the resistive-loss mean right.
	 */
	double  current = inputVariables[kInputIndexLineCurrent];
	double  voltage = inputVariables[kInputIndexLineVoltage];

	return voltage * current + kConstLineResistanceOhm * pow(current, 2);
}

/**
 *	@brief	Model 1 -- Total decay heat: Q = Qshort + Qlong [MW].
 *
 *	@param	inputVariables	: The input variables.
 *	@return			: The computed output value.
 */
static double
calculateTotalDecayHeatOutput(double * inputVariables)
{
	/*
	 *	Total decay heat: sum of a short-lived (measured) and a long-lived
	 *	(more uncertain, LogNormal) fission-product contribution.
	 */
	double  decayHeatShort  = inputVariables[kInputIndexDecayHeatShort];
	double  decayHeatLong   = inputVariables[kInputIndexDecayHeatLong];

	return decayHeatShort + decayHeatLong;
}

/**
 *	@brief	Model 2 -- Neutron reaction rate: R = Phi * Sigma.
 *
 *	@param	inputVariables	: The input variables.
 *	@return			: The computed output value.
 */
static double
calculateReactionRateOutput(double * inputVariables)
{
	/*
	 *	Neutron reaction rate R = Phi * Sigma (flux times macroscopic cross-section).
	 */
	double  neutronFlux     = inputVariables[kInputIndexNeutronFlux];
	double  crossSection    = inputVariables[kInputIndexCrossSection];

	return neutronFlux * crossSection;
}

/**
 *	@brief	Model 3 -- DNBR safety margin: qcrit / qlocal.
 *
 *	@param	inputVariables	: The input variables.
 *	@return			: The computed output value.
 */
static double
calculateDNBROutput(double * inputVariables)
{
	/*
	 *	Departure-from-nucleate-boiling ratio: critical heat flux divided by the
	 *	local heat flux. The safety question is P(DNBR < kConstDNBRSafetyLimit).
	 */
	double  criticalHeatFlux    = inputVariables[kInputIndexCriticalHeatFlux];
	double  localHeatFlux       = inputVariables[kInputIndexLocalHeatFlux];

	return criticalHeatFlux / localHeatFlux;
}

/**
 *	@brief	Model 4 -- Grid power: P = G*A + S*sin(phase) + k*G^2 [GW].
 *
 *	@param	inputVariables	: The input variables.
 *	@return			: The computed output value.
 */
static double
calculateGridPowerOutput(double * inputVariables)
{
	/*
	 *	Illustrative grid-power balance: dispatchable generation (G*A), a periodic
	 *	renewable (diurnal) term S*sin(phase), and a quadratic demand-response term
	 *	k*G^2. Base generation G is reused in the linear and quadratic terms.
	 */
	double  baseGeneration  = inputVariables[kInputIndexBaseGeneration];
	double  availability    = inputVariables[kInputIndexGridAvailability];
	double  solarPhase      = inputVariables[kInputIndexSolarPhase];

	return baseGeneration * availability
	       + kConstSolarAmplitudeGW * sin(solarPhase)
	       + kConstDemandQuadraticCoeff * pow(baseGeneration, 2);
}

/**
 *	@brief	Model 5 -- Flood overflow height above the dyke crest [m].
 *		H = (Q / (Ks*B*sqrt((Zm-Zv)/L)))^0.6, S = Zv + H - Zb - Hd.
 *
 *	@param	inputVariables	: The input variables.
 *	@return			: The computed output value.
 */
static double
calculateFloodOverflowOutput(double * inputVariables)
{
	/*
	 *	OpenTURNS flood use case (simplified 1D Saint-Venant / Manning-Strickler).
	 *	Water depth H from the Manning-Strickler relation, then overflow height S
	 *	above the dyke crest. S > 0 means the river overtops the dyke.
	 *
	 *	The upstream/downstream altitude ranges guarantee (Zm - Zv) > 0, so the
	 *	slope is always positive. Q and Ks arrive from the calling kernel already
	 *	limited to their supports (Q >= 0, Ks >= kConstStricklerFloor), so the
	 *	division and the fractional power are well-defined.
	 */
	double  flowRate            = inputVariables[kInputIndexFloodFlowRate];
	double  stricklerCoeff      = inputVariables[kInputIndexStricklerCoeff];
	double  downstreamAltitude  = inputVariables[kInputIndexRiverDownstream];
	double  upstreamAltitude    = inputVariables[kInputIndexRiverUpstream];

	double  riverSlope  = (upstreamAltitude - downstreamAltitude) / kConstRiverLengthL;
	double  waterDepth  = pow(
		flowRate / (stricklerCoeff * kConstRiverWidthB * sqrt(riverSlope)),
		kConstManningExponent
	);

	return downstreamAltitude + waterDepth - kConstBankAltitudeZb - kConstDykeHeightHd;
}

/**
 *	@brief	Model 6 -- Fission gas release fraction: f = fd*xdiff + fc*xcrack.
 *
 *	@param	inputVariables	: The input variables.
 *	@return			: The computed output value.
 */
static double
calculateFissionGasReleaseOutput(double * inputVariables)
{
	/*
	 *	OpenTURNS fission-gas use case, linear reduced form. The released fraction
	 *	is the sum of a thermal-diffusion contribution and a micro-cracking
	 *	contribution, each scaled by an uncertain (LogNormal) multiplier. This
	 *	stands in for the OpenTURNS Gaussian-process surrogate over TRANSURANUS runs.
	 */
	double  diffusionMultiplier = inputVariables[kInputIndexDiffusionMultiplier];
	double  crackingMultiplier  = inputVariables[kInputIndexCrackingMultiplier];

	return kConstDiffusionNominalFraction * diffusionMultiplier
	       + kConstCrackingNominalFraction * crackingMultiplier;
}

/**
 *	@brief	Evaluate the selected model, or every model when `-S` selects all
 *		outputs, from the current values of the input variables.
 *
 *	@param	arguments	: Pointer to command line arguments struct.
 *	@param	inputVariables	: The input variables.
 *	@param	outputVariables	: The output variables.
 */
static void
calculateSelectedOutputs(
	CommandLineArguments *  arguments,
	double *                inputVariables,
	double *                outputVariables)
{
	bool calculateAllOutputs = (arguments->common.outputSelect == kOutputDistributionIndexMax);

	if (calculateAllOutputs || (arguments->common.outputSelect == kOutputIndexTransmissionPower))
	{
		outputVariables[kOutputIndexTransmissionPower] = calculateTransmissionPowerOutput(inputVariables);
	}

	if (calculateAllOutputs || (arguments->common.outputSelect == kOutputIndexTotalDecayHeat))
	{
		outputVariables[kOutputIndexTotalDecayHeat] = calculateTotalDecayHeatOutput(inputVariables);
	}

	if (calculateAllOutputs || (arguments->common.outputSelect == kOutputIndexReactionRate))
	{
		outputVariables[kOutputIndexReactionRate] = calculateReactionRateOutput(inputVariables);
	}

	if (calculateAllOutputs || (arguments->common.outputSelect == kOutputIndexDNBR))
	{
		outputVariables[kOutputIndexDNBR] = calculateDNBROutput(inputVariables);
	}

	if (calculateAllOutputs || (arguments->common.outputSelect == kOutputIndexGridPower))
	{
		outputVariables[kOutputIndexGridPower] = calculateGridPowerOutput(inputVariables);
	}

	if (calculateAllOutputs || (arguments->common.outputSelect == kOutputIndexFloodOverflow))
	{
		outputVariables[kOutputIndexFloodOverflow] = calculateFloodOverflowOutput(inputVariables);
	}

	if (calculateAllOutputs || (arguments->common.outputSelect == kOutputIndexFissionGasRelease))
	{
		outputVariables[kOutputIndexFissionGasRelease] = calculateFissionGasReleaseOutput(inputVariables);
	}

	return;
}

void
calculateOutputMonteCarlo(
	CommandLineArguments *  arguments,
	double *                inputVariables,
	double *                outputVariables,
	double *                monteCarloOutputSamples)
{
	/*
	 *	Monte Carlo mode rejects input from file, so every iteration draws
	 *	a fresh sample of each input before evaluating the selected model.
	 */
	for (size_t ii = 0; ii < arguments->common.numberOfMonteCarloIterations; ++ii)
	{
		setInputVariables(inputVariables);

		/*
		 *	OpenTURNS truncates Q and Ks from below. Per sample that is a clamp.
		 */
		inputVariables[kInputIndexFloodFlowRate]    = fmax(inputVariables[kInputIndexFloodFlowRate], kConstFloodFlowRateFloor);
		inputVariables[kInputIndexStricklerCoeff]   = fmax(inputVariables[kInputIndexStricklerCoeff], kConstStricklerFloor);

		calculateSelectedOutputs(arguments, inputVariables, outputVariables);
		monteCarloOutputSamples[ii] = outputVariables[arguments->common.outputSelect];
	}

	return;
}

void
calculateOutputUxHw(
	CommandLineArguments *  arguments,
	double *                inputVariables,
	double *                outputVariables)
{
	/*
	 *	If input from file is not enabled, set inputs via UxHw distribution
	 *	calls. One pass is enough. Each input carries its full distribution,
	 *	so every output does too.
	 */
	if (!arguments->common.isInputFromFileEnabled)
	{
		setInputVariables(inputVariables);
	}

	/*
	 *	OpenTURNS truncates Q and Ks from below. Limit the distribution support
	 *	accordingly, whether the inputs were sampled above or read from CSV.
	 */
	inputVariables[kInputIndexFloodFlowRate] = UxHwDoubleLimitDistributionSupport(
		inputVariables[kInputIndexFloodFlowRate],
		kConstFloodFlowRateFloor,
		INFINITY
	);
	inputVariables[kInputIndexStricklerCoeff] = UxHwDoubleLimitDistributionSupport(
		inputVariables[kInputIndexStricklerCoeff],
		kConstStricklerFloor,
		INFINITY
	);

	calculateSelectedOutputs(arguments, inputVariables, outputVariables);

	return;
}
