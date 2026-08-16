@echo off
setlocal

cd /d "%~dp0"

echo [1/4] Configuring Release build...
cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 goto :error

echo.
echo [2/4] Building project...
cmake --build build-release
if errorlevel 1 goto :error

if not exist "results" mkdir "results"

echo.
echo [3/4] Running tests and standard benchmark...
(
    echo 3
    echo 4
    echo 5
) | ".\build-release\sensor_benchmark.exe"

if errorlevel 1 goto :error

echo.
echo [4/4] Verifying benchmark checksums...
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
"$results = Import-Csv '.\results\benchmark_results.csv';" ^
"$allPassed = $true;" ^
"$results | Group-Object dataset_size | ForEach-Object {" ^
"    $sizeResults = $_.Group;" ^
"    $arrayChecksum = ($sizeResults | Where-Object algorithm -eq 'Array').checksum;" ^
"    $prefixChecksum = ($sizeResults | Where-Object algorithm -eq 'Prefix Sum').checksum;" ^
"    $bitChecksum = ($sizeResults | Where-Object algorithm -eq 'Binary Indexed Tree').checksum;" ^
"    $segmentChecksum = ($sizeResults | Where-Object algorithm -eq 'Segment Tree').checksum;" ^
"    $arraySegmentPassed = $arrayChecksum -eq $segmentChecksum;" ^
"    $prefixBitPassed = $prefixChecksum -eq $bitChecksum;" ^
"    if (-not $arraySegmentPassed -or -not $prefixBitPassed) {" ^
"        $allPassed = $false;" ^
"    };" ^
"    Write-Host '';" ^
"    Write-Host ('Dataset N = ' + $_.Name);" ^
"    Write-Host ('Array = Segment Tree: ' + $arraySegmentPassed);" ^
"    Write-Host ('Prefix Sum = BIT:     ' + $prefixBitPassed);" ^
"};" ^
"Write-Host '';" ^
"if ($allPassed) {" ^
"    Write-Host '[PASSED] All benchmark checksums are consistent.';" ^
"    exit 0;" ^
"} else {" ^
"    Write-Host '[FAILED] Benchmark checksum mismatch detected.';" ^
"    exit 1;" ^
"}"

if errorlevel 1 goto :error

echo.
echo ================================================
echo [PASSED] Build, tests, benchmark and checksum
echo verification completed successfully.
echo ================================================
echo.
pause
exit /b 0

:error
echo.
echo ================================================
echo [FAILED] The process stopped because of an error.
echo ================================================
echo.
pause
exit /b 1