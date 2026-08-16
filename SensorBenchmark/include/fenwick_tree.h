#ifndef SENSOR_BENCHMARK_FENWICK_TREE_H
#define SENSOR_BENCHMARK_FENWICK_TREE_H

#include "common.h"

/*
 * Cấu trúc Binary Indexed Tree, còn gọi là Fenwick Tree.
 *
 * Quy ước indexing:
 * - Mảng dữ liệu bên ngoài sử dụng index 0-based.
 * - Mảng tree nội bộ sử dụng index 1-based.
 */
typedef struct
{
    StepCount *tree;
    StepCount *data;
    DataIndex size;
} FenwickTree;

/*
 * Khởi tạo Fenwick Tree từ mảng dữ liệu ban đầu.
 *
 * Hàm cấp phát bộ nhớ cho:
 * - Mảng dữ liệu nội bộ.
 * - Mảng Fenwick Tree có kích thước size + 1.
 *
 * Độ phức tạp:
 * - Time:  O(n log n)
 * - Space: O(n)
 */
StatusCode bitBuild(
    FenwickTree *fenwickTree,
    const StepCount data[],
    DataIndex size
);

/*
 * Giải phóng toàn bộ bộ nhớ của Fenwick Tree.
 */
void bitDestroy(
    FenwickTree *fenwickTree
);

/*
 * Cộng delta vào phần tử tại index.
 *
 * index sử dụng quy ước 0-based.
 *
 * Độ phức tạp:
 * - Time:  O(log n)
 * - Space: O(1)
 */
StatusCode bitAdd(
    FenwickTree *fenwickTree,
    DataIndex index,
    StepCount delta
);

/*
 * Thay giá trị tại index bằng newValue.
 *
 * Hàm tự tính delta dựa trên giá trị cũ:
 * delta = newValue - oldValue
 *
 * index sử dụng quy ước 0-based.
 *
 * Độ phức tạp:
 * - Time:  O(log n)
 * - Space: O(1)
 */
StatusCode bitUpdate(
    FenwickTree *fenwickTree,
    DataIndex index,
    StepCount newValue
);

/*
 * Tính tổng các phần tử trong khoảng [0, index].
 *
 * index sử dụng quy ước 0-based.
 *
 * Độ phức tạp:
 * - Time:  O(log n)
 * - Space: O(1)
 */
StatusCode bitPrefixSum(
    const FenwickTree *fenwickTree,
    DataIndex index,
    StepCount *result
);

/*
 * Tính tổng các phần tử trong khoảng [left, right].
 *
 * Công thức:
 * - Nếu left == 0:
 *     sum = prefixSum(right)
 * - Ngược lại:
 *     sum = prefixSum(right) - prefixSum(left - 1)
 *
 * left và right sử dụng quy ước 0-based.
 *
 * Độ phức tạp:
 * - Time:  O(log n)
 * - Space: O(1)
 */
StatusCode bitRangeSum(
    const FenwickTree *fenwickTree,
    DataIndex left,
    DataIndex right,
    StepCount *result
);

#endif