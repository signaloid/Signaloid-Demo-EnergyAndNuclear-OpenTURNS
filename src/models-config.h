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

/*
 *	Input random variables, grouped by the physical model that consumes them.
 *	Every model owns its inputs (no sharing) so each reframed problem is a
 *	self-contained energy/nuclear uncertainty-propagation example. See
 *	docs/PHYSICAL_MODELS.md for the physical meaning of each variable.
 */
typedef enum
{
	/* Model 0: Transmission-line power  P = V*I + R*I^2 */
	kInputIndexLineCurrent = 0,         /* I  ~ Normal(1.0, 0.1)   [kA] */
	kInputIndexLineVoltage,             /* V  ~ Uniform(220, 240)  [kV] */

	/* Model 1: Total decay heat  Q = Qshort + Qlong */
	kInputIndexDecayHeatShort,          /* ~ Normal(5.0, 0.5)      [MW] */
	kInputIndexDecayHeatLong,           /* ~ LogNormal(0, 0.3)     [MW] */

	/* Model 2: Neutron reaction rate  R = Phi * Sigma */
	kInputIndexNeutronFlux,             /* Phi   ~ Normal(5.0, 0.5)  [1e13 n/cm^2/s] */
	kInputIndexCrossSection,            /* Sigma ~ LogNormal(0, 0.3) [1/cm] */

	/* Model 3: DNBR safety margin  qcrit / qlocal */
	kInputIndexCriticalHeatFlux,        /* qcrit  ~ Normal(5.0, 0.5)  [MW/m^2] */
	kInputIndexLocalHeatFlux,           /* qlocal ~ LogNormal(0, 0.3) [MW/m^2] */

	/* Model 4: Grid power  P = G*A + S*sin(phase) + k*G^2 */
	kInputIndexBaseGeneration,          /* G     ~ Normal(1.0, 0.1)   [GW] */
	kInputIndexGridAvailability,        /* A     ~ Uniform(0.8, 1.0)  [-] */
	kInputIndexSolarPhase,              /* phase ~ Exponential(1.0)   [rad] */

	/* Model 5: Flood overflow (Saint-Venant / Manning-Strickler) */
	kInputIndexFloodFlowRate,           /* Q  ~ Gumbel(1013, 558)   [m^3/s] (truncated at 0) */
	kInputIndexStricklerCoeff,          /* Ks ~ Normal(30, 7.5)     [m^(1/3)/s] */
	kInputIndexRiverDownstream,         /* Zv ~ Uniform(49, 51)     [m] */
	kInputIndexRiverUpstream,           /* Zm ~ Uniform(54, 56)     [m] */

	/* Model 6: Fission gas release  f = fd*xdiff + fc*xcrack */
	kInputIndexDiffusionMultiplier,         /* xdiff  ~ LogNormal(0, 0.2) [-] */
	kInputIndexCrackingMultiplier,          /* xcrack ~ LogNormal(0, 0.3) [-] */

	kInputDistributionIndexMax,
} InputDistributionIndex;

typedef enum
{
	kOutputIndexTransmissionPower = 0,  /* Energy  */
	kOutputIndexTotalDecayHeat,         /* Nuclear */
	kOutputIndexReactionRate,           /* Nuclear */
	kOutputIndexDNBR,                   /* Nuclear */
	kOutputIndexGridPower,              /* Energy  */
	kOutputIndexFloodOverflow,          /* Energy/hydro (OpenTURNS flood use case) */
	kOutputIndexFissionGasRelease,      /* Nuclear (OpenTURNS fission-gas use case) */
	kOutputDistributionIndexMax,
} OutputDistributionIndex;

/*
 *	Model 0 -- Transmission-line power  P = V*I + R*I^2  [MW].
 *	I (current) is reused in both terms, exercising self-correlation handling.
 */
#define kParamLineCurrentMean   (1.0)
#define kParamLineCurrentStdDev (0.1)
#define kParamLineVoltageMin    (220.0)
#define kParamLineVoltageMax    (240.0)
#define kConstLineResistanceOhm (5.0)

/*
 *	Model 1 -- Total decay heat  Q = Qshort + Qlong  [MW].
 */
