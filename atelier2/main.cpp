#include <bits/stdc++.h>
#include "sha256.h"
using namespace std;

// Utility functions
static inline string to_hex(const vector<int>& bits256) {
    if (bits256.size() != 256) throw runtime_error("to_hex expects 256 bits");
    static const char* lut = "0123456789abcdef";
    string out; out.reserve(64);
    for (int b = 0; b < 256; b += 8) {
        unsigned v = 0;
        for (int i = 0; i < 8; ++i) v = (v << 1) | (bits256[b + i] & 1);
        out.push_back(lut[(v >> 4) & 0xF]);
        out.push_back(lut[v & 0xF]);
    }
    return out;
}

static inline vector<int> hex_to_bits(const string& hex) {
    static const string digits = "0123456789abcdef";
    vector<int> bits; bits.reserve(hex.size()*4);
    for (char c : hex) {
        int v;
        if (c >= '0' && c <= '9') v = c - '0';
        else v = 10 + (tolower(c) - 'a');
        for (int i = 3; i >= 0; --i) bits.push_back((v >> i) & 1);
    }
    return bits;
}

static inline vector<int> text_to_bits(const string& s) {
    vector<int> bits; bits.reserve(s.size()*8);
    for (unsigned char c : s)
        for (int i = 7; i >= 0; --i)
            bits.push_back((c >> i) & 1);
    return bits;
}

// Cellular Automaton (1D, r=1, binary)

struct CellularAutomaton1D {
    vector<int> state;
    CellularAutomaton1D() = default;
    explicit CellularAutomaton1D(const vector<int>& init) : state(init) {}

    static inline int next_cell(int L, int C, int R, uint32_t rule) {
        int idx = (L << 2) | (C << 1) | R;
        return (rule >> idx) & 1;
    }

    void evolve(uint32_t rule) {
        vector<int> next(state.size());
        size_t n = state.size();
        for (size_t i = 0; i < n; ++i) {
            int L = state[(i + n - 1) % n];
            int C = state[i];
            int R = state[(i + 1) % n];
            next[i] = next_cell(L, C, R, rule);
        }
        state.swap(next);
    }
};

// AC Hash Function

static vector<int> fold_to_256(const vector<int>& s) {
    vector<int> out(256, 0);
    for (size_t i = 0; i < s.size(); ++i)
        out[i % 256] ^= (s[i] & 1);
    return out;
}

static vector<int> init_state_from_bits(const vector<int>& bits, size_t W) {
    vector<int> st(W, 0);
    for (size_t i = 0; i < W; ++i)
        st[i] = bits[i % bits.size()];
    return st;
}

string ac_hash(const string& input, uint32_t rule, size_t steps) {
    vector<int> bits = text_to_bits(input);
    size_t W = max<size_t>(256, bits.size());
    CellularAutomaton1D ca(init_state_from_bits(bits, W));
    vector<int> acc(256, 0);

    for (size_t t = 0; t < steps; ++t) {
        vector<int> folded = fold_to_256(ca.state);
        for (int i = 0; i < 256; ++i)
            acc[i] ^= folded[(i + (t*13)%256)];
        ca.evolve(rule);
    }

    return to_hex(acc);
}

// Blockchain Structures
struct Block {
    int index;
    string prev_hash;
    string data;
    uint64_t nonce = 0;
    string timestamp;
    string hash;
};

enum class HashMode { SHA256_MODE, AC_MODE };

struct SimpleBlockchain {
    vector<Block> chain;
    HashMode mode = HashMode::SHA256_MODE;
    uint32_t ac_rule = 30;
    size_t ac_steps = 128;
    int difficulty_prefix_zeros = 4;

