import npn
import itertools


def test_npn():
    num_npn_classes = {1: 2, 2: 4, 3: 14, 4: 222}
    for num_inputs in num_npn_classes.keys():
        npn_classes = set()
        for i, tt in enumerate(itertools.product([False, True], repeat=2 ** num_inputs)):
            c = npn.npn_canonical_representative(tt)
            npn_classes.add(c)
        print("number of npn classes for %d-input boolean functions: %d" % (num_inputs, len(npn_classes)))
        assert len(npn_classes) == num_npn_classes[num_inputs]
