#ifndef SENSOR_BENCHMARK_WORKLOAD_H
#define SENSOR_BENCHMARK_WORKLOAD_H

#include "common.h"

/*
 * Khoảng giá trị mặc định của số bước được sinh ngẫu nhiên.
 */
#define DEFAULT_MIN_STEP_COUNT UINT64_C(1000)
#define DEFAULT_MAX_STEP_COUNT UINT64_C(20000)

/*
 * Đại diện cho một workload benchmark.
 *
 * operations:
 * - Mảng chứa các thao tác Query và Update.
 *
 * count:
 * - Tổng số thao tác trong workload.
 *
 * queryCount:
 * - Số thao tác SUM, MIN hoặc MAX.
 *
 * updateCount:
 * - Số thao tác UPDATE.
 *
 * seed:
 * - Random seed dùng để tạo workload.
 */
typedef struct
{
    Operation *operations;
    DataIndex count;
    DataIndex queryCount;
    DataIndex updateCount;
    uint32_t seed;
} Workload;

/*
 * Sinh dataset số bước ngẫu nhiên trong đoạn [minimum, maximum].
 *
 * Cùng seed, kích thước và khoảng giá trị sẽ tạo ra cùng một dataset.
 *
 * Độ phức tạp:
 * - Time:  O(n)
 * - Space: O(1), không tính mảng đầu ra
 */
StatusCode generateDataset(
    StepCount data[],
    DataIndex size,
    StepCount minimum,
    StepCount maximum,
    uint32_t seed
);

/*
 * Khởi tạo workload gồm các thao tác Query và Update.
 *
 * Quy ước:
 * - Query sử dụng khoảng [left, right] hợp lệ.
 * - Update sử dụng left làm index cần cập nhật.
 * - Các index trong workload sử dụng indexing 0-based.
 * - Query và Update được sinh theo tỷ lệ truyền vào.
 * - Tổng queryPercentage và updatePercentage phải bằng 100.
 *
 * Query có thể là:
 * - OPERATION_SUM
 * - OPERATION_MIN
 * - OPERATION_MAX
 *
 * Độ phức tạp:
 * - Time:  O(operationCount)
 * - Space: O(operationCount)
 */
StatusCode generateWorkload(
    Workload *workload,
    DataIndex dataSize,
    DataIndex operationCount,
    unsigned int queryPercentage,
    unsigned int updatePercentage,
    StepCount minimumUpdateValue,
    StepCount maximumUpdateValue,
    uint32_t seed
);

/*
 * Tạo workload benchmark mặc định.
 *
 * Cấu hình mặc định:
 * - Số thao tác bằng kích thước dataset.
 * - 70% Query.
 * - 30% Update.
 * - Giá trị Update nằm trong khoảng số bước mặc định.
 */
StatusCode generateDefaultWorkload(
    Workload *workload,
    DataIndex dataSize,
    uint32_t seed
);

/*
 * Sao chép một workload.
 *
 * Hàm này giúp mỗi cấu trúc dữ liệu sử dụng chính xác cùng một
 * danh sách Query và Update trong quá trình benchmark.
 */
StatusCode cloneWorkload(
    const Workload *source,
    Workload *destination
);

/*
 * Giải phóng bộ nhớ của workload và đưa các trường về trạng thái ban đầu.
 */
void destroyWorkload(
    Workload *workload
);

#endif