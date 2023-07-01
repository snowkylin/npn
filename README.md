# NPN: Compute canonical representatives for boolean functions

This package is a boolean matcher. It simply computes the canonical representative, which is unique for each NPN equivalence class, for a given boolean function represented by truth table.

The backend of this package is implemented by C++ for efficiency.

Currently this package only support boolean functions with number of inputs <= 6, as the canonical representative is returned as an unsigned long long with 2^6 bits. Please file a feature request if you need more.

## Install

```bash
pip install npn
```

## Usage

```python
import npn

# the truth table of f(x_1, x_2, x_3) as [f(0, 0, 0), f(1, 0, 0), f(0, 1, 0), f(1, 1, 0), ...]
tt = [True, True, True, False, True, True, True, True]
npn.npn_canonical_representative(tt)    # 254 (11111101)
```

## Build

Compile the shared library `npn.dll` and `libnpn.so` via CMake on Windows and Linux environment, place them in the `\npn` folder, then

```bash
python setup.py sdist
```

## Test

```bash
pip install pytest  # if you haven't install it
python -m pytest
```

## References
- Chai, D., and A. Kuehlmann. “Building a Better Boolean Matcher and Symmetry Detector.” In Proceedings of the Design Automation & Test in Europe Conference, 1:1–6, 2006. https://doi.org/10.1109/DATE.2006.243959.
- Debnath, D., and T. Sasao. “Efficient Computation of Canonical Form for Boolean Matching in Large Libraries.” In ASP-DAC 2004: Asia and South Pacific Design Automation Conference 2004 (IEEE Cat. No.04EX753), 591–96, 2004. https://doi.org/10.1109/ASPDAC.2004.1337660.
- Hinsberger, U., and R. Kolla. “Boolean Matching for Large Libraries.” In Proceedings 1998 Design and Automation Conference. 35th DAC. (Cat. No.98CH36175), 206–11, 1998. https://doi.org/10.1145/277044.277100.
