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
typedef unsigned long long ulonglong;
const uint8 MAX_NUM_INPUTS = 6;
constexpr uint Factorial(uint n) {uint res = n; for (uint i = 1; i < n; i++) {res *= i;} return res;}
const uint MAX_PERM_SIZE = Factorial(MAX_NUM_INPUTS);
const uint MAX_SIZE = (1 << MAX_NUM_INPUTS) * MAX_PERM_SIZE;
const uint MAX_POS_SIZE = 1 << MAX_NUM_INPUTS;

uint8 pos[MAX_PERM_SIZE][MAX_POS_SIZE];
uint8 perm[MAX_PERM_SIZE][MAX_NUM_INPUTS];
uint8 phase_[MAX_SIZE], phase_next_[MAX_SIZE];
uint ids_[MAX_SIZE], ids_next_[MAX_SIZE];

uint8 c_num_inputs;

extern "C" {
    LIBRARY_API uint8* GeneratePermutationTable(uint8 num_inputs);
    LIBRARY_API ulonglong NpCanonicalRepresentative(bool* tt, uint8 num_inputs, uint8* phase_p, uint* id_p);
    LIBRARY_API ulonglong NpnCanonicalRepresentative(bool* tt, uint8 num_inputs, uint8* phase_p, uint* id_p, bool* not_p);
}

uint8* GeneratePermutationTable(uint8 num_inputs) {
    uint n = Factorial(num_inputs);
    uint num_prev_perm = 1;
    for (uint i = 0; i < n; i++) {
        pos[i][0] = 0;
        for (uint8 j = 0; j < num_inputs; j++) perm[i][j] = num_inputs;
    }

    for (uint8 k = 0; k < num_inputs; k++) {
        uint8 num_levels = 1 << k;
        uint8 num_perm = num_inputs - k;
        uint num_duplicate = n / num_prev_perm / num_perm;
        uint id = 0;
        for (uint i = 0; i < num_prev_perm; i++) {
            for (uint8 j = 0; j < num_inputs; j++) if (perm[id][j] == num_inputs) {
                for (uint t = 0; t < num_duplicate; t++) {
                    perm[id][j] = k;
                    for (uint8 l = 0; l < num_levels; l++) {
                        pos[id][num_levels + l] = pos[id][l] + (1 << j);
                    }
                    id++;
                }
            }
        }
        num_prev_perm *= num_perm;
    }
    c_num_inputs = num_inputs;
    return (uint8*) perm;
}

ulonglong NpCanonicalRepresentative(bool* tt, uint8 num_inputs, uint8* phase_p, uint* id_p) {
    uint n = Factorial(num_inputs);
    uint tt_size = 1 << num_inputs;
    bool all_false = true;
    for (uint8 i = 0; i < tt_size; i++) {
        if (tt[i]) {all_false = false; break;}
    }
    if (all_false) return 0;

    auto phase = phase_, phase_next = phase_next_;
    auto ids = ids_, ids_next = ids_next_;

    uint q_size = 0;
    for (uint8 i = 0; i < tt_size; i++) {
        if (tt[i]) {
            phase[q_size] = i;
            ids[q_size] = 0;
            q_size++;
        }
    }
    uint num_prev_perm = 1;
    for (uint8 k = 0; k < num_inputs; k++) {
        uint8 num_levels = 1 << k;
        uint8 num_perm = num_inputs - k;
        uint num_duplicate = n / num_prev_perm / num_perm;
        uint q_next_size = 0;
        for (uint i = 0; i < q_size; i++) {
            for (uint8 j = 0; j < num_perm; j++) {
                phase_next[q_next_size] = phase[i];
                ids_next[q_next_size] = ids[i] + j * num_duplicate;
                q_next_size++;
            }
        }
        swap<uint*>(ids, ids_next);
        swap<uint8*>(phase, phase_next);
        q_size = q_next_size;
        for (uint8 l = 0; l < num_levels; l++) {
            q_next_size = 0;
            bool all_zeros = true;
            for (uint i = 0; i < q_size; i++) {
                uint8 pos_1_phase_l = phase[i] ^ pos[ids[i]][l + num_levels];   // xor, true when two operands are different
                bool seq_part_l = tt[pos_1_phase_l];
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
            swap<uint*>(ids, ids_next);
            swap<uint8*>(phase, phase_next);
            q_size = q_next_size;
        }
        num_prev_perm *= num_perm;
    }
    ulonglong c = 0;
    uint8 c_phase = phase[0];
    uint c_id = ids[0];
    for (uint8 i = 0; i < tt_size; i++) {
        if (tt[c_phase ^ pos[c_id][i]]) c+= (ulonglong)(1) << (tt_size - 1 - i);
    }
    *phase_p = c_phase;
    *id_p = c_id;
    return c;
}

ulonglong NpnCanonicalRepresentative(bool* tt, uint8 num_inputs, uint8* phase_p, uint* id_p, bool* not_p) {
    c_num_inputs = num_inputs;
    uint8 c_phase_0, c_phase_1;
    uint c_id_0, c_id_1;
    ulonglong c_1 = NpCanonicalRepresentative(tt, num_inputs, &c_phase_0, &c_id_0);
    for (uint8 i = 0; i < 1 << num_inputs; i++) tt[i] = !tt[i];
    ulonglong c_2 = NpCanonicalRepresentative(tt, num_inputs, &c_phase_1, &c_id_1);
    for (uint8 i = 0; i < 1 << num_inputs; i++) tt[i] = !tt[i];
    if (c_1 > c_2) {
        *phase_p = c_phase_0;
        *id_p = c_id_0;
        *not_p = false;
        return c_1;
    } else {
        *phase_p = c_phase_1;
        *id_p = c_id_1;
        *not_p = true;
        return c_2;
    }
}

/*int main() {
    map<ulonglong, ulonglong> counter;
    const uint8 num_inputs = 6;
    GeneratePermutationTable(num_inputs);
    const uint tt_size = 1 << num_inputs;
//    bool tt_test[8] = {true, true, true, false, true, true, true, true};
//    NpnCanonicalRepresentative(tt_test, 3);
    bool tt[tt_size];
    ulonglong max_tt_num = 0;
    for (uint8 i = 0; i < tt_size; i++) max_tt_num += (ulonglong)(1) << i;
    auto start_time = chrono::system_clock::now();
    for (ulonglong tt_num = 0; tt_num <= max_tt_num; tt_num++) {
        for (uint8 i = 0; i < tt_size; i++) tt[i] = (tt_num >> i) & 1;
        ulonglong c = NpnCanonicalRepresentative(tt, num_inputs);
        counter[c]++;
        if (tt_num % 10000 == 0) {
            auto current_time = chrono::system_clock::now();
            auto duration = chrono::duration_cast<chrono::milliseconds>(current_time - start_time);
            cout << tt_num << " " << counter.size() << " " << double(duration.count()) / 1000 << endl;
            start_time = current_time;
        }
    }
    cout << counter.size() << endl;
    return 0;
}*/
