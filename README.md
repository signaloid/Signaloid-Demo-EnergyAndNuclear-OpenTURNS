[<img src="https://assets.signaloid.io/add-to-signaloid-cloud-logo-dark-latest.png#gh-dark-mode-only" alt="[Add to signaloid.io]" height="30">](https://signaloid.io/repositories?connect=https://github.com/signaloid/Signaloid-Demo-EnergyAndNuclear-OpenTURNS#gh-dark-mode-only)
[<img src="https://assets.signaloid.io/add-to-signaloid-cloud-logo-light-latest.png#gh-light-mode-only" alt="[Add to signaloid.io]" height="30">](https://signaloid.io/repositories?connect=https://github.com/signaloid/Signaloid-Demo-EnergyAndNuclear-OpenTURNS#gh-light-mode-only)


# OpenTURNS Energy and Nuclear Models with Signaloid UxHw® Support

This application propagates input uncertainty through seven closed-form
energy and nuclear models derived from OpenTURNS examples, with support for
Signaloid's distributional arithmetic (UxHw) as an alternative to traditional
Monte Carlo sampling. Models 0 to 4 give physical meaning to generic
OpenTURNS propagation expressions. Models 5 and 6 adapt the OpenTURNS
[flood](https://openturns.github.io/openturns/latest/usecases/use_case_flood_model.html)
and
[fission-gas](https://openturns.github.io/openturns/latest/usecases/use_case_fission_gas.html)
use cases. The repository has been structured to fit within the
[Signaloid UxHw benchmarking template](https://github.com/signaloid/Signaloid-Demo-Benchmarking-C-Template.git),
allowing direct comparison between traditional Monte Carlo sampling and
Signaloid's single-pass uncertainty tracking on the Signaloid Cloud
Compute Engine (SCCE).

The repository also holds `OpenTURNS/signaloid_openturns`, a Python package
that maps OpenTURNS distributions and expressions to generated C code and runs
it on SCCE. See the [Python package](#python-package) section.


## Cloning the repository

```bash
git clone --recursive https://github.com/signaloid/Signaloid-Demo-EnergyAndNuclear-OpenTURNS.git
```

To update all submodules:
```bash
git pull --recurse-submodules
git submodule update --remote --recursive
```

If you forgot to clone with `--recursive`:
```bash
git submodule update --init --recursive
```


## Quickstart

### Native build and run
```bash
make local-build              # Build the executable
make local-run                # Build and run all 7 outputs with timing

# Run individual outputs:
./demo-native-mc -M 10000 -S 0 -T    # Transmission-line power with 10000 MC samples and timing
./demo-native-mc -M 10000 -S 5       # Flood overflow with 10000 MC samples
./demo-native-mc -M 10000 -S 0 -j    # Transmission-line power with JSON output
```

## Overview

In traditional Monte Carlo mode (`-M N`), the application draws N independent
samples of every input, evaluates the selected model on each, and reports the
mean, variance, minimum and maximum of the N output samples. `-M 1` is a
one-sample Monte Carlo run, not the UxHw mode.

In UxHw single-shot mode (no `-M`, on the Signaloid Cloud Compute Engine), the
application sets each input to its full distribution once and evaluates the
model in a single pass. Signaloid's uncertainty-tracking hardware propagates
the distributions through the arithmetic, so each output is itself a
distribution. Natively the same mode runs against the compatibility layer and
yields one random sample.

## Models

The seven models are small algebraic expressions, so the point is the
uncertainty propagation rather than the model complexity. Full physical
definitions, units, and OpenTURNS provenance are in
[`docs/PHYSICAL_MODELS.md`](docs/PHYSICAL_MODELS.md).

| `-S` | Name | Domain | Formula | Inputs |
|------|------|--------|---------|--------|
| 0 | Transmission-line power | Energy | `P = V*I + R*I^2` [MW] | `I~Normal(1,0.1)` kA, `V~Uniform(220,240)` kV, `R=5` Ω |
| 1 | Total decay heat | Nuclear | `Q = Qshort + Qlong` [MW] | `Qshort~Normal(5,0.5)`, `Qlong~LogNormal(0,0.3)` |
| 2 | Neutron reaction rate | Nuclear | `R = Phi*Sigma` | `Phi~Normal(5,0.5)`, `Sigma~LogNormal(0,0.3)` |
| 3 | DNBR safety margin | Nuclear | `qcrit / qlocal` | `qcrit~Normal(5,0.5)`, `qlocal~LogNormal(0,0.3)`, limit 1.3 |
| 4 | Grid power balance | Energy | `G*A + S*sin(phase) + k*G^2` [GW] | `G~Normal(1,0.1)`, `A~Uniform(0.8,1)`, `phase~Exp(1)` |
| 5 | Flood overflow | Energy/hydro | `Zv + H - Zb - Hd` [m] | `Q~Gumbel(1013,558)` truncated at 0, `Ks~Normal(30,7.5)` floored at 5, `Zv~Uniform(49,51)`, `Zm~Uniform(54,56)` |
| 6 | Fission gas release | Nuclear | `fd*xdiff + fc*xcrack` | `xdiff~LogNormal(0,0.2)`, `xcrack~LogNormal(0,0.3)` |

Models 0 and 4 reuse an input (`I` and `G`) in more than one term. On a
distributional processor the two occurrences must remain the same random
variable, so these two models are the cross-check for correlation tracking.

## Output indices

| `-S` | Name | Description |
|------|------|-------------|
| 0 | TransmissionPower | `P = V*I + R*I^2` [MW] |
| 1 | TotalDecayHeat | `Q = Qshort + Qlong` [MW] |
| 2 | ReactionRate | `R = Phi*Sigma` |
| 3 | DNBR | `DNBR = qcrit/qlocal` |
| 4 | GridPower | `P = G*A + S*sin(phase) + k*G^2` [GW] |
| 5 | FloodOverflow | `S = Zv + H - Zb - Hd` [m] |
| 6 | FissionGasRelease | `f = fd*xdiff + fc*xcrack` |
| 7 | All outputs | Every model above (UxHw mode only) |

## Building and running

### Prerequisites (native build)
- GCC with C11 support
- GSL (GNU Scientific Library): `sudo apt install libgsl-dev`

### Native build and run
```bash
make local-build              # Build the executable
make local-run                # Build and run all 7 outputs with timing

# Run individual outputs:
./demo-native-mc -M 10000 -S 0 -T          # Transmission-line power with 10000 MC samples and timing
./demo-native-mc -S 0                       # UxHw kernel (natively one random sample)
./demo-native-mc -S 7                       # Every output, UxHw kernel
./demo-native-mc -M 10000 -S 0 -j          # JSON output
./demo-native-mc -S 7 -i inputs/input.csv   # Inputs from CSV samples
```

### Command-line options
```
Common options:
  -S <index>       Select output (0-6, see table above). 7, or no -S,
                   computes all outputs and is allowed in UxHw mode only
  -M <iterations>  Monte Carlo mode with N samples. N is required and has no
                   default. Without -M the UxHw kernel runs
  -i <path>        Read the input distributions from a CSV file
                   (UxHw mode only, see inputs/README.md)
  -o <path>        Write the outputs to a CSV file
  -T               Enable timing
  -b               Benchmarking output format
  -j               JSON output mode
  -v               Verbose (accepted but not supported by this demo)
  -h               Help
```

This demo adds no options of its own.

## Architecture

### Dual execution modes
The two modes are implemented as separate kernels, dispatched from `main.c` on
`isMonteCarloMode`. Both evaluate the same model functions. They differ only
in how the inputs are populated.

- **Monte Carlo mode** (`-M N`): `calculateOutputMonteCarlo()` in `kernel.c`. On each of N iterations it draws a fresh sample of every input through the UxHw calls in `setInputVariables()`, which the compatibility layer backs with GSL, evaluates the selected model, and records the output. `main.c` saves the samples to `data.out` and the common print routines report their statistics.
- **UxHw single-shot mode** (no `-M`): `calculateOutputUxHw()` in `kernel.c`. Sets each input to its full distribution once, or takes the distributions built from the CSV samples when `-i` is given, and evaluates the model in a single pass on SCCE.

### Module flow
```
Input distributions          (setInputVariables() in kernel.c, or -i CSV)
  -> Model evaluation        (one static function per model in kernel.c)
    -> Output                (common.c printing, data.out in Monte Carlo mode)
```

### Constants
Every distribution parameter (`kParam*`) and physical constant (`kConst*`),
and the input and output index enums, live in
[`src/models-config.h`](src/models-config.h).

## Python package

`OpenTURNS/signaloid_openturns` is a Python package that takes an OpenTURNS
model, maps its `ot.Distribution` inputs to UxHw calls and its muParser
expressions to C, generates a complete C source file, and runs it on SCCE
through the REST API. It is independent of the C demo in `src/`. See
[`OpenTURNS/signaloid_openturns/README.md`](OpenTURNS/signaloid_openturns/README.md).


## References

This project bridges OpenTURNS (uncertainty quantification) and Signaloid (hardware-accelerated distributional arithmetic). The models implemented here are derived from standard OpenTURNS uncertainty propagation examples, executed on Signaloid's SCCE which implements the Laplace microarchitecture for tracking probability distributions through computation.

### OpenTURNS

1. **Baudin, M., Dutfoy, A., Iooss, B., and Popelin, A.-L.** (2017). "OpenTURNS: An Industrial Software for Uncertainty Quantification in Simulation." In Ghanem, R., Higdon, D., Owhadi, H. (eds), *Handbook of Uncertainty Quantification*, pp. 1--38. Springer International Publishing. DOI: [10.1007/978-3-319-11259-6_64-1](https://doi.org/10.1007/978-3-319-11259-6_64-1)

2. **Andrianov, G., Burriel, S., Cambier, S., Dutfoy, A., Dutka-Malen, I., de Rocquigny, E., Sudret, B., Benjamin, P., Lebrun, R., Mangeant, F., and Pendola, M.** (2007). "Open TURNS, an Open Source Initiative to Treat Uncertainties, Risks'N Statistics in a Structured Industrial Approach." *Proceedings of the ESREL'2007 Safety and Reliability Conference*, Stavanger, Norway.

### Signaloid and uncertainty-tracking computation

3. **Tsoutsouras, V., Kaparounakis, O., Bilgin, B., Samarakoon, C., Meech, J., Heck, J., and Stanley-Marbell, P.** (2021). "The Laplace Microarchitecture for Tracking Data Uncertainty and Its Implementation in a RISC-V Processor." In *MICRO-54: 54th Annual IEEE/ACM International Symposium on Microarchitecture*, pp. 1254--1269. ACM. DOI: [10.1145/3466752.3480131](https://doi.org/10.1145/3466752.3480131). This is the foundational paper for Signaloid's technology, introducing the Laplace microarchitecture that tracks probability distributions through computation on a RISC-V processor and defining the ISA extensions that became the UxHw API.

4. **Bilgin, B. A., Elias, O. H., Selby, M., and Stanley-Marbell, P.** (2025). "Quantization of Probability Distributions via Divide-and-Conquer: Convergence and Error Propagation under Distributional Arithmetic Operations." arXiv preprint arXiv:2505.15283. DOI: [10.48550/arXiv.2505.15283](https://doi.org/10.48550/arXiv.2505.15283). Provides mathematical foundations for how Signaloid's processor discretizes continuous probability distributions, with error bounds for distributional arithmetic operations.

