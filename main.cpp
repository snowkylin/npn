#include <iostream>
#include <map>
#include <chrono>

using namespace std;

typedef unsigned int uint;
typedef char uint8;
typedef unsigned long long ulonglong;
const uint8 MAX_NUM_INPUTS = 6;
constexpr uint Factorial(uint n) {uint res = n; for (uint i = 1; i < n; i++) {res *= i;} return res;}
const uint MAX_SIZE = (1 << MAX_NUM_INPUTS) * Factorial(MAX_NUM_INPUTS);
const uint MAX_POS_SIZE = 1 << MAX_NUM_INPUTS;

uint8 pos[MAX_SIZE][MAX_POS_SIZE];
bool perm[MAX_SIZE][MAX_NUM_INPUTS];
uint8 phase_[MAX_SIZE], phase_next_[MAX_SIZE];
uint ids_[MAX_SIZE], ids_next_[MAX_SIZE];

void GeneratePermutationTable(uint8 num_inputs) {
    uint n = Factorial(num_inputs);
    uint num_prev_perm = 1;
    for (uint i = 0; i < n; i++) {
        pos[i][0] = 0;
        for (uint8 j = 0; j < num_inputs; j++) perm[i][j] = true;
    }

    for (uint8 k = 0; k < num_inputs; k++) {
        uint8 num_levels = 1 << k;
        uint8 num_perm = num_inputs - k;
        uint num_duplicate = n / num_prev_perm / num_perm;
        uint id = 0;
        for (uint i = 0; i < num_prev_perm; i++) {
            for (uint8 j = 0; j < num_inputs; j++) if (perm[id][j]) {
                for (uint t = 0; t < num_duplicate; t++) {
                    perm[id][j] = false;
                    for (uint8 l = 0; l < num_levels; l++) {
                        pos[id][num_levels + l] = pos[id][l] + (1 << j);
                    }
                    id++;
                }
            }
        }
        num_prev_perm *= num_perm;
    }
}

ulonglong NpCanonicalRepresentation(bool* tt, uint8 num_inputs) {
    uint n = Factorial(num_inputs);
    uint8 tt_size = 1 << num_inputs;
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
                uint8 pos_1_phase_l = phase[i] ^ pos[ids[i]][l + num_levels];
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
    for (uint8 i = 0; i < tt_size; i++) {
        if (tt[phase[0] ^ pos[ids[0]][i]]) c+= (ulonglong)(1) << (tt_size - 1 - i);
    }
    return c;
}

ulonglong NpnCanonicalRepresentation(bool* tt, uint8 num_inputs) {
    ulonglong c_1 = NpCanonicalRepresentation(tt, num_inputs);
    for (uint8 i = 0; i < 1 << num_inputs; i++) tt[i] = !tt[i];
    ulonglong c_2 = NpCanonicalRepresentation(tt, num_inputs);
    for (uint8 i = 0; i < 1 << num_inputs; i++) tt[i] = !tt[i];
    return (c_1 > c_2)? c_1 : c_2;
}

int main() {
    map<ulonglong, ulonglong> counter;
    uint8 num_inputs = 6;
    GeneratePermutationTable(num_inputs);
    uint8 tt_size = 1 << num_inputs;
//    bool tt_test[8] = {true, true, true, false, true, true, true, true};
//    NpnCanonicalRepresentation(tt_test, 3);
    bool tt[tt_size];
    ulonglong max_tt_num = 0;
    for (uint8 i = 0; i < tt_size; i++) max_tt_num += (ulonglong)(1) << i;
    auto start_time = chrono::system_clock::now();
    for (ulonglong tt_num = 0; tt_num <= max_tt_num; tt_num++) {
        for (uint8 i = 0; i < tt_size; i++) tt[i] = (tt_num >> i) & 1;
        ulonglong c = NpnCanonicalRepresentation(tt, num_inputs);
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
}
