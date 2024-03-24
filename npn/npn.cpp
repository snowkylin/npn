#include <iostream>
#include <map>
#include <chrono>

#ifdef _WIN32
#    define LIBRARY_API __declspec(dllexport)
#else
#    define LIBRARY_API
#endif

using namespace std;

typedef unsigned int uint;
typedef char uint8;
//typedef unsigned long long ulonglong;
const uint MAX_NUM_INPUTS = 8;
constexpr uint Factorial(uint n) {uint res = n; for (uint i = 1; i < n; i++) {res *= i;} return res;}
const uint MAX_PERM_SIZE = Factorial(MAX_NUM_INPUTS);
const uint MAX_SIZE = (1 << MAX_NUM_INPUTS) * MAX_PERM_SIZE;
const uint MAX_POS_SIZE = 1 << MAX_NUM_INPUTS;

uint pos[MAX_PERM_SIZE][MAX_POS_SIZE];      // 8! * 2^8 = 40320 * 256 = 10,321,920
uint8 perm[MAX_PERM_SIZE][MAX_NUM_INPUTS];
uint phase_[MAX_SIZE], phase_next_[MAX_SIZE];
uint ids_[MAX_SIZE], ids_next_[MAX_SIZE];
bool res[MAX_POS_SIZE];

extern "C" {
    LIBRARY_API uint8* GeneratePermutationTable(uint8 num_inputs);
    LIBRARY_API bool* NpCanonicalRepresentative(bool* tt, uint8 num_inputs, uint* phase_p, uint* id_p, bool refinement);
    LIBRARY_API bool* NpnCanonicalRepresentative(bool* tt, uint8 num_inputs, uint* phase_p, uint* id_p, bool* not_p, bool refinement);
}

uint8* GeneratePermutationTable(uint8 num_inputs) {
    /*
     * permutation table (num_inputs = 3):
     *
     * (k = 0, num_prev_perm = 1, num_perm = 3, num_duplicate = 2)
     * 0, 0, *, *, *, *
     * *, *, 0, 0, *, *
     * *, *, *, *, 0, 0
     *
     * (k = 1, num_prev_perm = 3, num_perm = 2, num_duplicate = 1)
     * 0, 0, 1, *, 1, *
     * 1, *, 0, 0, *, 1
     * *, 1, *, 1, 0, 0
     *
     * (k = 2, num_prev_perm = 6, num_perm = 1, num_duplicate = 1)
     * 0, 0, 1, 2, 1, 2
     * 1, 2, 0, 0, 2, 1
     * 2, 1, 2, 1, 0, 0
     *
     * position table (num_inputs = 3):
     * 0, 0, 0, 0, 0, 0
     * 1, 1, 2, 2, 4, 4     (k = 0) (shift row 0 depending on the index of 0 in the permutation table)
     * 2, 4, 1, 4, 1, 2     (k = 1) (shift row 0 depending on the index of 1 in the permutation table)
     * 3, 5, 3, 6, 5, 6     (k = 1) (shift row 1 depending on the index of 1 in the permutation table)
     * 4, 2, 4, 1, 2, 1     (k = 2) (shift row 0 depending on the index of 2 in the permutation table)
     * 5, 3, 6, 3, 6, 5     (k = 2) (shift row 1 depending on the index of 2 in the permutation table)
     * 6, 6, 5, 5, 3, 3     (k = 2) (shift row 2 depending on the index of 2 in the permutation table)
     * 7, 7, 7, 7, 7, 7     (k = 2) (shift row 3 depending on the index of 2 in the permutation table)
     */
    uint n = Factorial(num_inputs);
    uint num_prev_perm = 1;
    for (uint i = 0; i < n; i++) {
        pos[i][0] = 0;
        for (uint8 j = 0; j < num_inputs; j++) perm[i][j] = num_inputs;
    }

    for (uint8 k = 0; k < num_inputs; k++) {
        uint num_levels = 1 << k;
        uint num_perm = num_inputs - k;
        uint num_duplicate = n / num_prev_perm / num_perm;
        uint id = 0;
        for (uint i = 0; i < num_prev_perm; i++) {
            for (uint8 j = 0; j < num_inputs; j++) if (perm[id][j] == num_inputs) {
                for (uint t = 0; t < num_duplicate; t++) {
                    perm[id][j] = k;
                    for (uint l = 0; l < num_levels; l++) {
                        pos[id][num_levels + l] = pos[id][l] + (1 << j);
                    }
                    id++;
                }
            }
        }
        num_prev_perm *= num_perm;
    }
    return (uint8*) perm;
}