    static string now_iso8601() {
        time_t t = time(nullptr);
        tm tm = *gmtime(&t);
        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm);
        return string(buf);
    }

    static string block_payload(const Block& b) {
        return to_string(b.index) + "|" + b.prev_hash + "|" + b.data + "|" +
               to_string(b.nonce) + "|" + b.timestamp;
    }

    string compute_hash(const Block& b) const {
        string payload = block_payload(b);
        if (mode == HashMode::SHA256_MODE)
            return SHA256::hash(payload);
        return ac_hash(payload, ac_rule, ac_steps);
    }

    bool valid_hash(const string& h) const {
        for (int i = 0; i < difficulty_prefix_zeros; ++i)
            if (h[i] != '0') return false;
        return true;
    }

    void add_genesis() {
        Block g{0, string(64, '0'), "Genesis", 0, now_iso8601(), ""};
        g.hash = compute_hash(g);
        while (!valid_hash(g.hash)) {
            g.nonce++;
            g.hash = compute_hash(g);
        }
        chain.push_back(g);
    }

    pair<Block, uint64_t> mine_next(const string& data) {
        Block b;
        b.index = chain.size();
        b.prev_hash = chain.back().hash;
        b.data = data;
        b.timestamp = now_iso8601();
        uint64_t iters = 0;
        b.hash = compute_hash(b);
        while (!valid_hash(b.hash)) {
            b.nonce++; iters++;
            b.hash = compute_hash(b);
        }
        return {b, iters};
    }

    bool validate_chain() const {
        for (size_t i = 1; i < chain.size(); ++i) {
            const auto& prev = chain[i-1];
            const auto& cur = chain[i];
            if (cur.prev_hash != prev.hash) return false;
            if (compute_hash(cur) != cur.hash) return false;
            if (!valid_hash(cur.hash)) return false;
        }
        return true;
    }
};


// Analysis & Tests
struct Timer {
    chrono::high_resolution_clock::time_point t0;
    void start() { t0 = chrono::high_resolution_clock::now(); }
    double stop_s() {
        return chrono::duration<double>(
            chrono::high_resolution_clock::now() - t0).count();
    }
};

int bit_diff(const string& h1, const string& h2) {
    auto b1 = hex_to_bits(h1);
    auto b2 = hex_to_bits(h2);
    int diff = 0;
    for (int i = 0; i < 256; ++i) diff += (b1[i] ^ b2[i]);
    return diff;
}

double avalanche_test(uint32_t rule, size_t steps) {
    mt19937_64 rng(42);
    double total = 0;
    for (int t = 0; t < 100; ++t) {
        string m(32, '\0');
        for (char& c : m) c = rng() & 0xFF;
        string m2 = m; m2[0] ^= 1; // flip one bit
        string h1 = ac_hash(m, rule, steps);
        string h2 = ac_hash(m2, rule, steps);
        total += bit_diff(h1, h2);
    }
    return total / 100;
}

double bit_distribution(uint32_t rule, size_t steps) {
    int ones = 0;
    int total = 0;
    for (int i = 0; i < 500; ++i) {
        string h = ac_hash("msg" + to_string(i), rule, steps);
        auto bits = hex_to_bits(h);
        for (int b : bits) { ones += b; total++; }
    }
    return 100.0 * ones / total;
}

// Main

int main() {
    cout << "Testing AC_HASH and Blockchain integration...\n\n";

    cout << "ac_hash('hello', rule=30, steps=128) = "
         << ac_hash("hello", 30, 128) << "\n\n";

    SimpleBlockchain bc;
    bc.mode = HashMode::AC_MODE;
    bc.ac_rule = 30;
    bc.add_genesis();
    auto [blk, iters] = bc.mine_next("Block 1");
    bc.chain.push_back(blk);
    cout << "Blockchain valid? " << (bc.validate_chain() ? "YES" : "NO") << "\n\n";

    cout << "Avalanche effect (Rule 30): "
         << fixed << setprecision(2)
         << avalanche_test(30, 128) / 256 * 100 << "% bits changed\n";

    cout << "Bit distribution (Rule 30): "
         << fixed << setprecision(2)
         << bit_distribution(30, 128) << "% ones\n";

    cout << "\n-- End of Tests --\n";
}
