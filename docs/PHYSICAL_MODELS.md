# Physical Models

This demo propagates input uncertainty through seven closed-form models drawn
from the energy and nuclear domains. Each is a small algebraic expression, so
the point is not the model complexity but the *uncertainty propagation*. The
same computation runs either as local Monte Carlo (GSL) or as a single-pass
distributional computation on the Signaloid Cloud Compute Engine (SCCE), and
the two should agree.

Models 0 to 4 give physical meaning to the demo's original generic OpenTURNS
propagation expressions (`x0*x1 + x0^2`, `x0 + x1`, `x0 * x1`, `x0 / x1`,
`x0*x1 + sin(x2) + x0^2`). The mathematical structure is unchanged. Only the
variables, units, and input distributions carry physical meaning. Models 5
and 6 are adapted from the OpenTURNS
[flood](https://openturns.github.io/openturns/latest/usecases/use_case_flood_model.html)
and
[fission-gas](https://openturns.github.io/openturns/latest/usecases/use_case_fission_gas.html)
use cases.

Every input uses one of four UxHw® parametric distributions. These are
`Gauss`, `Uniform`, `LogNormal`, and `Exponential`.

| `-S` | Model | Domain | Formula | Reused variable |
|------|-------|--------|---------|-----------------|
| 0 | Transmission-line power | Energy | `P = V·I + R·I²` | `I` |
| 1 | Total decay heat | Nuclear | `Q = Q_short + Q_long` | none |
| 2 | Neutron reaction rate | Nuclear | `R = Φ·Σ` | none |
| 3 | DNBR safety margin | Nuclear | `DNBR = q_crit / q_local` | none |
| 4 | Grid power balance | Energy | `P = G·A + S·sin(φ) + k·G²` | `G` |
| 5 | Flood overflow | Energy/hydro | `S = Zv + H − Zb − Hd` | none |
| 6 | Fission gas release | Nuclear | `f = f_d·x_diff + f_c·x_crack` | none |
| 7 | All of the above | | | |

---

## Model 0, transmission-line power `P = V·I + R·I²`

Power on a transmission line is the delivered term `V·I` plus the resistive
(ohmic) loss `R·I²`. The line current `I` appears in **both** terms, so a
distributional processor must preserve the self-correlation of `I`. Treating
the two occurrences as independent would misestimate the loss term's mean.
The original expression was `x0*x1 + x0²`.

| Variable | Meaning | Distribution | Units |
|----------|---------|--------------|-------|
| `I` | line current | Normal(1.0, 0.1) | kA |
| `V` | line voltage | Uniform(220, 240) | kV |
| `R` | line resistance (constant) | 5.0 | Ω |

`kV · kA = MW` and `kA² · Ω = MW`, so `P` is in MW (≈ 235 MW nominal).

## Model 1, total decay heat `Q = Q_short + Q_long`

Decay heat from a shut-down core as the sum of a dominant short-lived
fission-product group, which is well measured and so Normal, and a
longer-lived group, which is more uncertain and so LogNormal. The original
expression was `x0 + x1`.

| Variable | Distribution | Units |
|----------|--------------|-------|
| `Q_short` | Normal(5.0, 0.5) | MW |
| `Q_long` | LogNormal(0, 0.3) | MW |

## Model 2, neutron reaction rate `R = Φ·Σ`

The canonical nuclear product. The reaction rate is the neutron flux times
the macroscopic cross-section. Cross-sections carry nuclear-data uncertainty
and are conventionally treated as **LogNormal**. The original expression was
`x0 * x1`.

| Variable | Meaning | Distribution | Units |
|----------|---------|--------------|-------|
| `Φ` | neutron flux | Normal(5.0, 0.5) | ×10¹³ n·cm⁻²·s⁻¹ |
| `Σ` | macroscopic cross-section | LogNormal(0, 0.3) | cm⁻¹ |

## Model 3, departure from nucleate boiling ratio `DNBR = q_crit / q_local`

The departure from nucleate boiling ratio (DNBR) is the critical heat flux
divided by the local heat flux. This is the most safety-relevant model. The
engineering question is `P(DNBR < 1.3)`, the probability of boiling crisis,
where 1.3 is the design limit `kConstDNBRSafetyLimit`. Propagating
uncertainty through a **quotient** produces a heavy-tailed output, which is
exactly where single-pass distributional arithmetic is worth comparing
against Monte Carlo. The original expression was `x0 / x1`.

| Variable | Meaning | Distribution | Units |
|----------|---------|--------------|-------|
| `q_crit` | critical heat flux | Normal(5.0, 0.5) | MW/m² |
| `q_local` | local heat flux | LogNormal(0, 0.3) | MW/m² |

## Model 4, grid power balance `P = G·A + S·sin(φ) + k·G²`

An **illustrative** net-power model with dispatchable generation `G·A`, a
periodic (diurnal) renewable term `S·sin(φ)`, and a quadratic
demand-response term `k·G²`. Base generation `G` is reused in the linear and
quadratic terms. This is the most synthetic of the seven. It exists to give
the original mixed nonlinear stress-test `x0*x1 + sin(x2) + x0²` a physical
form rather than to model a specific grid.

| Variable | Meaning | Distribution | Units |
|----------|---------|--------------|-------|
| `G` | base generation | Normal(1.0, 0.1) | GW |
| `A` | availability | Uniform(0.8, 1.0) | dimensionless |
| `φ` | diurnal phase | Exponential(1.0) | rad |
| `S` | solar amplitude (constant) | 0.3 | GW |
| `k` | demand quadratic coefficient (constant) | 0.1 | GW⁻¹ |

## Model 5, flood overflow `S = Zv + H − Zb − Hd`

Adapted from the OpenTURNS
[flood use case](https://openturns.github.io/openturns/latest/usecases/use_case_flood_model.html).
A simplified 1-D Saint-Venant / Manning-Strickler river model. The water
depth `H` comes from the Manning-Strickler relation, and the overflow height
`S` is measured relative to the dyke crest in meters. `S > 0` means the river
overtops the dyke.

```
α = (Zm − Zv) / L                        (river slope)
H = (Q / (Ks · B · √α))^0.6              (Manning-Strickler water depth)
S = Zv + H − Zb − Hd                     (overflow above dyke crest)
```

| Variable | Meaning | Distribution | Units |
|----------|---------|--------------|-------|
| `Q` | flow rate | Gumbel(mode 1013, scale 558), truncated to `Q ≥ 0` | m³/s |
| `Ks` | Strickler roughness coefficient | Normal(30, 7.5), truncated to `Ks ≥ 5` | m^(1/3)/s |
| `Zv` | downstream bed altitude | Uniform(49, 51) | m |
| `Zm` | upstream bed altitude | Uniform(54, 56) | m |
| `B` | river width (constant) | 300 | m |
| `L` | river length (constant) | 5000 | m |
| `Zb` | bank altitude (constant) | 55.5 | m |
| `Hd` | dyke height (constant) | 3.0 | m |

`Q` and `Ks` are the OpenTURNS distributions. OpenTURNS truncates both from
below, and the two kernels apply that truncation differently.
- The UxHw kernel calls `UxHwDoubleLimitDistributionSupport()`, which removes
  the mass below the floor and rescales the rest to unit mass, exactly as
  OpenTURNS does.
- The Monte Carlo kernel clamps each sample with `fmax()`. The 0.2 percent of
  Gumbel mass below zero therefore sits at `Q = 0`, which maps to
  `S = Zv − Zb − Hd ≈ −8.5 m` inside the existing left tail, instead of being
  redrawn. The area between the two resulting cumulative distribution
  functions of `S` is about 0.006 m.
- `Ks` is floored at 5 m^(1/3)/s rather than at 0 (`P(Ks < 5) ≈ 4×10⁻⁴`),
  which keeps a near-zero denominator from producing a spurious water-depth
  spike.
- The upstream and downstream altitude ranges guarantee `Zm − Zv ≥ 3`, so the
  slope is always positive and `√α` is always well-defined.

With these parameters the dyke normally holds (mean `S ≈ −6 m`) and overtops
only in the tail of the flow distribution. This is the safety-margin behavior
the OpenTURNS case is designed to illustrate.

## Model 6, fission gas release `f = f_d·x_diff + f_c·x_crack`

Adapted from the OpenTURNS
[fission-gas use case](https://openturns.github.io/openturns/latest/usecases/use_case_fission_gas.html).
Fraction of fission gas released from nuclear fuel, expressed as a fraction
of what the fission reaction creates. The physical release has two
mechanisms, thermal diffusion of single atoms and release from
micro-cracking. Each is scaled here by an uncertain multiplier.

```
f = f_d · x_diff + f_c · x_crack
```

| Variable | Meaning | Distribution | Units |
|----------|---------|--------------|-------|
| `x_diff` | diffusion multiplier | LogNormal(0, 0.2) | dimensionless |
| `x_crack` | micro-cracking multiplier | LogNormal(0, 0.3) | dimensionless |
| `f_d` | nominal diffusion fraction (constant) | 0.15 | dimensionless |
| `f_c` | nominal cracking fraction (constant) | 0.05 | dimensionless |

Nominal release ≈ 20 %, a plausible value for high-burnup fuel under transient
conditions.

This model deviates from the OpenTURNS use case in one way. The reference
fits **Gaussian-process surrogate models** to TRANSURANUS fuel-performance
simulations across a set of experimental conditions, with a heteroscedastic
measurement model `σ_y = √((y/20)² + 10⁻⁴)`. A GP surrogate is not
expressible as a closed-form kernel here, so this model is a **linear reduced
form** that captures the two-mechanism additive structure. The
measurement-noise model is documented here for reference but is not applied
to the released fraction itself.

---

## Reused-variable models and self-correlation

Models 0 and 4 reuse an input (`I` and `G` respectively) in more than one term.
On a distributional processor the two occurrences must remain the *same* random
variable. If they are split into independent copies, the mean of the squared
and product terms is wrong. These two models are therefore the most valuable
cross-checks between the SCCE result and Monte Carlo. For the same reason,
they are the ones to watch when validating correlation handling.
