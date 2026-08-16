#ifndef SENSOR_BENCHMARK_BENCHMARK_H
#define SENSOR_BENCHMARK_BENCHMARK_H

#include "common.h"
#include "workload.h"

/*
 * Số lần chạy benchmark mặc định.
 *
 * Kết quả cuối cùng sẽ sử dụng giá trị trung bình
 * của các lần chạy hợp lệ.
 */
#define DEFAULT_BENCHMARK_RUNS ((DataIndex)5U)

/*
 * Tên file kết quả benchmark mặc định.
 */
#define DEFAULT_BENCHMARK_RESULT_FILE \
    "results/benchmark_results.csv"

/*
 * Tên file mô tả môi trường benchmark mặc định.
 */
#define DEFAULT_BENCHMARK_ENVIRONMENT_FILE \
    "results/benchmark_environment.txt"

/*
 * Các giải pháp được benchmark.
 */
typedef enum
{
    BENCHMARK_ARRAY = 0,
    BENCHMARK_PREFIX_SUM,
    BENCHMARK_FENWICK_TREE,
    BENCHMARK_SEGMENT_TREE,
    BENCHMARK_ALGORITHM_COUNT
} BenchmarkAlgorithm;

/*
 * Cấu hình cho một phiên benchmark.
 *
 * datasetSize:
 * - Số phần tử trong dataset.
 *
 * operationCount:
 * - Tổng số thao tác Query và Update.
 *
 * runCount:
 * - Số lần lặp lại benchmark.
 *
 * seed:
 * - Random seed dùng để sinh dataset và workload.
 *
 * minimumStepCount, maximumStepCount:
 * - Khoảng giá trị của dữ liệu số bước.
 */
typedef struct
{
    DataIndex datasetSize;
    DataIndex operationCount;
    DataIndex runCount;
    uint32_t seed;
    StepCount minimumStepCount;
    StepCount maximumStepCount;
} BenchmarkConfig;

/*
 * Kết quả benchmark của một thuật toán.
 *
 * Các giá trị thời gian sử dụng đơn vị millisecond.
 *
 * buildTimeMs:
 * - Thời gian xây dựng cấu trúc dữ liệu.
 *
 * queryTimeMs:
 * - Tổng thời gian xử lý các thao tác Query.
 *
 * updateTimeMs:
 * - Tổng thời gian xử lý các thao tác Update.
 *
 * totalExecutionTimeMs:
 * - Tổng thời gian Build, Query và Update.
 *
 * memoryUsageBytes:
 * - Bộ nhớ ước lượng được sử dụng bởi cấu trúc dữ liệu.
 *
 * checksum:
 * - Giá trị tổng hợp từ kết quả Query.
 * - Dùng để kiểm tra các thuật toán có xử lý cùng workload
 *   và trả về kết quả nhất quán hay không.
 */
typedef struct
{
    BenchmarkAlgorithm algorithm;
    DataIndex datasetSize;
    DataIndex operationCount;
    DataIndex queryCount;
    DataIndex updateCount;
    DataIndex runCount;
    double buildTimeMs;
    double queryTimeMs;
    double updateTimeMs;
    double totalExecutionTimeMs;
    size_t memoryUsageBytes;
    StepCount checksum;
} BenchmarkResult;

/*
 * Trả về tên hiển thị của thuật toán benchmark.
 */
const char *benchmarkAlgorithmName(
    BenchmarkAlgorithm algorithm
);

/*
 * Tạo cấu hình benchmark mặc định cho một kích thước dataset.
 *
 * Cấu hình mặc định:
 * - operationCount = datasetSize.
 * - runCount = DEFAULT_BENCHMARK_RUNS.
 * - seed = DEFAULT_RANDOM_SEED.
 * - Giá trị số bước nằm trong khoảng mặc định.
 */
BenchmarkConfig createDefaultBenchmarkConfig(
    DataIndex datasetSize
);

/*
 * Kiểm tra cấu hình benchmark có hợp lệ hay không.
 */
StatusCode validateBenchmarkConfig(
    const BenchmarkConfig *config
);

