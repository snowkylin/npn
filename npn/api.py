import ctypes
import math
import os
import itertools
import platform

this_dir = os.path.abspath(os.path.dirname(__file__))
if platform.system() == 'Windows':
    mod = ctypes.cdll.LoadLibrary(this_dir + "/libnpn.dll")
else:
    mod = ctypes.cdll.LoadLibrary(this_dir + "/libnpn.so")
# mod = ctypes.cdll.LoadLibrary("cmake-build-release/libnpn.dll")
# mod = ctypes.cdll.LoadLibrary("cmake-build-release/libnpn.so")

max_num_inputs = 8

c_bool_p = ctypes.POINTER(ctypes.c_bool)

generate_permutation_table_cpp = mod.GeneratePermutationTable
generate_permutation_table_cpp.argtypes = (ctypes.c_uint8,)
generate_permutation_table_cpp.restype = ctypes.POINTER(ctypes.c_uint8)

npn_canonical_representative_cpp = mod.NpnCanonicalRepresentative
npn_canonical_representative_cpp.argtypes = (c_bool_p, ctypes.c_uint8,
                                             ctypes.POINTER(ctypes.c_uint), ctypes.POINTER(ctypes.c_uint), c_bool_p, ctypes.c_bool)
npn_canonical_representative_cpp.restype = c_bool_p

current_num_inputs = -1
perm_table = None


def generate_permutation_table(num_inputs):
    global current_num_inputs
    if num_inputs > max_num_inputs:
        raise ValueError("This package only support boolean functions with number of inputs <= %d" % max_num_inputs)
    if num_inputs != current_num_inputs:
        global perm_table
        perm_table = generate_permutation_table_cpp(num_inputs)
        current_num_inputs = num_inputs
    else:
        print("Warning: the permutation table with num_inputs = %d is already generated" % num_inputs)


def npn_canonical_representative(tt, num_inputs=None, return_details=False, refinement=False):
    if num_inputs is None:
        num_inputs = base_2_log(len(tt))
    if num_inputs != current_num_inputs:
        generate_permutation_table(num_inputs)
        print("Permutation table generated for %d-input functions" % num_inputs)
    if isinstance(tt, list) or isinstance(tt, tuple):
        tt = (ctypes.c_bool * len(tt))(*tt)
    phase = ctypes.c_uint()
    id = ctypes.c_uint()
    output_inv = ctypes.c_bool()
    c_ptr = npn_canonical_representative_cpp(tt, num_inputs, ctypes.pointer(phase), ctypes.pointer(id), ctypes.pointer(output_inv), refinement)
    c = [c_ptr[i] for i in range(len(tt))]
    if return_details:
        phase = [bool(phase.value & (1 << i)) for i in range(num_inputs)]
        p = [perm_table[i] for i in range(max_num_inputs * id.value, max_num_inputs * id.value + num_inputs)]
        return c, phase, p, output_inv.value
    else:
        return c


# static inline int      Abc_Base2Log( unsigned n )
#   { int r; if ( n < 2 ) return (int)n; for ( r = 0, n--; n; n >>= 1, r++ ) {}; return r; }
def base_2_log(n: int):     # see src/misc/util/abc_global.h in abc
    if n < 2:
        return n
    else:
        r = 0
        n -= 1
        while n > 0:
            r += 1
            n >>= 1
        return r


def transform_tt(tt, phase, perm, output_inv):
    res = [False] * len(tt)
    num_inputs = base_2_log(len(tt))
    for i, value in enumerate(itertools.product([False, True], repeat=num_inputs)):   # value = (x_2, x_1, x_0)
        value = list(reversed(value))       # reverse `value` to (x_0, x_1, x_2) so that x_i can be accessed by index i
        value_new = [0] * num_inputs
        for j in range(num_inputs):
            value_new[j] = value[perm[j]]   # permutate first, then change the phase of the input
            if phase[j]:                    # x_i -> not(x_i), note that `phase` is already reversed
                value_new[j] = not value_new[j]
        id = 0
        for j in range(num_inputs):
            if value_new[j]:
                id += 2 ** j
        res[i] = not(tt[id]) if output_inv else tt[id]
    return res


def tt_to_int(tt):      # use reverse order, see line 1482 of src/misc/util/utilTruth.h in abc
    tt_size = len(tt)
    res = 0
    for i, b in enumerate(tt):
        if b:
            res += 2 ** (tt_size - 1 - i)
    return res


def int_to_tt(n, num_inputs=None):
    if num_inputs is None:
        num_inputs = base_2_log(base_2_log(n + 1))
    tt = [False] * (2 ** num_inputs)
    for i in reversed(range(2 ** num_inputs)):
        tt[i] = n % 2 == 1
        n >>= 1
        if n == 0:
            break
    return tt


# num_inputs = 2
# generate_permutation_table(num_inputs)
# # p = [0] * num_inputs
# # p_ctypes = (ctypes.c_uint * len(p))(*p)
# tt = [False, True, False, True]
# # tt = [False, False, False, True, False, False, False, False]
# # tt = [True, True, True, False, True, True, True, True]  # f(x_2, x_1, x_0) = x_2 + not(x_1) + not(x_0)
# c, phase, perm, output_inv = npn_canonical_representative(tt, return_details=True)
# npn_tt = transform_tt(tt, phase, perm, output_inv)
# c_npn_tt = tt_to_int(npn_tt)
# pass



