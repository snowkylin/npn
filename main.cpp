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

uint8 phase_[MAX_SIZE], pos_[MAX_SIZE][MAX_POS_SIZE], phase_next_[MAX_SIZE], pos_next_[MAX_SIZE][MAX_POS_SIZE];
uint ids_[MAX_SIZE];
bool perm_[MAX_SIZE][MAX_NUM_INPUTS], perm_next_[MAX_SIZE][MAX_NUM_INPUTS];

//void SwapArrays(uint8 * &a, uint8 * &b) {
//    uint8 * c = a;
//    a = b;
//    b = c;
//}

ulonglong NpCanonicalRepresentation(bool* tt, uint8 num_inputs) {
    auto phase = phase_;
    auto pos = pos_;
    auto perm = perm_;
    auto phase_next = phase_next_;
    auto pos_next = pos_next_;
    auto perm_next = perm_next_;
    auto ids = ids_;
    uint8 tt_size = 1 << num_inputs;
    bool all_false = true;
    for (uint8 i = 0; i < tt_size; i++) {
        if (tt[i]) {all_false = false; break;}
    }
    if (all_false) return 0;
    uint q_size = 0;
    for (uint8 i = 0; i < tt_size; i++) {
        if (tt[i]) {
            phase[q_size] = i;
            for (uint8 j = 0; j < num_inputs; j++) perm[q_size][j] = true;
            q_size++;
        }
    }
    for (uint8 k = 0; k < num_inputs; k++) {
        uint8 num_levels = 1 << k;
        uint q_next_size = 0;
        for (uint i = 0; i < q_size; i++) {
            for (uint8 j = 0; j < num_inputs; j++) {
                if (perm[i][j]) {
                    for (uint8 l = 0; l < num_levels; l++) {
                        pos_next[q_next_size][l] = pos[i][l];
                        pos_next[q_next_size][l + num_levels] = pos[i][l] + (1 << j);
                    }
                    phase_next[q_next_size] = phase[i];
                    ids[q_next_size] = q_next_size;
                    for (uint8 l = 0; l < num_inputs; l++) perm_next[q_next_size][l] = perm[i][l];
                    perm_next[q_next_size][j] = false;
                    q_next_size++;
                }
            }
        }
        uint pos_next_size = q_next_size;
        for (uint8 l = 0; l < num_levels; l++) {
            bool all_zeros = true;
            uint post_next_new_size = 0;
            for (uint i = 0; i < pos_next_size; i++) {
                uint8 pos_1_phase_l = phase_next[ids[i]] ^ pos_next[ids[i]][l + num_levels];
                bool seq_part_l = tt[pos_1_phase_l];
                if (all_zeros && seq_part_l) {
                    all_zeros = false;
                    post_next_new_size = 0;
                }
                if (all_zeros || seq_part_l) {
                    ids[post_next_new_size] = ids[i];
                    post_next_new_size++;
                }
            }
            pos_next_size = post_next_new_size;
        }
        for (uint i = 0; i < pos_next_size; i++) {
            phase[i] = phase_next[ids[i]];
            for (uint8 l = 0; l < num_levels * 2; l++) pos[i][l] = pos_next[ids[i]][l];
            for (uint8 j = 0; j < num_inputs; j++) perm[i][j] = perm_next[ids[i]][j];
        }
//        swap<uint8*>(phase, phase_next);
//        swap<uint8*>(pos, pos_next);
//        swap<bool (*) [6]>(perm, perm_next);
        q_size = pos_next_size;
    }
    ulonglong c = 0;
    for (uint8 i = 0; i < tt_size; i++) {
        if (tt[phase[0] ^ pos[0][i]]) c+= (ulonglong)(1) << (tt_size - 1 - i);
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
    uint8 tt_size = 1 << num_inputs;
    bool tt[tt_size]; // = {true, true, true, false, true, true, true, true} ;
    ulonglong max_tt_num = 0;
    for (uint8 i = 0; i < tt_size; i++) max_tt_num += (ulonglong)(1) << i;
    auto start_time = chrono::system_clock::now();
    for (ulonglong tt_num = 0; tt_num <= max_tt_num; tt_num++) {
        for (uint8 i = 0; i < tt_size; i++) tt[i] = (tt_num >> i) & 1;
        ulonglong c = NpnCanonicalRepresentation(tt, num_inputs);
        counter[c]++;
//        cout << NpCanonicalRepresentation(tt, 3) << endl;
        if (tt_num % 100 == 0) {
            auto current_time = chrono::system_clock::now();
            auto duration = chrono::duration_cast<chrono::milliseconds>(current_time - start_time);
            cout << tt_num << " " << counter.size() << " " << double(duration.count()) / 1000 << endl;
            start_time = current_time;
        }
    }
    cout << counter.size() << endl;
    return 0;
}
