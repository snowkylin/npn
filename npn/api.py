import ctypes
import math
import os
import platform

this_dir = os.path.abspath(os.path.dirname(__file__))
if (platform.system() == 'Windows'):
    mod = ctypes.cdll.LoadLibrary(this_dir + "/npn.dll")
else:
    mod = ctypes.cdll.LoadLibrary(this_dir + "/libnpn.so")
# mod = ctypes.cdll.LoadLibrary("cmake-build-release/npn.dll")
# mod = ctypes.cdll.LoadLibrary("cmake-build-release/libnpn.so")

c_bool_p = ctypes.POINTER(ctypes.c_bool)

generate_permutation_table_cpp = mod.GeneratePermutationTable
generate_permutation_table_cpp.argtypes = (ctypes.c_uint8,)
generate_permutation_table_cpp.restype = ctypes.c_void_p

npn_canonical_representative_cpp = mod.NpnCanonicalRepresentative
npn_canonical_representative_cpp.argtypes = (c_bool_p, ctypes.c_uint8)
npn_canonical_representative_cpp.restype = ctypes.c_ulonglong

current_num_inputs = -1


def generate_permutation_table(num_inputs):
    global current_num_inputs
    if num_inputs > 6:
        raise ValueError("This package only support boolean functions with number of inputs <= 6")
    if num_inputs != current_num_inputs:
        generate_permutation_table_cpp(num_inputs)
        current_num_inputs = num_inputs
    else:
        print("Warning: the permutation table with num_inputs = %d is already generated" % num_inputs)


def npn_canonical_representative(tt, num_inputs=None):
    if num_inputs is None:
        num_inputs = int(math.log2(len(tt)))
    if num_inputs != current_num_inputs:
        generate_permutation_table(num_inputs)
        print("Permutation table generated for %d-input functions" % num_inputs)
    if isinstance(tt, list) or isinstance(tt, tuple):
        tt = (ctypes.c_bool * len(tt))(*tt)
    return npn_canonical_representative_cpp(tt, num_inputs)



