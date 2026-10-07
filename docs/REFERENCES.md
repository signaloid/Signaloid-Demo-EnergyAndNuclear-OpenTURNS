# References

Research articles relevant to this repository.

This project bridges two technologies, OpenTURNS (uncertainty quantification)
and Signaloid (hardware-accelerated distributional arithmetic). The papers
below cover the theoretical and engineering foundations of both.

---

## OpenTURNS

### [1] Baudin et al. (2017), primary OpenTURNS reference

**Baudin, M., Dutfoy, A., Iooss, B., and Popelin, A.-L.** (2017).
"OpenTURNS: An Industrial Software for Uncertainty Quantification in
Simulation." In Ghanem, R., Higdon, D., Owhadi, H. (eds), *Handbook of
Uncertainty Quantification*, pp. 1--38. Springer International Publishing.

- **DOI:** [10.1007/978-3-319-11259-6_64-1](https://doi.org/10.1007/978-3-319-11259-6_64-1)
- **arXiv:** [1501.05242](https://arxiv.org/abs/1501.05242)

### [2] Andrianov et al. (2007), original OpenTURNS announcement

**Andrianov, G., Burriel, S., Cambier, S., Dutfoy, A., Dutka-Malen, I.,
de Rocquigny, E., Sudret, B., Benjamin, P., Lebrun, R., Mangeant, F., and
Pendola, M.** (2007). "Open TURNS, an Open Source Initiative to Treat
Uncertainties, Risks'N Statistics in a Structured Industrial Approach."
*Proceedings of the ESREL'2007 Safety and Reliability Conference*, Stavanger,
Norway.

---

## Signaloid and uncertainty-tracking computation

### [3] Tsoutsouras et al. (2021), Laplace microarchitecture (MICRO '21)

**Tsoutsouras, V., Kaparounakis, O., Bilgin, B., Samarakoon, C., Meech, J.,
Heck, J., and Stanley-Marbell, P.** (2021). "The Laplace Microarchitecture
for Tracking Data Uncertainty and Its Implementation in a RISC-V Processor."
In *MICRO-54: 54th Annual IEEE/ACM International Symposium on
Microarchitecture*, pp. 1254--1269. ACM.

- **DOI:** [10.1145/3466752.3480131](https://doi.org/10.1145/3466752.3480131)

This is the foundational paper for Signaloid's technology. It introduces the
Laplace microarchitecture that tracks probability distributions through
computation on a RISC-V processor, and defines the ISA extensions that became
the UxHw® API used by this package.

### [4] Bilgin et al. (2025), distribution quantization error bounds

**Bilgin, B. A., Elias, O. H., Selby, M., and Stanley-Marbell, P.** (2025).
"Quantization of Probability Distributions via Divide-and-Conquer:
Convergence and Error Propagation under Distributional Arithmetic Operations."
arXiv preprint arXiv:2505.15283.

- **DOI:** [10.48550/arXiv.2505.15283](https://doi.org/10.48550/arXiv.2505.15283)

Provides the mathematical foundations for how Signaloid's processor discretizes
continuous probability distributions, with error bounds for distributional
arithmetic operations.
