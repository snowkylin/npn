import npn
import itertools


def test_npn():
    num_npn_classes = {1: 2, 2: 4, 3: 14, 4: 222}
    for num_inputs in num_npn_classes.keys():
        npn_classes = set()
        for i, tt in enumerate(itertools.product([False, True], repeat=2 ** num_inputs)):
            c_tt, phase, perm, output_inv = npn.npn_canonical_representative(tt, return_details=True)
            npn_tt = npn.transform_tt(tt, phase, perm, output_inv)
            equ = True
            for c_minterm, npn_minterm in zip(c_tt, npn_tt):
                if c_minterm != npn_minterm:
                    equ = False
                    break
            assert equ
            npn_classes.add(tuple(c_tt))
        print("number of npn classes for %d-input boolean functions: %d" % (num_inputs, len(npn_classes)))
        assert len(npn_classes) == num_npn_classes[num_inputs]


if __name__ == "__main__":
    test_npn()
