# Sensor Benchmark

Dự án minh họa và benchmark các cấu trúc dữ liệu dùng để xử lý bài toán **Dynamic Range Query** trên dữ liệu số bước chân.

Chương trình được viết bằng C11 và hỗ trợ:

- Range Sum.
- Range Minimum.
- Range Maximum.
- Point Update.
- Kiểm thử tính đúng đắn.
- So sánh kết quả giữa các cấu trúc dữ liệu.
- Benchmark thời gian thực thi và bộ nhớ sử dụng.
- Demo tự động và demo tương tác.

## 1. Bài toán

Mỗi phần tử trong mảng đại diện cho số bước chân của một ngày.

Dữ liệu mẫu:

```text
Ngày:      1     2     3     4     5      6      7
Số bước:  6000  8500  7200  9000  5500   12000  10000


Các thao tác mẫu:

SUM 2 6
MIN 1 7
MAX 3 7
UPDATE 3 10000


Ý nghĩa:

SUM 2 6: Tính tổng số bước từ ngày 2 đến ngày 6.
MIN 1 7: Tìm số bước nhỏ nhất từ ngày 1 đến ngày 7.
MAX 3 7: Tìm số bước lớn nhất từ ngày 3 đến ngày 7.
UPDATE 3 10000: Cập nhật số bước của ngày 3 thành 10000.
2. Các cấu trúc dữ liệu
Array

Hỗ trợ:

Range Sum.
Range Minimum.
Range Maximum.
Point Update.

Độ phức tạp:

Build:  O(n)
Query:  O(n)
Update: O(1)
Space:  O(n)


Array là giải pháp đơn giản nhất và được sử dụng làm kết quả chuẩn để kiểm tra các cấu trúc dữ liệu khác.

Prefix Sum

Hỗ trợ:

Range Sum.
Point Update.

Độ phức tạp:

Build:  O(n)
Query:  O(1)
Update: O(n)
Space:  O(n)


Prefix Sum phù hợp với dữ liệu ít thay đổi nhưng có nhiều truy vấn tính tổng.

Binary Indexed Tree

Binary Indexed Tree còn được gọi là Fenwick Tree.

Hỗ trợ:

Range Sum.
Point Update.

Độ phức tạp:

Build:  O(n log n)
Query:  O(log n)
Update: O(log n)
Space:  O(n)


BIT phù hợp với dữ liệu động khi chỉ cần tính tổng và cập nhật một phần tử.

Segment Tree

Hỗ trợ:

Range Sum.
Range Minimum.
Range Maximum.
Point Update.

Độ phức tạp:

Build:  O(n)
Query:  O(log n)
Update: O(log n)
Space:  O(n)


Segment Tree linh hoạt hơn BIT nhưng cần nhiều bộ nhớ và có phần hiện thực phức tạp hơn.

3. Quy ước indexing

Chương trình sử dụng hai quy ước:

Người dùng nhập ngày theo indexing 1-based.
Các hàm và cấu trúc dữ liệu bên trong sử dụng indexing 0-based.
Mảng nội bộ của Binary Indexed Tree sử dụng indexing 1-based.

Ví dụ:

Ngày người dùng nhập:  1  2  3  4  5  6  7
Index trong Array:     0  1  2  3  4  5  6


Khoảng ngày [2, 6] tương ứng với khoảng index [1, 5].

4. Cấu trúc thư mục
SensorBenchmark/
├── CMakeLists.txt
├── README.md
├── include/
│   ├── common.h
│   ├── raw_array.h
│   ├── prefix_sum.h
│   ├── fenwick_tree.h
│   ├── segment_tree.h
│   ├── workload.h
│   ├── benchmark.h
│   ├── test.h
│   └── demo.h
├── src/
│   ├── main.c
│   ├── raw_array.c
│   ├── prefix_sum.c
│   ├── fenwick_tree.c
│   ├── segment_tree.c
│   ├── workload.c
│   ├── benchmark.c
│   ├── test.c
│   └── demo.c
├── data/
│   └── sample_steps.csv
├── results/
│   ├── benchmark_results.csv
│   └── benchmark_environment.txt
├── tests/
│   └── test_cases.txt
└── backup/
    ├── screenshots/
    └── videos/

5. Vai trò của từng thành phần
include/

Chứa khai báo hàm, cấu trúc dữ liệu, kiểu dữ liệu và hằng số dùng chung.

src/

Chứa phần hiện thực của các thuật toán, test runner, benchmark runner và chương trình demo.

data/

Chứa dữ liệu số bước mẫu.

results/

Chứa kết quả benchmark và thông tin môi trường chạy.

tests/

Chứa danh sách các test case dự kiến.

backup/

Chứa ảnh chụp và video demo dự phòng cho buổi thuyết trình.

6. Yêu cầu môi trường
CMake 3.10 trở lên.
Compiler hỗ trợ C11.
GCC, Clang hoặc MSVC.
Ninja, MinGW Makefiles hoặc Visual Studio generator.
Windows PowerShell nếu chạy theo các lệnh trong tài liệu này.

Môi trường đã được sử dụng để kiểm tra dự án:

Operating system: Windows
Compiler: MinGW GCC
Build system: CMake + Ninja
C standard: C11

7. Build dự án

Mở PowerShell tại thư mục gốc:

PS C:\Users\BaoLH19\Desktop\CP\SensorBenchmark>

Build Debug
cmake -S . -B build
cmake --build build


Chạy chương trình:

.\build\sensor_benchmark.exe

Build Release

Khuyến nghị sử dụng Release khi chạy benchmark:

cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release


Chạy chương trình:

.\build-release\sensor_benchmark.exe

Build lại từ đầu

Nếu thay đổi compiler, generator hoặc cấu hình quan trọng, hãy xóa thư mục build cũ:

Remove-Item -Recurse -Force .\build-release
cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release

8. Menu chính

Sau khi chạy, chương trình hiển thị:

Dynamic Range Query Sensor Benchmark
====================================

Sensor Benchmark
================
1. Run automatic demo
2. Run interactive demo
3. Run test suite
4. Run standard benchmark
5. Exit
Select command:

Lựa chọn 1: Automatic Demo

Chạy tự động kịch bản:

Khởi tạo dữ liệu mẫu.
Chạy SUM 2 6.
Chạy MIN 1 7.
Chạy MAX 3 7.
Chạy UPDATE 3 10000.
Hiển thị dữ liệu sau cập nhật.
Chạy lại các truy vấn.
Kiểm tra kết quả giữa các cấu trúc.
Lựa chọn 2: Interactive Demo

Cho phép người dùng lựa chọn trực tiếp:

1. Show step data
2. Range Sum
3. Range Minimum
4. Range Maximum
5. Update step count
6. Validate all structures
7. Run benchmark
8. Reset sample data
9. Exit

Lựa chọn 3: Test Suite

Chạy toàn bộ unit test và cross-validation test.

Các nhóm test gồm:

Array tests.
Prefix Sum tests.
Binary Indexed Tree tests.
Segment Tree tests.
Cross-validation tests.

Kết quả đúng cần hiển thị:

Test summary
============
Total:  48
Passed: 48
Failed: 0

All 48 tests completed successfully.

Lựa chọn 4: Standard Benchmark

Chạy benchmark tiêu chuẩn cho tất cả cấu trúc dữ liệu.

Lựa chọn 5: Exit

Giải phóng tài nguyên và kết thúc chương trình.

9. Kịch bản demo tự động

Dữ liệu ban đầu:

6000 8500 7200 9000 5500 12000 10000


Kết quả trước Update:

SUM 2 6 = 42200
MIN 1 7 = 5500
MAX 3 7 = 12000


Thực hiện:

UPDATE 3 10000


Dữ liệu sau Update:

6000 8500 10000 9000 5500 12000 10000


Kết quả sau Update:

SUM 2 6 = 45000
MIN 1 7 = 5500
MAX 3 7 = 12000

10. Kiểm thử

Array được sử dụng làm kết quả chuẩn.

Chương trình kiểm tra:

Kết quả Range Sum của Array, Prefix Sum, BIT và Segment Tree.
Kết quả Range Min của Array và Segment Tree.
Kết quả Range Max của Array và Segment Tree.
Kết quả trước và sau nhiều Point Update.
Query một phần tử.
Query toàn bộ dữ liệu.
Khoảng có left > right.
Index nằm ngoài phạm vi.
Con trỏ NULL.
Giải phóng bộ nhớ.
Khả năng tái lập dataset và workload.
11. Khả năng tái lập workload

Dataset và workload được sinh bằng random seed cố định:

Random seed: 2026


Cấu hình mặc định:

Query:  70%
Update: 30%


Với N = 1000:

Operations: 1000
Queries:    700
Updates:    300


Hai dataset hoặc workload được sinh với cùng:

Seed.
Dataset size.
Operation count.
Khoảng giá trị.

sẽ có nội dung giống nhau.

12. Cấu hình benchmark

Dataset sizes:

N = 1.000
N = 10.000
N = 100.000


Số thao tác:

Q = N


Tỷ lệ workload:

70% Query
30% Update


Số lần chạy mặc định:

5 lần cho mỗi thuật toán


Khoảng số bước ngẫu nhiên:

1000 đến 20000

13. Metric benchmark

Chương trình thu thập:

Build Time.
Query Time.
Update Time.
Total Execution Time.
Memory Usage.
Checksum.

Đơn vị:

Time:   milliseconds
Memory: bytes, giá trị ước lượng


Thời gian in dữ liệu bằng printf không được tính vào thời gian thực thi thuật toán.

14. Quy tắc so sánh công bằng

Benchmark sử dụng:

Cùng dataset ban đầu.
Cùng workload.
Cùng random seed.
Cùng số thao tác.
Cùng số lần chạy.
Cùng compiler và optimization level.

Phạm vi chức năng:

Array:
SUM, MIN, MAX, UPDATE

Prefix Sum:
SUM, UPDATE

Binary Indexed Tree:
SUM, UPDATE

Segment Tree:
SUM, MIN, MAX, UPDATE


Vì Prefix Sum và BIT không xử lý trực tiếp MIN và MAX, checksum được so sánh theo hai nhóm:

Array = Segment Tree
Prefix Sum = Binary Indexed Tree

15. File kết quả benchmark
results/benchmark_results.csv

Chứa các cột:

algorithm
dataset_size
operation_count
query_count
update_count
run_count
build_time_ms
query_time_ms
update_time_ms
total_execution_time_ms
memory_usage_bytes
checksum


Xem kết quả bằng PowerShell:

Get-Content .\results\benchmark_results.csv

results/benchmark_environment.txt

Chứa thông tin:

Compiler.
Chuẩn C.
Build mode.
Kích thước các kiểu dữ liệu.
Dataset size.
Operation count.
Benchmark runs.
Random seed.
Query và Update percentage.
Đơn vị đo thời gian.
Cách ước lượng bộ nhớ.

Xem file bằng PowerShell:

Get-Content .\results\benchmark_environment.txt

16. Kết quả kỳ vọng

Khi dữ liệu tăng:

Array chậm dần vì mỗi Query cần duyệt một khoảng.
Prefix Sum xử lý Range Sum rất nhanh nhưng Update chậm.
BIT cân bằng tốt giữa Range Sum và Update.
Segment Tree linh hoạt với Sum, Min và Max nhưng dùng nhiều bộ nhớ hơn.
Segment Tree thường có constant factor lớn hơn BIT.

Ma trận lựa chọn:

Dữ liệu nhỏ, ít Query:
Array

Dữ liệu tĩnh, nhiều Range Sum:
Prefix Sum

Dữ liệu động, cần Sum:
Binary Indexed Tree

Dữ liệu động, cần Sum, Min và Max:
Segment Tree

17. Lưu ý khi benchmark

Để có số liệu ổn định hơn:

Build bằng Release.
Đóng các ứng dụng nặng không cần thiết.
Cắm nguồn nếu sử dụng laptop.
Không chạy benchmark trong khi hệ thống đang cập nhật.
Chạy nhiều lần.
Không sử dụng Debug build để lấy số liệu cuối.
Ghi lại compiler, CPU và optimization level.
Kiểm tra checksum trước khi phân tích hiệu năng.

Lệnh khuyến nghị:

cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
.\build-release\sensor_benchmark.exe


Sau đó chọn:

4. Run standard benchmark

18. Phạm vi dự án

Dự án tập trung vào:

Point Update.
Range Sum.
Range Minimum.
Range Maximum.
Benchmark các cấu trúc dữ liệu.
Demo bằng giao diện dòng lệnh.

Không nằm trong phạm vi chính:

Range Update.
Lazy Propagation.
Persistent Segment Tree.
Dữ liệu nhiều người dùng.
Đọc workload lớn từ hệ thống thực tế.

Lazy Propagation chỉ được nhắc đến như một hướng mở rộng.

19. Xử lý lỗi thường gặp
CMake không tìm thấy compiler

Kiểm tra:

gcc --version
cmake --version
ninja --version

Build không nhận thay đổi mới

Xóa thư mục build rồi cấu hình lại:

Remove-Item -Recurse -Force .\build-release
cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release

Không ghi được file kết quả

Đảm bảo chương trình được chạy từ thư mục gốc:

PS C:\Users\BaoLH19\Desktop\CP\SensorBenchmark>


Không nên chạy executable khi terminal đang đứng bên trong thư mục build-release, vì đường dẫn tương đối results/ có thể không tồn tại tại đó.

Benchmark chạy chậm

Prefix Sum và Array có thể chậm rõ rệt ở dataset lớn vì:

Array Query:       O(n)
Prefix Sum Update: O(n)


Đây là kết quả dự kiến, không nhất thiết là lỗi chương trình.

20. Trạng thái dự án

Các phase đã hoàn thành:

Phase 1: Nền tảng và cấu hình dự án
Phase 2: Array
Phase 3: Prefix Sum
Phase 4: Binary Indexed Tree
Phase 5: Segment Tree
Phase 6: Test suite
Phase 7: Dataset và workload
Phase 8: Benchmark runner
Phase 9: Demo tổng thể
Phase 11: Hoàn thiện và tích hợp


Phase có thể hoàn thiện sau:

Phase 10: Ảnh chụp và video demo dự phòng