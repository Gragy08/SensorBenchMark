#ifndef SENSOR_BENCHMARK_RAW_ARRAY_H
#define SENSOR_BENCHMARK_RAW_ARRAY_H

#include "common.h"

/*
 * Tính tổng các phần tử trong khoảng [left, right].
 *
 * Độ phức tạp:
 * - Time:  O(n)
 * - Space: O(1)
 */
StatusCode arrayRangeSum(
    const StepCount data[],
    DataIndex size,
    DataIndex left,
    DataIndex right,
    StepCount *result
);

/*
 * Tìm giá trị nhỏ nhất trong khoảng [left, right].
 *
 * Độ phức tạp:
 * - Time:  O(n)
 * - Space: O(1)
 */
StatusCode arrayRangeMin(
    const StepCount data[],
    DataIndex size,
    DataIndex left,
    DataIndex right,
    StepCount *result
);

/*
 * Tìm giá trị lớn nhất trong khoảng [left, right].
 *
 * Độ phức tạp:
 * - Time:  O(n)
 * - Space: O(1)
 */
StatusCode arrayRangeMax(
    const StepCount data[],
    DataIndex size,
    DataIndex left,
    DataIndex right,
    StepCount *result
);

/*
 * Cập nhật giá trị tại một index.
 *
 * Độ phức tạp:
 * - Time:  O(1)
 * - Space: O(1)
 */
StatusCode arrayUpdate(
    StepCount data[],
    DataIndex size,
    DataIndex index,
    StepCount newValue
);

#endif