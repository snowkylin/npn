import npn
import itertools


def test_npn():
    num_npn_classes = {1: 2, 2: 4, 3: 14, 4: 222}
    for num_inputs in num_npn_classes.keys():
        npn_classes = set()
        for i, tt in enumerate(itertools.product([False, True], repeat=2 ** num_inputs)):
            c, phase, perm, output_inv = npn.npn_canonical_representative(tt, return_details=True)
            # print(tt, c, phase, perm, output_inv, end=' ')
            npn_tt = npn.transform_tt(tt, phase, perm, output_inv)
            c_npn_tt = npn.tt_to_int(npn_tt)
            # print(npn_tt, c_npn_tt)
            assert c_npn_tt == c
            npn_classes.add(c)
        print("number of npn classes for %d-input boolean functions: %d" % (num_inputs, len(npn_classes)))
        assert len(npn_classes) == num_npn_classes[num_inputs]
