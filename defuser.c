#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

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

    uint32_t high = (uint32_t)(challenge >> 32);
    uint32_t middle = (uint32_t)(challenge >> 16);
    uint32_t low = (uint32_t)challenge;

    uint32_t a = (high * UINT32_C(5))
               ^ (middle + UINT32_C(0xe277));

    uint32_t b = (low * UINT32_C(3) + high)
               ^ UINT32_C(0xfbe0);

    values[0] = (b + a * UINT32_C(2)) ^ UINT32_C(0xeffc);

    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 6;

    values[0] = (uint32_t)((challenge >> 36) & UINT64_C(127))
              + UINT32_C(16);

    for (uint32_t i = 1; i < 6; ++i) {
        unsigned shift = 18 + 2 * (i - 1);

        uint32_t part =
            (uint32_t)((challenge >> shift) & UINT64_C(63))
            + UINT32_C(13);

        values[i] = (values[i - 1] * UINT32_C(3) - i) ^ part;
    }

    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;

    uint32_t index = (uint32_t)challenge & UINT32_C(7);
    uint32_t x = (uint32_t)(challenge >> 32)
               ^ (uint32_t)(challenge >> 3);
    uint32_t result = 0;

    switch (index) {
    case 0: /* p3_case_0 */
        result = (x * UINT32_C(2)) ^ UINT32_C(0x7aa3);
        break;
    case 1: /* p3_case_6 */
        result = x * UINT32_C(5) + UINT32_C(0xe00e);
        break;
    case 2: /* p3_case_2 */
        result = x * UINT32_C(5) + UINT32_C(0x1291c);
        break;
    case 3: /* p3_case_3 */
        result = (x ^ UINT32_C(0x16c4)) + (x >> 1);
        break;
    case 4: /* p3_case_4 */
        result = x * UINT32_C(5) + UINT32_C(0x45529);
        break;
    case 5: /* p3_case_7 */
        result = (x * UINT32_C(4)) ^ UINT32_C(0x50d2);
        break;
    case 6: /* p3_case_1 */
        result = x * UINT32_C(3) + UINT32_C(0x6c6e);
        break;
    case 7: /* p3_case_5 */
        result = (x ^ UINT32_C(0x95bd)) + (x >> 2);
        break;
    }

    values[0] = index;
    values[1] = result;

    return write_answer(out, cap, values, count);
}

static uint32_t phase4_walk(uint32_t target, uint32_t low,
                            uint32_t high, uint32_t key) {
    uint32_t mid = low + ((high - low) >> 1);

    if (target == mid) {
        return (mid + UINT32_C(0xd2)) ^ key;
    }

    uint32_t child;

    if (target < mid) {
        child = phase4_walk(target, low, mid - 1, key);
        return (child * UINT32_C(5))
             ^ (mid + key)
             ^ UINT32_C(0x4);
    }

    child = phase4_walk(target, mid + 1, high, key);
    return (child * UINT32_C(5))
         ^ (mid + key)
         ^ UINT32_C(0x12);
}
int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t target = (uint32_t)challenge & UINT32_C(127);
    uint32_t key = (uint32_t)(challenge >> 7)
                 ^ (uint32_t)(challenge >> 32);

    values[0] = phase4_walk(target, 0, 127, key);

    return write_answer(out, cap, values, count);
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;

    /* 按 next 指针连接顺序排列，前八项分别是八个起点。 */
    static const uint32_t node_value[11] = {
        0xbc2f, 0x3e72, 0x1b4f, 0xe9cd,
        0x10b6, 0x3cf9, 0x0ebf, 0x3d7a,
        0x1425, 0xbdf8, 0xa8eb
    };

    static const uint32_t node_tag[11] = {
        0x0885, 0x0441, 0x0974, 0x0370,
        0x0168, 0x0a87, 0x0211, 0x0b19,
        0x075f, 0x0633, 0x050d
    };

    uint32_t start = (uint32_t)(challenge & UINT64_C(7));
    uint32_t visits =
        (uint32_t)((challenge >> 3) & UINT64_C(3)) + 1;

    uint32_t acc = (uint32_t)(challenge >> 6)
                 ^ (uint32_t)(challenge >> 32)
                 ^ UINT32_C(0x8256);

    uint32_t last_tag = 0;

    for (uint32_t i = 0; i < visits; ++i) {
        uint32_t pos = start + i;

        acc = (acc + node_value[pos]) * UINT32_C(5);
        acc ^= node_tag[pos];

        last_tag = node_tag[pos];
    }

    values[0] = acc;
    values[1] = last_tag;

    return write_answer(out, cap, values, count);
}

static uint32_t phase6_apply(uint32_t index, uint32_t x) {
    switch (index) {
    case 0:
    case 3: /* p6_op_4：循环左移 5 位后异或 */
        return ((x << 5) | (x >> 27)) ^ UINT32_C(0xa6b1);

    case 1: /* p6_op_0 */
        return (x * UINT32_C(8) + UINT32_C(0x4abc))
             ^ (x * UINT32_C(3));

    case 2:
    case 4: /* p6_op_5 */
        return (x ^ UINT32_C(0xcbc4)) + x * UINT32_C(8);

    case 5: /* p6_op_3：注意常量为 0x30c7 */
        return ((x >> 2) ^ x) + UINT32_C(0x30c7);

    case 6: /* p6_op_1 */
        return (x + UINT32_C(0x9c54)) ^ (x >> 5);

    case 7: /* p6_op_2：循环右移 2 位 */
        return (x >> 2) | (x << 30);
    }

    return 0;
}
int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t index1 =
        (uint32_t)(challenge & UINT64_C(7));
    uint32_t index2 =
        (uint32_t)((challenge >> 3) & UINT64_C(7));

    uint32_t x = (uint32_t)(challenge >> 6)
               ^ (uint32_t)(challenge >> 32);

    uint32_t first = phase6_apply(index1, x);
    uint32_t second =
        phase6_apply(index2, first ^ UINT32_C(0xf624));

    values[0] = second + UINT32_C(0xa1b3);

    return write_answer(out, cap, values, count);
}

int solve_secret(uint64_t challenge, uint64_t context,
                 char *out, size_t cap) {
    uint32_t a = (uint32_t)(challenge >> 32)
               ^ (uint32_t)challenge;

    uint32_t b = (uint32_t)(context >> 32)
               ^ (uint32_t)context;

    uint32_t v0 = (uint32_t)context & UINT32_C(3);

    uint32_t x = a ^ b;
    uint32_t mixed = ((x >> 1) | (x << 31))
                   ^ UINT32_C(0x1254);

    uint32_t v1 = mixed
                ^ (v0 * UINT32_C(0xacd7))
                ^ (uint32_t)(context >> 16);

    uint32_t v2 = v1
                ^ (b + v0 + UINT32_C(0xbe83) + (a >> 1));

    int size = snprintf(
        out, cap,
        "%.*sOPEN%" PRIu32 " %" PRIu32 " %" PRIu32,
        32, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
        v0, v1, v2
    );

    return size < 0 || (size_t)size >= cap;
}
