#ifndef SENSOR_BENCHMARK_SEGMENT_TREE_H
#define SENSOR_BENCHMARK_SEGMENT_TREE_H

#include "common.h"

/*
 * Giá trị được lưu tại mỗi node của Segment Tree.
 *
 * Mỗi node đại diện cho một đoạn dữ liệu và lưu:
 * - Tổng số bước trong đoạn.
 * - Số bước nhỏ nhất trong đoạn.
 * - Số bước lớn nhất trong đoạn.
 */
typedef struct
{
    StepCount sum;
    StepCount minimum;
    StepCount maximum;
} SegmentNode;

/*
 * Cấu trúc Segment Tree.
 *
 * tree:
 * - Mảng chứa các node của cây.
 * - Thường được cấp phát với kích thước khoảng 4 * size.
 *
 * data:
 * - Bản sao của dữ liệu gốc.
 *
 * size:
 * - Số phần tử trong dữ liệu gốc.
 *
 * capacity:
 * - Số node đã được cấp phát cho mảng tree.
 */
typedef struct
{
    SegmentNode *tree;
    StepCount *data;
    DataIndex size;
    DataIndex capacity;
} SegmentTree;

/*
 * Kết hợp thông tin của hai node con.
 *
 * Node kết quả có:
 * - sum = left.sum + right.sum
 * - minimum = min(left.minimum, right.minimum)
 * - maximum = max(left.maximum, right.maximum)
 *
 * Độ phức tạp:
 * - Time:  O(1)
 * - Space: O(1)
 */
SegmentNode mergeNodes(
    SegmentNode leftNode,
    SegmentNode rightNode
);

/*
 * Khởi tạo Segment Tree từ mảng dữ liệu ban đầu.
 *
 * Hàm cấp phát bộ nhớ cho:
 * - Bản sao của mảng dữ liệu.
 * - Mảng node của Segment Tree.
 *
 * Độ phức tạp:
 * - Time:  O(n)
 * - Space: O(n)
 */
StatusCode buildSegmentTree(
    SegmentTree *segmentTree,
    const StepCount data[],
    DataIndex size
);

/*
 * Giải phóng toàn bộ bộ nhớ của Segment Tree.
 */
void destroySegmentTree(
    SegmentTree *segmentTree
);

/*
 * Truy vấn đồng thời Sum, Min và Max trong khoảng [left, right].
 *
 * left và right sử dụng indexing 0-based.
 * Kết quả được lưu vào result.
 *
 * Độ phức tạp:
 * - Time:  O(log n)
 * - Space: O(log n) do sử dụng đệ quy
 */
StatusCode querySegmentTree(
    const SegmentTree *segmentTree,
    DataIndex left,
    DataIndex right,
    SegmentNode *result
);

/*
 * Cập nhật phần tử tại index thành newValue.
 *
 * Hàm cập nhật:
 * - Dữ liệu gốc tại index.
 * - Node lá tương ứng.
 * - Các node cha trên đường từ lá đến gốc.
 *
 * index sử dụng indexing 0-based.
 *
 * Độ phức tạp:
 * - Time:  O(log n)
 * - Space: O(log n) do sử dụng đệ quy
 */
StatusCode updateSegmentTree(
    SegmentTree *segmentTree,
    DataIndex index,
    StepCount newValue
);

#endif