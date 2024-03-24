import npn
import itertools
import time
import random


def test_npn():
    num_npn_classes = {1: 2, 2: 4, 3: 14, 4: 222}   # 5: 616126
    start_time = time.time()
    for num_inputs in num_npn_classes.keys():
        npn_classes = set()
        for i, tt in enumerate(itertools.product([False, True], repeat=2 ** num_inputs)):
            c_tt, phase, perm, output_inv = npn.npn_canonical_representative(tt, return_details=True, refinement=False)
            npn_tt = npn.transform_tt(tt, phase, perm, output_inv)
            equ = True
            for c_minterm, npn_minterm in zip(c_tt, npn_tt):
                if c_minterm != npn_minterm:
                    equ = False
                    break
            assert equ
            if i % 100000 == 0 and i != 0:
                print("%d truth tables computed, #(NPN) = %d, time elapsed %.2fs" %
                      (i, len(npn_classes), time.time() - start_time))
            npn_classes.add(tuple(c_tt))
        print("number of npn classes for %d-input boolean functions: %d" % (num_inputs, len(npn_classes)))
        assert len(npn_classes) == num_npn_classes[num_inputs]
    random.seed(0)
    num_test_tts = 10
    for refinement in [True, False]:
        start_time = time.time()
        count = 0
        for tt in itertools.product([False, True], repeat=2 ** 8):
            if count != 0:
                c_tt = npn.npn_canonical_representative(tt, refinement=refinement)
            count += 1
            if count > num_test_tts:
                break
        print("average time cost (refinement=%s): %.2f" % (refinement, time.time() - start_time))


if __name__ == "__main__":
    test_npn()
