#ifndef SENSOR_BENCHMARK_DEMO_H
#define SENSOR_BENCHMARK_DEMO_H

#include "common.h"

/*
 * Các lệnh điều khiển chương trình demo.
 */
typedef enum
{
    DEMO_COMMAND_SHOW_DATA = 1,
    DEMO_COMMAND_SUM,
    DEMO_COMMAND_MIN,
    DEMO_COMMAND_MAX,
    DEMO_COMMAND_UPDATE,
    DEMO_COMMAND_VALIDATE,
    DEMO_COMMAND_BENCHMARK,
    DEMO_COMMAND_RESET,
    DEMO_COMMAND_EXIT
} DemoCommand;

/*
 * Hiển thị dữ liệu số bước hiện tại.
 *
 * Ngày được hiển thị theo indexing 1-based để thuận tiện
 * cho người dùng và phần thuyết trình.
 */
void displayStepData(
    const StepCount data[],
    DataIndex size
);

/*
 * Hiển thị danh sách lệnh và chức năng của chương trình demo.
 */
void displayDemoMenu(void);

/*
 * Chạy kịch bản demo tự động:
 *
 * 1. Hiển thị dữ liệu ban đầu.
 * 2. Thực hiện SUM 2 6.
 * 3. Thực hiện MIN 1 7.
 * 4. Thực hiện MAX 3 7.
 * 5. Thực hiện UPDATE 3 10000.
 * 6. Chạy lại các Query.
 * 7. Kiểm tra kết quả giữa các cấu trúc dữ liệu.
 *
 * Benchmark không được chạy tự động trong hàm này để tránh
 * làm gián đoạn phần trình bày.
 */
StatusCode runAutomaticDemo(void);

/*
 * Chạy chương trình demo tương tác.
 *
 * Người dùng có thể chọn:
 * - Hiển thị dữ liệu.
 * - Range Sum.
 * - Range Minimum.
 * - Range Maximum.
 * - Point Update.
 * - Kiểm tra kết quả giữa các giải pháp.
 * - Chạy benchmark.
 * - Reset dữ liệu.
 * - Thoát chương trình.
 */
StatusCode runInteractiveDemo(void);

/*
 * Chạy Range Sum trên các cấu trúc được hỗ trợ và hiển thị:
 * - Array.
 * - Prefix Sum.
 * - Binary Indexed Tree.
 * - Segment Tree.
 *
 * leftDay và rightDay sử dụng indexing 1-based.
 */
StatusCode demoRangeSum(
    DataIndex leftDay,
    DataIndex rightDay
);

/*
 * Chạy Range Minimum trên:
 * - Array.
 * - Segment Tree.
 *
 * leftDay và rightDay sử dụng indexing 1-based.
 */
StatusCode demoRangeMin(
    DataIndex leftDay,
    DataIndex rightDay
);

/*
 * Chạy Range Maximum trên:
 * - Array.
 * - Segment Tree.
 *
 * leftDay và rightDay sử dụng indexing 1-based.
 */
StatusCode demoRangeMax(
    DataIndex leftDay,
    DataIndex rightDay
);

/*
 * Cập nhật số bước tại một ngày trên tất cả cấu trúc dữ liệu.
 *
 * day sử dụng indexing 1-based.
 */
StatusCode demoUpdate(
    DataIndex day,
    StepCount newValue
);

/*
 * Kiểm tra tính nhất quán của tất cả cấu trúc dữ liệu
 * trên dữ liệu hiện tại.
 *
 * Kiểm tra:
 * - Range Sum giữa Array, Prefix Sum, BIT và Segment Tree.
 * - Range Min giữa Array và Segment Tree.
 * - Range Max giữa Array và Segment Tree.
 */
StatusCode validateDemoStructures(void);

/*
 * Reset dữ liệu và tất cả cấu trúc về bộ dữ liệu mẫu:
 *
 * 6000, 8500, 7200, 9000, 5500, 12000, 10000
 */
StatusCode resetDemoData(void);

/*
 * Giải phóng toàn bộ tài nguyên được sử dụng bởi chương trình demo.
 */
void destroyDemo(void);

#endif