#define kParamDecayHeatShortMean        (5.0)
#define kParamDecayHeatShortStdDev      (0.5)
#define kParamDecayHeatLongLogMean      (0.0)
#define kParamDecayHeatLongLogStdDev    (0.3)

/*
 *	Model 2 -- Neutron reaction rate  R = Phi * Sigma.
 *	Cross-section Sigma is LogNormal, the standard choice for nuclear-data uncertainty.
 */
#define kParamNeutronFluxMean       (5.0)
#define kParamNeutronFluxStdDev     (0.5)
#define kParamCrossSectionLogMean   (0.0)
#define kParamCrossSectionLogStdDev (0.3)

/*
 *	Model 3 -- DNBR safety margin  qcrit / qlocal.
 *	Departure-from-nucleate-boiling limit is typically 1.3; here the mean margin is ~5.
 */
#define kParamCriticalHeatFluxMean      (5.0)
#define kParamCriticalHeatFluxStdDev    (0.5)
#define kParamLocalHeatFluxLogMean      (0.0)
#define kParamLocalHeatFluxLogStdDev    (0.3)
#define kConstDNBRSafetyLimit           (1.3)

/*
 *	Model 4 -- Grid power  P = G*A + S*sin(phase) + k*G^2  [GW].
 *	G (base generation) is reused in the linear and quadratic terms.
 */
#define kParamBaseGenerationMean    (1.0)
#define kParamBaseGenerationStdDev  (0.1)
#define kParamGridAvailabilityMin   (0.8)
#define kParamGridAvailabilityMax   (1.0)
#define kParamSolarPhaseRate        (1.0)
#define kConstSolarAmplitudeGW      (0.3)
#define kConstDemandQuadraticCoeff  (0.1)

/*
 *	Model 5 -- Flood overflow (OpenTURNS flood use case).
 *	H  = (Q / (Ks * B * sqrt((Zm - Zv) / L)))^0.6      (Manning-Strickler)
 *	S  = Zv + H - Zb - Hd                              (overflow above dyke crest)
 *	Q is the OpenTURNS Gumbel flow rate (mode 1013, scale 558), truncated to
 *	Q >= 0 as in OpenTURNS. The kernels apply the truncation: the UxHw kernel
 *	limits the distribution support and the Monte Carlo kernel clamps samples.
 */
#define kParamFloodFlowRateMode     (1013.0)
#define kParamFloodFlowRateScale    (558.0)
#define kConstFloodFlowRateFloor    (0.0)
#define kParamStricklerMean         (30.0)
#define kParamStricklerStdDev       (7.5)
#define kParamRiverDownstreamMin    (49.0)
#define kParamRiverDownstreamMax    (51.0)
#define kParamRiverUpstreamMin      (54.0)
#define kParamRiverUpstreamMax      (56.0)
#define kConstRiverWidthB           (300.0)
#define kConstRiverLengthL          (5000.0)
#define kConstBankAltitudeZb        (55.5)
#define kConstDykeHeightHd          (3.0)
#define kConstManningExponent       (0.6)
/*
 *	Physical minimum for the Strickler coefficient. OpenTURNS truncates Ks to
 *	positive values. The kernels limit Ks to this floor instead (the UxHw kernel
 *	by limiting the distribution support, the Monte Carlo kernel per sample),
 *	which moves negligible probability mass (P(Ks < 5) ~ 4e-4) while preventing
 *	a near-zero denominator from producing a spurious water-depth spike.
 */
#define kConstStricklerFloor (5.0)

/*
 *	Model 6 -- Fission gas release fraction (OpenTURNS fission-gas use case).
 *	Linear reduced form  f = fd*xdiff + fc*xcrack  standing in for the
 *	OpenTURNS Gaussian-process surrogate fitted to TRANSURANUS simulations.
 */
#define kParamDiffusionMultiplierLogMean    (0.0)
#define kParamDiffusionMultiplierLogStdDev  (0.2)
#define kParamCrackingMultiplierLogMean     (0.0)
#define kParamCrackingMultiplierLogStdDev   (0.3)
#define kConstDiffusionNominalFraction      (0.15)
#define kConstCrackingNominalFraction       (0.05)