bool* NpCanonicalRepresentative(bool* tt, uint8 num_inputs, uint* phase_p, uint* id_p, bool refinement) {
    uint n = Factorial(num_inputs);
    uint tt_size = 1 << num_inputs;
    bool all_false = true;
    for (uint i = 0; i < tt_size; i++) {
        if (tt[i]) {all_false = false; break;}
    }
    if (all_false) {
        for (uint i = 0; i < tt_size; i++) res[i] = false;
        return (bool*) res;
    }

    auto phase = phase_, phase_next = phase_next_;
    auto ids = ids_, ids_next = ids_next_;

    uint q_size = 0;
    /*
     * The initial phases are the ones that can make the first bit of the transformed truth table be 1
     * For example, when f(1, 1, 0) = 1, phase = (1, 1, 0) can make f'(0, 0, 0) = f(1, 1, 0) = 1
     */
    for (uint i = 0; i < tt_size; i++) {
        if (tt[i]) {
            phase[q_size] = i;
            ids[q_size] = 0;
            q_size++;
        }
    }
    if (refinement) {
        /*
         * Selecting the phases with minimal row sum, see Sec 3.1 of "Building a Better Boolean Matcher and Symmetry Detector" for details
         */
        uint row_sum[q_size];
        uint min_row_sum = tt_size * num_inputs;
        for (uint i = 0; i < q_size; i++) {
            row_sum[i] = 0;
            for (uint j = 0; j < tt_size; j++) {
                if (tt[j ^ phase[i]]) {
                    for (uint k = 0; k < num_inputs; k++) row_sum[i] += (j >> k) & 1;
                }
            }
            if (row_sum[i] < min_row_sum) min_row_sum = row_sum[i];
        }
        uint row_squared_sum[q_size];
        uint min_row_squared_sum = tt_size * num_inputs * num_inputs;
        for (uint i = 0; i < q_size; i++) {
            if (row_sum[i] == min_row_sum) {
                row_squared_sum[i] = 0;
                for (uint j = 0; j < tt_size; j++) {
                    if (tt[j ^ phase[i]]) {
                        uint num_ones = 0;
                        for (uint k = 0; k < num_inputs; k++) num_ones += (j >> k) & 1;
                        row_squared_sum[i] += num_ones * num_ones;
                    }
                }
                if (row_squared_sum[i] < min_row_squared_sum) min_row_squared_sum = row_squared_sum[i];
            }
        }
        uint q_next_size = 0;
        for (uint i = 0; i < q_size; i++) {
            if (row_sum[i] == min_row_sum and row_squared_sum[i] == min_row_squared_sum) {
                row_squared_sum[i] = 0;
                phase_next[q_next_size] = phase[i];
                ids_next[q_next_size] = ids[i];
                q_next_size++;
            }
        }
        q_size = q_next_size;
        swap<uint*>(phase, phase_next);
        swap<uint*>(ids, ids_next);
    }
    uint num_prev_perm = 1;
    for (uint k = 0; k < num_inputs; k++) {
        uint num_levels = 1 << k;
        uint num_perm = num_inputs - k;
        uint num_duplicate = n / num_prev_perm / num_perm;
        uint q_next_size = 0;
        /*
         * Expand each of the item in the queue for `num_perm` times with different ids (columns/first axis in the permutation table)
         * For example, 
         * for k = 0, phase = ... and id = 0, we need to keep the phase and expand id to 0+0, 0+2, 0+4, corresponding to 3 possible positions of x_0
         * for k = 1, phase = ... and id = 2, we need to keep the phase and expand id to 2+0, 2+1, corresponding to 2 possible positions of x_1
         */
        for (uint i = 0; i < q_size; i++) {
            for (uint j = 0; j < num_perm; j++) {
                phase_next[q_next_size] = phase[i];
                ids_next[q_next_size] = ids[i] + j * num_duplicate;
                q_next_size++;
            }
        }
        swap<uint*>(phase, phase_next);
        swap<uint*>(ids, ids_next);
        /*
         * Filter the queue so that only those whose `num_levels + l`th bit in the transformed truth table equals to 1 are preserved
         * The value of the bit in the transformed truth table can be calculated by 
         * 1. find the `num_levels + l`th row of the id-th column in the position table, which is the position without phase transformation
         * 2. apply bitwise XOR between the above position and phase, so as to apply phase transformation to the above position 
         * It is possible that all the `num_levels + l`th bit in the queue are equal to 0. In this case we need to keep all the items in the queue
         */
        q_size = q_next_size;
        for (uint l = 0; l < num_levels; l++) {
            q_next_size = 0;
            bool all_zeros = true;
            for (uint i = 0; i < q_size; i++) {
                uint new_pos = phase[i] ^ pos[ids[i]][l + num_levels];   // xor, true when two operands are different
                bool seq_part_l = tt[new_pos];
                if (all_zeros && seq_part_l) {
                    all_zeros = false;
                    q_next_size = 0;
                }
                if (all_zeros || seq_part_l) {
                    phase_next[q_next_size] = phase[i];
                    ids_next[q_next_size] = ids[i];
                    q_next_size++;
                }
            }
            swap<uint*>(phase, phase_next);
            swap<uint*>(ids, ids_next);
            q_size = q_next_size;
        }
        num_prev_perm *= num_perm;
    }
    uint c_phase = phase[0];
    uint c_id = ids[0];
    for (uint i = 0; i < tt_size; i++) {
        res[i] = tt[c_phase ^ pos[c_id][i]];
    }
    *phase_p = c_phase;
    *id_p = c_id;
    return (bool*) res;
}

