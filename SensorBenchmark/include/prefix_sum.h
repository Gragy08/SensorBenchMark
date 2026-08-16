#ifndef SENSOR_BENCHMARK_PREFIX_SUM_H
#define SENSOR_BENCHMARK_PREFIX_SUM_H

#include "common.h"

/*
 * Xây dựng mảng Prefix Sum từ mảng dữ liệu ban đầu.
 *
 * prefix[i] lưu tổng các phần tử trong khoảng [0, i].
 *
 * Độ phức tạp:
 * - Time:  O(n)
 * - Space: O(n)
 */
StatusCode buildPrefixSum(
    const StepCount data[],
    DataIndex size,
    StepCount prefix[]
);

/*
 * T[ính tổng các phần tử trong khoảng left, right].
 *
 * Công thức:
 * - Nếu left == 0:
 *     sum = prefix[right]
 * - Ngược lại:
 *    [right Ngược lại:
 sum = prefix[right] - prefixộ phức tạp:
 * - Time:  O(1)
 * - Space: O(1)
 */
StatusCode prefixRangeSum(
    const StepCount prefix[],
    DataIndex size,
    DataIndex left,
    DataIndex right,
    StepCount *result
);

/*
 * Cập nhật giá trị tại một index và điều chỉnh mảng Prefix Sum.
 *
 * Hàm cập nhật cả:
 * - Mảng dữ liệu gốc.
 * - Mảng Prefix Sum từ index đến cuối mảng.
 *
 * Độ phức tạp:
 * - Time:  O(n)
 * - Space: O(1)
 */
StatusCode prefixUpdate(
    StepCount data[],
    StepCount prefix[],
    DataIndex size,
    DataIndex index,
    StepCount newValue
);

#endif