#define SHA256_DIGEST_SIZE 32

#define F1(x,y,z) (bitselect(z,y,x))
#define F0(x,y,z) (bitselect (x, y, (x ^ z)))
#define shr32(x,n) ((x) >> (n))
#define rotl32(a,n) rotate((a), (n))

uint SWAP (uint val){
    return (rotate(((val) & 0x00FF00FF), 24U) | rotate(((val) & 0xFF00FF00), 8U));
}

#define S0(x) (rotl32 ((x), 25u) ^ rotl32 ((x), 14u) ^ shr32 ((x),  3u))
#define S1(x) (rotl32 ((x), 15u) ^ rotl32 ((x), 13u) ^ shr32 ((x), 10u))
#define S2(x) (rotl32 ((x), 30u) ^ rotl32 ((x), 19u) ^ rotl32 ((x), 10u))
#define S3(x) (rotl32 ((x), 26u) ^ rotl32 ((x), 21u) ^ rotl32 ((x),  7u))

constant uint k_sha256[64] = {
  0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
  0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
  0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
  0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
  0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
  0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
  0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
  0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
  0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
  0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
  0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u,
  0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
  0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u,
  0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
  0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
  0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u,
};

#define SHA256_STEP(F0a, F1a, a, b, c, d, e, f, g, h, x, K)  \
{                                                            \
  h += K;                                                    \
  h += x;                                                    \
  h += S3 (e);                                               \
  h += F1a (e,f,g);                                          \
  d += h;                                                    \
  h += S2 (a);                                               \
  h += F0a (a,b,c);                                          \
}

#define SHA256_EXPAND(x, y, z, w) (S1(x) + y + S0(z) + w)

static void sha256_process2(const uint *W, uint *digest){
  uint a = digest[0];
  uint b = digest[1];
  uint c = digest[2];
  uint d = digest[3];
  uint e = digest[4];
  uint f = digest[5];
  uint g = digest[6];
  uint h = digest[7];

  uint w0_t = W[0];
  uint w1_t = W[1];
  uint w2_t = W[2];
  uint w3_t = W[3];
  uint w4_t = W[4];
  uint w5_t = W[5];
  uint w6_t = W[6];
  uint w7_t = W[7];
  uint w8_t = W[8];
  uint w9_t = W[9];
  uint wa_t = W[10];
  uint wb_t = W[11];
  uint wc_t = W[12];
  uint wd_t = W[13];
  uint we_t = W[14];
  uint wf_t = W[15];

  #define ROUND_EXPAND(i)                          \
  {                                                \
    w0_t = SHA256_EXPAND(we_t, w9_t, w1_t, w0_t);  \
    w1_t = SHA256_EXPAND(wf_t, wa_t, w2_t, w1_t);  \
    w2_t = SHA256_EXPAND(w0_t, wb_t, w3_t, w2_t);  \
    w3_t = SHA256_EXPAND(w1_t, wc_t, w4_t, w3_t);  \
    w4_t = SHA256_EXPAND(w2_t, wd_t, w5_t, w4_t);  \
    w5_t = SHA256_EXPAND(w3_t, we_t, w6_t, w5_t);  \
    w6_t = SHA256_EXPAND(w4_t, wf_t, w7_t, w6_t);  \
    w7_t = SHA256_EXPAND(w5_t, w0_t, w8_t, w7_t);  \
    w8_t = SHA256_EXPAND(w6_t, w1_t, w9_t, w8_t);  \
    w9_t = SHA256_EXPAND(w7_t, w2_t, wa_t, w9_t);  \
    wa_t = SHA256_EXPAND(w8_t, w3_t, wb_t, wa_t);  \
    wb_t = SHA256_EXPAND(w9_t, w4_t, wc_t, wb_t);  \
    wc_t = SHA256_EXPAND(wa_t, w5_t, wd_t, wc_t);  \
    wd_t = SHA256_EXPAND(wb_t, w6_t, we_t, wd_t);  \
    we_t = SHA256_EXPAND(wc_t, w7_t, wf_t, we_t);  \
    wf_t = SHA256_EXPAND(wd_t, w8_t, w0_t, wf_t);  \
  }

  #define ROUND_STEP(i)                                                  \
  {                                                                      \
    SHA256_STEP(F0, F1, a, b, c, d, e, f, g, h, w0_t, k_sha256[i +  0]); \
    SHA256_STEP(F0, F1, h, a, b, c, d, e, f, g, w1_t, k_sha256[i +  1]); \
    SHA256_STEP(F0, F1, g, h, a, b, c, d, e, f, w2_t, k_sha256[i +  2]); \
    SHA256_STEP(F0, F1, f, g, h, a, b, c, d, e, w3_t, k_sha256[i +  3]); \
    SHA256_STEP(F0, F1, e, f, g, h, a, b, c, d, w4_t, k_sha256[i +  4]); \
    SHA256_STEP(F0, F1, d, e, f, g, h, a, b, c, w5_t, k_sha256[i +  5]); \
    SHA256_STEP(F0, F1, c, d, e, f, g, h, a, b, w6_t, k_sha256[i +  6]); \
    SHA256_STEP(F0, F1, b, c, d, e, f, g, h, a, w7_t, k_sha256[i +  7]); \
    SHA256_STEP(F0, F1, a, b, c, d, e, f, g, h, w8_t, k_sha256[i +  8]); \
    SHA256_STEP(F0, F1, h, a, b, c, d, e, f, g, w9_t, k_sha256[i +  9]); \
    SHA256_STEP(F0, F1, g, h, a, b, c, d, e, f, wa_t, k_sha256[i + 10]); \
    SHA256_STEP(F0, F1, f, g, h, a, b, c, d, e, wb_t, k_sha256[i + 11]); \
    SHA256_STEP(F0, F1, e, f, g, h, a, b, c, d, wc_t, k_sha256[i + 12]); \
    SHA256_STEP(F0, F1, d, e, f, g, h, a, b, c, wd_t, k_sha256[i + 13]); \
    SHA256_STEP(F0, F1, c, d, e, f, g, h, a, b, we_t, k_sha256[i + 14]); \
    SHA256_STEP(F0, F1, b, c, d, e, f, g, h, a, wf_t, k_sha256[i + 15]); \
  }

  ROUND_STEP(0);

  ROUND_EXPAND();
  ROUND_STEP(16);

  ROUND_EXPAND();
  ROUND_STEP(32);

  ROUND_EXPAND();
  ROUND_STEP(48);

  digest[0] += a;
  digest[1] += b;
  digest[2] += c;
  digest[3] += d;
  digest[4] += e;
  digest[5] += f;
  digest[6] += g;
  digest[7] += h;
}

