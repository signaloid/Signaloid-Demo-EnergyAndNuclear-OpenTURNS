# Source Code

## `main.c`
Entry point. Parses the command line, reads the input distributions from the
input CSV when `-i` is given, dispatches to the Monte Carlo or UxHw® kernel on
`isMonteCarloMode`, and prints or saves the selected outputs.

## `kernel.c` and `kernel.h`
The two calculation kernels, `calculateOutputMonteCarlo()` and
`calculateOutputUxHw()`, together with the input sampling and the seven model
functions they share. Both kernels evaluate the same closed-form expressions.
They differ only in how the inputs are populated. The Monte Carlo kernel draws
a fresh sample of every input on each iteration and records the selected
output. The UxHw kernel sets each input to its full distribution once.

## `models-config.h`
Central header for the models. It holds the input and output index enums and
the distribution parameters and physical constants of every model. See
[`docs/PHYSICAL_MODELS.md`](../docs/PHYSICAL_MODELS.md) for their meaning.

## `utilities.c` and `utilities.h`
Demo-specific command-line handling, layered on the common arguments handled
by `common.c`. This demo adds no options of its own.

## `common.c` and `common.h`
Utility methods for parsing, setting, and reporting the usage of command-line
arguments common to all of our C/C++ demo applications, as well as other
methods commonly used across them, such as CSV and JSON I/O. These source
files are symlinks to the original files contained in the repository
[Signaloid-Demo-CommonUtilityRoutines](https://github.com/signaloid/Signaloid-Demo-CommonUtilityRoutines),
which is included as a submodule in `submodules/common`.

## `uxhw.c` and `uxhw.h`
Methods that implement the probabilistic versions of the methods in the UxHw
API (e.g., `UxHwDoubleGaussDist`) using the GNU Scientific Library (GSL)
random number generators. This allows building our C/C++ demo applications
natively (i.e., on conventional architectures) and running native Monte Carlo
evaluations without modifying the source code. These source files are symlinks
to the original files contained in the repository
[Signaloid-Demo-UxHwCompatibilityForNativeExecution](https://github.com/signaloid/Signaloid-Demo-UxHwCompatibilityForNativeExecution),
which is included as a submodule in `submodules/compat`.

## `config.mk`
The source list shared by the Signaloid cloud build and the native build. New
source files must be added here.

## Building natively
Use the top-level `Makefile`, which reads `config.mk`, adds `uxhw.c` and links
GSL:

```bash
make local-build
```
