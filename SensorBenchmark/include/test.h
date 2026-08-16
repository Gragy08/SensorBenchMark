#ifndef SENSOR_BENCHMARK_TEST_H
#define SENSOR_BENCHMARK_TEST_H

#include "common.h"

/*
 * Thống kê kết quả của một nhóm test.
 */
typedef struct
{
    DataIndex total;
    DataIndex passed;
    DataIndex failed;
} TestSummary;

/*
 * Chạy test cho giải pháp Array.
 *
 * Kiểm tra:
 * - Range Sum.
 * - Range Minimum.
 * - Range Maximum.
 * - Point Update.
 * - Các khoảng biên.
 * - Input không hợp lệ.
 */
TestSummary runArrayTests(void);

/*
 * Chạy test cho Prefix Sum.
 *
 * Kiểm tra:
 * - Build Prefix Sum.
 * - Range Sum.
 * - Point Update.
 * - Truy vấn trước và sau Update.
 * - Input không hợp lệ.
 */
TestSummary runPrefixSumTests(void);

/*
 * Chạy test cho Binary Indexed Tree.
 *
 * Kiểm tra:
 * - Build BIT.
 * - Prefix Sum.
 * - Range Sum.
 * - Point Update.
 * - Kết quả trước và sau Update.
 * - Input không hợp lệ.
 */
TestSummary runFenwickTreeTests(void);

/*
 * Chạy test cho Segment Tree.
 *
 * Kiểm tra:
 * - Build Segment Tree.
 * - Range Sum.
 * - Range Minimum.
 * - Range Maximum.
 * - Point Update.
 * - Kết quả trước và sau Update.
 * - Input không hợp lệ.
 */
TestSummary runSegmentTreeTests(void);

/*
 * So sánh kết quả của các cấu trúc dữ liệu với Array.
 *
 * Array được sử dụng làm kết quả chuẩn.
 *
 * Kiểm tra:
 * - Array với Prefix Sum cho Range Sum.
 * - Array với BIT cho Range Sum.
 * - Array với Segment Tree cho Sum, Min và Max.
 * - Kết quả trước và sau nhiều Point Update.
 */
TestSummary runCrossValidationTests(void);

/*
 * Chạy toàn bộ test của dự án.
 *
 * Giá trị trả về:
 * - STATUS_SUCCESS nếu tất cả test đều thành công.
 * - STATUS_TEST_FAILED nếu có ít nhất một test thất bại.
 */
StatusCode runAllTests(TestSummary *summary);

#endif