/**
 * Executes SHA256(SHA256(lhs || rhs))
 */
static void sha256d_cat(global const uint* lhs, global const uint* rhs, global uint* md){
    uint W[16]={0};
    uint state[8];

    // first round
    state[0] = 0x6a09e667;
    state[1] = 0xbb67ae85;
    state[2] = 0x3c6ef372;
    state[3] = 0xa54ff53a;
    state[4] = 0x510e527f;
    state[5] = 0x9b05688c;
    state[6] = 0x1f83d9ab;
    state[7] = 0x5be0cd19;

    // first 512 bit block of first round
    W[0x0] = SWAP(lhs[0x0]);
    W[0x1] = SWAP(lhs[0x1]);
    W[0x2] = SWAP(lhs[0x2]);
    W[0x3] = SWAP(lhs[0x3]);
    W[0x4] = SWAP(lhs[0x4]);
    W[0x5] = SWAP(lhs[0x5]);
    W[0x6] = SWAP(lhs[0x6]);
    W[0x7] = SWAP(lhs[0x7]);
    W[0x8] = SWAP(rhs[0x0]);
    W[0x9] = SWAP(rhs[0x1]);
    W[0xA] = SWAP(rhs[0x2]);
    W[0xB] = SWAP(rhs[0x3]);
    W[0xC] = SWAP(rhs[0x4]);
    W[0xD] = SWAP(rhs[0x5]);
    W[0xE] = SWAP(rhs[0x6]);
    W[0xF] = SWAP(rhs[0x7]);

    sha256_process2(W, state);

    // second 512 block (padding) of first round
    W[0x0] = SWAP(0x80); // leading 1 bit followed by 7 zero bits
    W[0x1] = 0x0;
    W[0x2] = 0x0;
    W[0x3] = 0x0;
    W[0x4] = 0x0;
    W[0x5] = 0x0;
    W[0x6] = 0x0;
    W[0x7] = 0x0;
    W[0x8] = 0x0;
    W[0x9] = 0x0;
    W[0xA] = 0x0;
    W[0xB] = 0x0;
    W[0xC] = 0x0;
    W[0xD] = 0x0;
    W[0xE] = 0x0;
    W[0xF] = 512; // bit-length of original data

    sha256_process2(W, state);

    // second round
    // first 512 block (includes padding) of second round
    W[0x0] = state[0];
    W[0x1] = state[1];
    W[0x2] = state[2];
    W[0x3] = state[3];
    W[0x4] = state[4];
    W[0x5] = state[5];
    W[0x6] = state[6];
    W[0x7] = state[7];
    W[0x8] = SWAP(0x80); // leading 1 bit followed by 7 zero bits
    W[0x9] = 0x0;
    W[0xA] = 0x0;
    W[0xB] = 0x0;
    W[0xC] = 0x0;
    W[0xD] = 0x0;
    W[0xE] = 0x0;
    W[0xF] = 256;

    state[0] = 0x6a09e667;
    state[1] = 0xbb67ae85;
    state[2] = 0x3c6ef372;
    state[3] = 0xa54ff53a;
    state[4] = 0x510e527f;
    state[5] = 0x9b05688c;
    state[6] = 0x1f83d9ab;
    state[7] = 0x5be0cd19;

    sha256_process2(W, state);

    md[0] = SWAP(state[0]);
    md[1] = SWAP(state[1]);
    md[2] = SWAP(state[2]);
    md[3] = SWAP(state[3]);
    md[4] = SWAP(state[4]);
    md[5] = SWAP(state[5]);
    md[6] = SWAP(state[6]);
    md[7] = SWAP(state[7]);
}

#define EQUAL(lhs, rhs) (lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] == rhs[2] && lhs[3] == rhs[3] && lhs[4] == rhs[4] && lhs[5] == rhs[5] && lhs[6] == rhs[6] && lhs[7] == rhs[7])

kernel void merkle(global const uchar* data, const size_t leaves, const size_t round, global int* mutated){
    size_t id = get_global_id(0);
    size_t size = get_global_size(0);

    global const uint* lhs = (global const uint*)&data[SHA256_DIGEST_SIZE * (id << (round + 1))];
    global const uint* rhs = (global const uint*)&data[SHA256_DIGEST_SIZE * ((id << (round + 1)) + (1 << round))];

    if(1+(2*id) == leaves){ // last work item and leaf count odd
        sha256d_cat(lhs, lhs, lhs); // SHA256(SHA256(lhs || lhs))
    } else {
        if(1+id == size && EQUAL(lhs, rhs)){ // last work item and leaf count even
            *mutated = 1; // -> set to mutated if duplicate hash has been found
        }
        sha256d_cat(lhs, rhs, lhs); // SHA256(SHA256(lhs || rhs))
    }
}
