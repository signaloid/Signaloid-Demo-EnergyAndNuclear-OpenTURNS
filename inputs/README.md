# Input Files

## `input.csv`
Example input for the `-i` option. The CSV has one column per model input,
17 in all, headed with the names in `expectedInputHeaders` in `src/main.c`.
The order of the columns is not important, but every header must match. Each
row is one sample, and the samples in a column become a distribution via
`UxHwDoubleDistFromSamples()`.

The shipped file holds 10,000 samples per column, drawn from the same
distributions that `setInputVariables()` in `src/kernel.c` uses. See
[`docs/PHYSICAL_MODELS.md`](../docs/PHYSICAL_MODELS.md) for the distribution
of each input.

`-i` is accepted only in UxHw® single-shot mode (no `-M`). Monte Carlo mode
rejects it. On the Signaloid Cloud Compute Engine each input is then the
distribution built from its samples, and every output is a distribution
derived from them. Natively, the compatibility layer picks one sample at
random.

```bash
./demo-native-mc -S 7 -i inputs/input.csv
```