/*
 * Đo benchmark Array trên dataset và workload đã cho.
 *
 * Array hỗ trợ:
 * - Range Sum.
 * - Range Minimum.
 * - Range Maximum.
 * - Point Update.
 */
StatusCode benchmarkArray(
    const StepCount initialData[],
    const Workload *workload,
    const BenchmarkConfig *config,
    BenchmarkResult *result
);

/*
 * Đo benchmark Prefix Sum trên dataset và workload đã cho.
 *
 * Prefix Sum chỉ benchmark OPERATION_SUM và OPERATION_UPDATE.
 * Các Query MIN và MAX không được xử lý bởi Prefix Sum.
 */
StatusCode benchmarkPrefixSum(
    const StepCount initialData[],
    const Workload *workload,
    const BenchmarkConfig *config,
    BenchmarkResult *result
);

/*
 * Đo benchmark Binary Indexed Tree trên dataset và workload đã cho.
 *
 * BIT chỉ benchmark OPERATION_SUM và OPERATION_UPDATE.
 * Các Query MIN và MAX không được xử lý bởi BIT.
 */
StatusCode benchmarkFenwickTree(
    const StepCount initialData[],
    const Workload *workload,
    const BenchmarkConfig *config,
    BenchmarkResult *result
);

/*
 * Đo benchmark Segment Tree trên dataset và workload đã cho.
 *
 * Segment Tree hỗ trợ:
 * - Range Sum.
 * - Range Minimum.
 * - Range Maximum.
 * - Point Update.
 */
StatusCode benchmarkSegmentTree(
    const StepCount initialData[],
    const Workload *workload,
    const BenchmarkConfig *config,
    BenchmarkResult *result
);

/*
 * Chạy benchmark cho tất cả các thuật toán.
 *
 * Hàm tự thực hiện:
 * - Sinh dataset bằng seed cố định.
 * - Sinh workload chung.
 * - Chạy benchmark nhiều lần.
 * - Tính kết quả trung bình.
 * - Kiểm tra checksum.
 *
 * results phải có ít nhất BENCHMARK_ALGORITHM_COUNT phần tử.
 */
StatusCode runAllBenchmarks(
    const BenchmarkConfig *config,
    BenchmarkResult results[BENCHMARK_ALGORITHM_COUNT]
);

/*
 * Chạy bộ benchmark chuẩn với các kích thước:
 * - N = 1.000
 * - N = 10.000
 * - N = 100.000
 *
 * results phải có khả năng chứa:
 * 3 * BENCHMARK_ALGORITHM_COUNT phần tử.
 *
 * resultCount trả về số kết quả thực tế đã được ghi.
 */
StatusCode runStandardBenchmarkSuite(
    BenchmarkResult results[],
    DataIndex resultCapacity,
    DataIndex *resultCount
);

/*
 * In một kết quả benchmark ra màn hình.
 */
void printBenchmarkResult(
    const BenchmarkResult *result
);

/*
 * In bảng kết quả benchmark ra màn hình.
 */
void printBenchmarkTable(
    const BenchmarkResult results[],
    DataIndex resultCount
);

/*
 * Ghi danh sách kết quả benchmark vào file CSV.
 *
 * Nếu append bằng false:
 * - Tạo mới hoặc ghi đè file.
 * - Ghi thêm dòng tiêu đề CSV.
 *
 * Nếu append bằng true:
 * - Ghi thêm kết quả vào cuối file hiện có.
 */
StatusCode writeBenchmarkResultsCsv(
    const char *filePath,
    const BenchmarkResult results[],
    DataIndex resultCount,
    bool append
);

/*
 * Ghi thông tin môi trường benchmark vào file.
 *
 * Thông tin bao gồm:
 * - Compiler.
 * - Chuẩn ngôn ngữ C.
 * - Chế độ build.
 * - Kích thước các kiểu dữ liệu chính.
 * - Random seed.
 * - Số lần chạy.
 */
StatusCode writeBenchmarkEnvironment(
    const char *filePath,
    const BenchmarkConfig *config
);

#endif