bool* NpnCanonicalRepresentative(bool* tt, uint8 num_inputs, uint* phase_p, uint* id_p, bool* not_p, bool refinement) {
    uint tt_size = 1 << num_inputs;
    if (refinement) {
        /*
         * See Section 2.2 of "Building a Better Boolean Matcher and Symmetry Detector" for details
         */
        uint num_ones = 0;
        for (uint i = 0; i < tt_size; i++)
            if (tt[i]) num_ones++;
        if (num_ones < tt_size / 2) {
            NpCanonicalRepresentative(tt, num_inputs, phase_p, id_p, refinement);
            *not_p = false;
            return (bool*) res;
        } else if (num_ones > tt_size / 2) {
            for (uint i = 0; i < tt_size; i++) tt[i] = !tt[i];
            NpCanonicalRepresentative(tt, num_inputs, phase_p, id_p, refinement);
            *not_p = true;
            for (uint i = 0; i < tt_size; i++) tt[i] = !tt[i];
            return (bool*) res;
        }
    }
    uint c_phase_0, c_phase_1;
    uint c_id_0, c_id_1;
    bool c_0[tt_size];
    bool c_1[tt_size];
    NpCanonicalRepresentative(tt, num_inputs, &c_phase_0, &c_id_0, refinement);
    for (uint i = 0; i < tt_size; i++) {c_0[i] = res[i]; tt[i] = !tt[i];}
    NpCanonicalRepresentative(tt, num_inputs, &c_phase_1, &c_id_1, refinement);
    for (uint i = 0; i < tt_size; i++) {c_1[i] = res[i]; tt[i] = !tt[i];}
    for (uint i = 0; i < tt_size; i++) {
        if (c_0[i] && !c_1[i]) {
            *phase_p = c_phase_0;
            *id_p = c_id_0;
            *not_p = false;
            for (uint j = 0; j < tt_size; j++) res[j] = c_0[j];
            return (bool*) res;
        } else if (c_1[i] && !c_0[i] || i == tt_size - 1) {
            *phase_p = c_phase_1;
            *id_p = c_id_1;
            *not_p = true;
            for (uint j = 0; j < tt_size; j++) res[j] = c_1[j];
            return (bool*) res;
        }
    }
    return (bool*) res;
}

int main() {
//    map<ulonglong, ulonglong> counter;
    const uint num_inputs = 3;
    GeneratePermutationTable(num_inputs);
    const uint tt_size = 1 << num_inputs;
    bool tt_test[8] = {true, true, true, false, true, true, true, true};
    uint phase, id;
    bool output_inv;
    NpnCanonicalRepresentative(tt_test, 3, &phase, &id, &output_inv, true);
//    bool tt[tt_size];
//    ulonglong max_tt_num = 0;
//    for (uint i = 0; i < tt_size; i++) max_tt_num += (ulonglong)(1) << i;
//    auto start_time = chrono::system_clock::now();
//    for (ulonglong tt_num = 0; tt_num <= max_tt_num; tt_num++) {
//        for (uint i = 0; i < tt_size; i++) tt[i] = (tt_num >> i) & 1;
//        ulonglong c = NpnCanonicalRepresentative(tt, num_inputs);
//        counter[c]++;
//        if (tt_num % 10000 == 0) {
//            auto current_time = chrono::system_clock::now();
//            auto duration = chrono::duration_cast<chrono::milliseconds>(current_time - start_time);
//            cout << tt_num << " " << counter.size() << " " << double(duration.count()) / 1000 << endl;
//            start_time = current_time;
//        }
//    }
//    cout << counter.size() << endl;
    return 0;
}
