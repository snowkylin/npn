# NPN: Compute canonical representatives for boolean functions

This package is a boolean matcher. It simply computes the canonical representative, which is unique for each NPN equivalence class, for a given boolean function represented by truth table.

The backend of this package is implemented by C++ for efficiency.

Currently, this package supports boolean functions with number of inputs <= 8.

## Install

```bash
pip install npn
```

## Usage

```python
import npn

# the truth table of f(x_2, x_1, x_0) as [f(0, 0, 0), f(0, 0, 1), f(0, 1, 0), f(0, 1, 1), ...]
tt = [True, True, True, False, True, True, True, True]
c = npn.npn_canonical_representative(tt)    # [True, True, True, True, True, True, True, False]
c_int = npn.tt_to_int(c)                    # 254 (11111110)
# return the NPN transformation information (phase, perm, output_inv)
# here phase = [False, False, True], perm = [0, 1, 2] and output_inv = False
# which means (x_0, x_1, x_2) should be mapped to (x_0, x_1, x_2), then the third variable (x_2) should be inverted
# and the final result should not be inverted
# note that permutation should be applied first before the inverse (see Section 2.1 of [1])
c, phase, perm, output_inv = npn.npn_canonical_representative(tt, return_details=True)
```

## Build

Compile the shared library `libnpn.dll` and `libnpn.so` via CMake on Windows and Linux environment, place them in the `\npn` folder, then

```bash
python setup.py sdist
```

To upload to PyPI, run

```bash
twine upload .\dist\npn-X.X.tar.gz
```

## Test

```bash
pip install pytest  # if you haven't install it
python -m pytest
```

## References
1. Chai, D., and A. Kuehlmann. “Building a Better Boolean Matcher and Symmetry Detector.” In Proceedings of the Design Automation & Test in Europe Conference, 1:1–6, 2006. https://doi.org/10.1109/DATE.2006.243959.
2. Debnath, D., and T. Sasao. “Efficient Computation of Canonical Form for Boolean Matching in Large Libraries.” In ASP-DAC 2004: Asia and South Pacific Design Automation Conference 2004 (IEEE Cat. No.04EX753), 591–96, 2004. https://doi.org/10.1109/ASPDAC.2004.1337660.
3. Hinsberger, U., and R. Kolla. “Boolean Matching for Large Libraries.” In Proceedings 1998 Design and Automation Conference. 35th DAC. (Cat. No.98CH36175), 206–11, 1998. https://doi.org/10.1145/277044.277100.
