#ifndef SENSOR_BENCHMARK_COMMON_H
#define SENSOR_BENCHMARK_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Kiểu dữ liệu lưu số bước của một ngày.
 *
 * uint64_t được sử dụng để hỗ trợ:
 * - Giá trị số bước lớn.
 * - Tổng số bước của một khoảng dài.
 * - Kết quả checksum trong benchmark.
 */
typedef uint64_t StepCount;

/*
 * Kiểu dữ liệu biểu diễn index và kích thước dữ liệu.
 */
typedef size_t DataIndex;

/*
 * Kích thước dữ liệu mẫu dùng trong phần demo.
 */
#define SAMPLE_DATA_SIZE ((DataIndex)7U)

/*
 * Random seed mặc định dùng để tái lập workload benchmark.
 */
#define DEFAULT_RANDOM_SEED UINT32_C(2026)

/*
 * Tỷ lệ Query và Update trong workload benchmark.
 */
#define QUERY_PERCENTAGE 70U
#define UPDATE_PERCENTAGE 30U

/*
 * Các kích thước dataset benchmark thống nhất.
 */
#define SMALL_DATASET_SIZE ((DataIndex)1000U)
#define MEDIUM_DATASET_SIZE ((DataIndex)10000U)
#define LARGE_DATASET_SIZE ((DataIndex)100000U)

/*
 * Loại thao tác mà chương trình hỗ trợ.
 */
typedef enum
{
    OPERATION_SUM = 0,
    OPERATION_MIN,
    OPERATION_MAX,
    OPERATION_UPDATE
} OperationType;

/*
 * Trạng thái trả về chung của các hàm.
 */
typedef enum
{
    STATUS_SUCCESS = 0,
    STATUS_INVALID_ARGUMENT,
    STATUS_INDEX_OUT_OF_RANGE,
    STATUS_ALLOCATION_FAILED,
    STATUS_FILE_ERROR
} StatusCode;

/*
 * Biểu diễn một thao tác Query hoặc Update.
 *
 * Với SUM, MIN và MAX:
 * - left và right là hai đầu của khoảng.
 * - value không được sử dụng.
 *
 * Với UPDATE:
 * - left là index cần cập nhật.
 * - right không được sử dụng.
 * - value là số bước mới.
 */
typedef struct
{
    OperationType type;
    DataIndex left;
    DataIndex right;
    StepCount value;
} Operation;

/*
 * Kiểm tra một index có nằm trong phạm vi dữ liệu hay không.
 */
static inline bool isValidIndex(DataIndex index, DataIndex size)
{
    return index < size;
}

/*
 * Kiểm tra khoảng [left, right] theo indexing 0-based.
 */
static inline bool isValidRange(
    DataIndex left,
    DataIndex right,
    DataIndex size
)
{
    return size > 0U && left <= right && right < size;
}

#endif