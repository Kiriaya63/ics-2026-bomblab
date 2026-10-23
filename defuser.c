#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t low = (uint32_t)challenge;
    uint32_t high = (uint32_t)(challenge >> 32);
    uint32_t middle = (uint32_t)(challenge >> 16);

    uint32_t t0 = (5u * low + high) ^ 0x5721u;

    uint32_t t1 = (high + 2u * middle) ^ 0x89e7u;

    uint32_t t2 = 4u * t1 + (low ^ 0x1470u);

    values[0] = t2 ^ t0;

    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 6;

    values[0] = (uint32_t)(((challenge >> 4) & 0x7f) ^ 0xa);

    for (uint32_t i = 1; i < 6; ++i) {
        uint32_t offset = (uint32_t)((challenge >> (i + 1)) & 31u);
        values[i] = 3u * (values[i - 1] + i) - offset;
    }

    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;

    uint32_t index = (uint32_t)(challenge & 7u);
    uint32_t high = (uint32_t)(challenge >> 32);
    uint32_t shifted = (uint32_t)(challenge >> 3);
    uint32_t x = high ^ shifted;

    values[0] = index;

    switch (index) {
        case 0:
            values[1] = 5u * x + 0x114a4u;
            break;
        case 1:
            values[1] = (2u * x) ^ 0xbb21u;
            break;
        case 2:
            values[1] = 3u * x + 0xd0d1u;
            break;
        case 3:
            values[1] = (4u * x) ^ 0x989fu;
            break;
        case 4:
            values[1] = 5u * x + 0x178e0u;
            break;
        case 5:
            values[1] = (x ^ 0x925cu) + (x >> 1);
            break;
        case 6:
            values[1] = (x ^ 0x41fbu) + (x >> 1);
            break;
        case 7:
            values[1] = 3u * x + 0xc873u;
            break;
    }

    return write_answer(out, cap, values, count);
}

static uint32_t p4_walk_solver(uint32_t target, uint32_t low,
                               uint32_t high, uint32_t salt) {
    uint32_t mid = low + ((high - low) >> 1);

    if (target == mid) {
        return (mid + 30u) ^ salt;
    }

    if (target < mid) {
        uint32_t result = p4_walk_solver(target, low, mid - 1, salt);
        return (5u * result) ^ (mid + salt) ^ 0x12u;
    }

    uint32_t result = p4_walk_solver(target, mid + 1, high, salt);
    return (5u * result) ^ (mid + salt) ^ 0x1fu;
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t target = (uint32_t)(challenge & 0x7fu);
    uint32_t salt = (uint32_t)(challenge >> 7)
                  ^ (uint32_t)(challenge >> 32);

    values[0] = p4_walk_solver(target, 0, 127, salt);

    return write_answer(out, cap, values, count);
}

typedef struct {
    uint32_t value;
    uint32_t bonus;
    int next;
} P5Node;

static const P5Node p5_nodes[12] = {
    {0x760a, 0x14d, 4},
    {0xe8a4, 0x17, 7},
    {0xb493, 0x582, 6},
    {0xeed9, 0x85f, 0},
    {0x5a0d, 0x926, 9},
    {0x5aa6, 0x43d, 1},
    {0xb5bc, 0x730, 5},
    {0x58dc, 0x340, 8},
    {0xeee3, 0x686, 3},
    {0x53d2, 0xa42, 10},
    {0x32d3, 0x27e, -1},
    {0xbfd6, 0xb5a, 2}
};

static const int p5_heads[8] = {
    11, 2, 6, 5, 1, 7, 8, 3
};

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;

    uint32_t index = (uint32_t)(challenge & 7u);
    uint32_t steps = (uint32_t)((challenge >> 3) & 3u);

    int node = p5_heads[index];

    uint32_t result = (uint32_t)(challenge >> 6)
                    ^ (uint32_t)(challenge >> 32)
                    ^ p5_nodes[node].value
                    ^ 0x729fu;

    result += p5_nodes[node].bonus;

    for (uint32_t i = 0; i < steps; ++i) {
        node = p5_nodes[node].next;
        result ^= p5_nodes[node].value;
        result += p5_nodes[node].bonus;
    }

    values[0] = result;
    values[1] = p5_nodes[node].bonus;

    return write_answer(out, cap, values, count);
}


static uint32_t p6_op0(uint32_t x) {
    return (x >> 5) | (x << 27);
}

static uint32_t p6_op1(uint32_t x) {
    return (x + 0xb5ceu) ^ (x >> 7);
}

static uint32_t p6_op2(uint32_t x) {
    return (x ^ 0x848fu) + 2u * x;
}

static uint32_t p6_op3(uint32_t x) {
    return ((x << 3) | (x >> 29)) ^ 0x76bdu;
}

static uint32_t p6_op4(uint32_t x) {
    return ((x >> 1) ^ x) + 0xa608u;
}

static uint32_t p6_op5(uint32_t x) {
    return (2u * x + 0x998u) ^ (5u * x);
}

typedef uint32_t (*P6Operation)(uint32_t);

static const P6Operation p6_ops[8] = {
    p6_op0,
    p6_op1,
    p6_op3,
    p6_op3,
    p6_op2,
    p6_op2,
    p6_op4,
    p6_op5
};

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t x = (uint32_t)(challenge >> 6)
               ^ (uint32_t)(challenge >> 32);

    uint32_t i = (uint32_t)(challenge & 7u);
    uint32_t j = (uint32_t)((challenge >> 3) & 7u);

    uint32_t first = p6_ops[i](x);
    uint32_t second = p6_ops[j](first ^ 0x810cu);

    values[0] = second + 0x669au;

    return write_answer(out, cap, values, count);
}



static uint32_t secret_mix_solver(uint32_t x) {
    uint32_t rotated = (x << 3) | (x >> 29);
    return (rotated + 0x260au) ^ 0xf938u;
}

int solve_secret(uint64_t challenge, uint64_t context,
                 char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 3;

    uint32_t a = (uint32_t)(context & 3u);

    uint32_t u = (uint32_t)(challenge >> 32)
               ^ (uint32_t)challenge;

    uint32_t v = (uint32_t)(context >> 32)
               ^ (uint32_t)context;

    uint32_t r = (a * 0x260au)
               ^ (uint32_t)(context >> 16);

    uint32_t b = secret_mix_solver(u ^ v) ^ r;

    uint32_t c = b ^ (v + a + 0xf938u + (u >> 3));

    values[0] = a;
    values[1] = b;
    values[2] = c;

    if (cap <= 36) {
        return 1;
    }

    memset(out, 'A', 32);
    memcpy(out + 32, "OPEN", 4);

    return write_answer(out + 36, cap - 36, values, count);
}

