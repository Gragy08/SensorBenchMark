#include "benchmark.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

#include "fenwick_tree.h"
#include "prefix_sum.h"
#include "raw_array.h"
#include "segment_tree.h"

static double currentTimeMilliseconds(void)
{
#ifdef _WIN32
    static LARGE_INTEGER frequency;
    static BOOL initialized = FALSE;
    LARGE_INTEGER counter;

    if (!initialized)
    {
        if (!QueryPerformanceFrequency(&frequency))
        {
            return 0.0;
        }

        initialized = TRUE;
    }

    if (!QueryPerformanceCounter(&counter))
    {
        return 0.0;
    }

    return (double)counter.QuadPart * 1000.0
        / (double)frequency.QuadPart;
#else
    struct timespec currentTime;

    if (timespec_get(&currentTime, TIME_UTC) != TIME_UTC)
    {
        return 0.0;
    }

    return (double)currentTime.tv_sec * 1000.0
        + (double)currentTime.tv_nsec / 1000000.0;
#endif
}

static bool isValidWorkload(
    const Workload *workload,
    const BenchmarkConfig *config
)
{
    return workload != NULL
        && workload->operations != NULL
        && workload->count == config->operationCount
        && workload->queryCount + workload->updateCount
            == workload->count;
}

static StatusCode allocateDataCopy(
    const StepCount initialData[],
    DataIndex size,
    StepCount **dataCopy
)
{
    if (
        initialData == NULL
        || size == 0U
        || dataCopy == NULL
    )
    {
        return STATUS_INVALID_ARGUMENT;
    }

    if (size > SIZE_MAX / sizeof(**dataCopy))
    {
        return STATUS_ALLOCATION_FAILED;
    }

    *dataCopy = malloc(size * sizeof(**dataCopy));

    if (*dataCopy == NULL)
    {
        return STATUS_ALLOCATION_FAILED;
    }

    memcpy(
        *dataCopy,
        initialData,
        size * sizeof(**dataCopy)
    );

    return STATUS_SUCCESS;
}

static void initializeBenchmarkResult(
    BenchmarkResult *result,
    BenchmarkAlgorithm algorithm,
    const BenchmarkConfig *config,
    const Workload *workload
)
{
    *result = (BenchmarkResult){
        .algorithm = algorithm,
        .datasetSize = config->datasetSize,
        .operationCount = config->operationCount,
        .queryCount = workload->queryCount,
        .updateCount = workload->updateCount,
        .runCount = config->runCount,
        .buildTimeMs = 0.0,
        .queryTimeMs = 0.0,
        .updateTimeMs = 0.0,
        .totalExecutionTimeMs = 0.0,
        .memoryUsageBytes = 0U,
        .checksum = 0U
    };
}

static void addChecksum(
    StepCount *checksum,
    StepCount value,
    const Operation *operation
)
{
    StepCount mixedValue = value;

    mixedValue ^= (StepCount)operation->type
        + UINT64_C(0x9E3779B97F4A7C15);

    mixedValue ^= (StepCount)operation->left
        + UINT64_C(0xBF58476D1CE4E5B9);

    mixedValue ^= (StepCount)operation->right
        + UINT64_C(0x94D049BB133111EB);

    *checksum += mixedValue;
}

static bool isSumCompatibleOperation(OperationType type)
{
    return type == OPERATION_SUM
        || type == OPERATION_UPDATE;
}

static DataIndex countSumCompatibleQueries(
    const Workload *workload
)
{
    DataIndex count = 0U;

    for (DataIndex index = 0U; index < workload->count; ++index)
    {
        if (workload->operations[index].type == OPERATION_SUM)
        {
            ++count;
        }
    }

    return count;
}

static DataIndex countUpdates(
    const Workload *workload
)
{
    DataIndex count = 0U;

    for (DataIndex index = 0U; index < workload->count; ++index)
    {
        if (workload->operations[index].type == OPERATION_UPDATE)
        {
            ++count;
        }
    }

    return count;
}

const char *benchmarkAlgorithmName(
    BenchmarkAlgorithm algorithm
)
{
    switch (algorithm)
    {
        case BENCHMARK_ARRAY:
            return "Array";

        case BENCHMARK_PREFIX_SUM:
            return "Prefix Sum";

        case BENCHMARK_FENWICK_TREE:
            return "Binary Indexed Tree";

        case BENCHMARK_SEGMENT_TREE:
            return "Segment Tree";

        default:
            return "Unknown";
    }
}

BenchmarkConfig createDefaultBenchmarkConfig(
    DataIndex datasetSize
)
{
    BenchmarkConfig config = {
        .datasetSize = datasetSize,
        .operationCount = datasetSize,
        .runCount = DEFAULT_BENCHMARK_RUNS,
        .seed = DEFAULT_RANDOM_SEED,
        .minimumStepCount = DEFAULT_MIN_STEP_COUNT,
        .maximumStepCount = DEFAULT_MAX_STEP_COUNT
    };

    return config;
}

StatusCode validateBenchmarkConfig(
    const BenchmarkConfig *config
)
{
    if (
        config == NULL
        || config->datasetSize == 0U
        || config->operationCount == 0U
        || config->runCount == 0U
        || config->minimumStepCount > config->maximumStepCount
    )
    {
        return STATUS_INVALID_ARGUMENT;
    }

    if (
        config->datasetSize
            > SIZE_MAX / sizeof(StepCount)
        || config->operationCount
            > SIZE_MAX / sizeof(Operation)
    )
    {
        return STATUS_ALLOCATION_FAILED;
    }

    return STATUS_SUCCESS;
}

StatusCode benchmarkArray(
    const StepCount initialData[],
    const Workload *workload,
    const BenchmarkConfig *config,
    BenchmarkResult *result
)
{
    if (
        initialData == NULL
        || result == NULL
        || validateBenchmarkConfig(config) != STATUS_SUCCESS
        || !isValidWorkload(workload, config)
    )
    {
        return STATUS_INVALID_ARGUMENT;
    }

    initializeBenchmarkResult(
        result,
        BENCHMARK_ARRAY,
        config,
        workload
    );

    double totalBuildTime = 0.0;
    double totalQueryTime = 0.0;
    double totalUpdateTime = 0.0;
    double totalExecutionTime = 0.0;
    StepCount referenceChecksum = 0U;

    for (DataIndex run = 0U; run < config->runCount; ++run)
    {
        StepCount *data = NULL;
        StepCount runChecksum = 0U;

        const double totalStart = currentTimeMilliseconds();
        const double buildStart = currentTimeMilliseconds();

        StatusCode status = allocateDataCopy(
            initialData,
            config->datasetSize,
            &data
        );

        const double buildEnd = currentTimeMilliseconds();

        if (status != STATUS_SUCCESS)
        {
            return status;
        }

        double queryTime = 0.0;
        double updateTime = 0.0;

        for (
            DataIndex index = 0U;
            index < workload->count;
            ++index
        )
        {
            const Operation *operation =
                &workload->operations[index];

            StepCount queryResult = 0U;
            double operationStart;
            double operationEnd;

            switch (operation->type)
            {
                case OPERATION_SUM:
                    operationStart = currentTimeMilliseconds();

                    status = arrayRangeSum(
                        data,
                        config->datasetSize,
                        operation->left,
                        operation->right,
                        &queryResult
                    );

                    operationEnd = currentTimeMilliseconds();
                    queryTime += operationEnd - operationStart;
                    break;

                case OPERATION_MIN:
                    operationStart = currentTimeMilliseconds();

                    status = arrayRangeMin(
                        data,
                        config->datasetSize,
                        operation->left,
                        operation->right,
                        &queryResult
                    );

                    operationEnd = currentTimeMilliseconds();
                    queryTime += operationEnd - operationStart;
                    break;

                case OPERATION_MAX:
                    operationStart = currentTimeMilliseconds();

                    status = arrayRangeMax(
                        data,
                        config->datasetSize,
                        operation->left,
                        operation->right,
                        &queryResult
                    );

                    operationEnd = currentTimeMilliseconds();
                    queryTime += operationEnd - operationStart;
                    break;

                case OPERATION_UPDATE:
                    operationStart = currentTimeMilliseconds();

                    status = arrayUpdate(
                        data,
                        config->datasetSize,
                        operation->left,
                        operation->value
                    );

                    operationEnd = currentTimeMilliseconds();
                    updateTime += operationEnd - operationStart;
                    break;

                default:
                    status = STATUS_INVALID_ARGUMENT;
                    break;
            }

            if (status != STATUS_SUCCESS)
            {
                free(data);
                return status;
            }

            if (operation->type != OPERATION_UPDATE)
            {
                addChecksum(
                    &runChecksum,
                    queryResult,
                    operation
                );
            }
        }

        const double totalEnd = currentTimeMilliseconds();

        free(data);

        if (run == 0U)
        {
            referenceChecksum = runChecksum;
        }
        else if (runChecksum != referenceChecksum)
        {
            return STATUS_INVALID_ARGUMENT;
        }

        totalBuildTime += buildEnd - buildStart;
        totalQueryTime += queryTime;
        totalUpdateTime += updateTime;
        totalExecutionTime += totalEnd - totalStart;
    }

    const double runCount = (double)config->runCount;

    result->buildTimeMs = totalBuildTime / runCount;
    result->queryTimeMs = totalQueryTime / runCount;
    result->updateTimeMs = totalUpdateTime / runCount;
    result->totalExecutionTimeMs =
        totalExecutionTime / runCount;
    result->memoryUsageBytes =
        config->datasetSize * sizeof(StepCount);
    result->checksum = referenceChecksum;

    return STATUS_SUCCESS;
}

StatusCode benchmarkPrefixSum(
    const StepCount initialData[],
    const Workload *workload,
    const BenchmarkConfig *config,
    BenchmarkResult *result
)
{
    if (
        initialData == NULL
        || result == NULL
        || validateBenchmarkConfig(config) != STATUS_SUCCESS
        || !isValidWorkload(workload, config)
    )
    {
        return STATUS_INVALID_ARGUMENT;
    }

    initializeBenchmarkResult(
        result,
        BENCHMARK_PREFIX_SUM,
        config,
        workload
    );

    result->queryCount = countSumCompatibleQueries(workload);
    result->updateCount = countUpdates(workload);

    double totalBuildTime = 0.0;
    double totalQueryTime = 0.0;
    double totalUpdateTime = 0.0;
    double totalExecutionTime = 0.0;
    StepCount referenceChecksum = 0U;

    for (DataIndex run = 0U; run < config->runCount; ++run)
    {
        StepCount *data = NULL;
        StepCount *prefix = NULL;
        StepCount runChecksum = 0U;

        const double totalStart = currentTimeMilliseconds();
        const double buildStart = currentTimeMilliseconds();

        StatusCode status = allocateDataCopy(
            initialData,
            config->datasetSize,
            &data
        );

        if (status != STATUS_SUCCESS)
        {
            return status;
        }

        prefix = malloc(
            config->datasetSize * sizeof(*prefix)
        );

        if (prefix == NULL)
        {
            free(data);
            return STATUS_ALLOCATION_FAILED;
        }

        status = buildPrefixSum(
            data,
            config->datasetSize,
            prefix
        );

        const double buildEnd = currentTimeMilliseconds();

        if (status != STATUS_SUCCESS)
        {
            free(prefix);
            free(data);
            return status;
        }

        double queryTime = 0.0;
        double updateTime = 0.0;

        for (
            DataIndex index = 0U;
            index < workload->count;
            ++index
        )
        {
            const Operation *operation =
                &workload->operations[index];

            if (!isSumCompatibleOperation(operation->type))
            {
                continue;
            }

            StepCount queryResult = 0U;
            double operationStart;
            double operationEnd;

            if (operation->type == OPERATION_SUM)
            {
                operationStart = currentTimeMilliseconds();

                status = prefixRangeSum(
                    prefix,
                    config->datasetSize,
                    operation->left,
                    operation->right,
                    &queryResult
                );

                operationEnd = currentTimeMilliseconds();
                queryTime += operationEnd - operationStart;

                if (status == STATUS_SUCCESS)
                {
                    addChecksum(
                        &runChecksum,
                        queryResult,
                        operation
                    );
                }
            }
            else
            {
                operationStart = currentTimeMilliseconds();

                status = prefixUpdate(
                    data,
                    prefix,
                    config->datasetSize,
                    operation->left,
                    operation->value
                );

                operationEnd = currentTimeMilliseconds();
                updateTime += operationEnd - operationStart;
            }

            if (status != STATUS_SUCCESS)
            {
                free(prefix);
                free(data);
                return status;
            }
        }

        const double totalEnd = currentTimeMilliseconds();

        free(prefix);
        free(data);

        if (run == 0U)
        {
            referenceChecksum = runChecksum;
        }
        else if (runChecksum != referenceChecksum)
        {
            return STATUS_INVALID_ARGUMENT;
        }

        totalBuildTime += buildEnd - buildStart;
        totalQueryTime += queryTime;
        totalUpdateTime += updateTime;
        totalExecutionTime += totalEnd - totalStart;
    }

    const double runCount = (double)config->runCount;

    result->buildTimeMs = totalBuildTime / runCount;
    result->queryTimeMs = totalQueryTime / runCount;
    result->updateTimeMs = totalUpdateTime / runCount;
    result->totalExecutionTimeMs =
        totalExecutionTime / runCount;
    result->memoryUsageBytes =
        config->datasetSize * sizeof(StepCount) * 2U;
    result->checksum = referenceChecksum;

    return STATUS_SUCCESS;
}

StatusCode benchmarkFenwickTree(
    const StepCount initialData[],
    const Workload *workload,
    const BenchmarkConfig *config,
    BenchmarkResult *result
)
{
    if (
        initialData == NULL
        || result == NULL
        || validateBenchmarkConfig(config) != STATUS_SUCCESS
        || !isValidWorkload(workload, config)
    )
    {
        return STATUS_INVALID_ARGUMENT;
    }

    initializeBenchmarkResult(
        result,
        BENCHMARK_FENWICK_TREE,
        config,
        workload
    );

    result->queryCount = countSumCompatibleQueries(workload);
    result->updateCount = countUpdates(workload);

    double totalBuildTime = 0.0;
    double totalQueryTime = 0.0;
    double totalUpdateTime = 0.0;
    double totalExecutionTime = 0.0;
    StepCount referenceChecksum = 0U;

    for (DataIndex run = 0U; run < config->runCount; ++run)
    {
        FenwickTree fenwickTree = {
            .tree = NULL,
            .data = NULL,
            .size = 0U
        };

        StepCount runChecksum = 0U;

        const double totalStart = currentTimeMilliseconds();
        const double buildStart = currentTimeMilliseconds();

        StatusCode status = bitBuild(
            &fenwickTree,
            initialData,
            config->datasetSize
        );

        const double buildEnd = currentTimeMilliseconds();

        if (status != STATUS_SUCCESS)
        {
            return status;
        }

        double queryTime = 0.0;
        double updateTime = 0.0;

        for (
            DataIndex index = 0U;
            index < workload->count;
            ++index
        )
        {
            const Operation *operation =
                &workload->operations[index];

            if (!isSumCompatibleOperation(operation->type))
            {
                continue;
            }

            StepCount queryResult = 0U;
            double operationStart;
            double operationEnd;

            if (operation->type == OPERATION_SUM)
            {
                operationStart = currentTimeMilliseconds();

                status = bitRangeSum(
                    &fenwickTree,
                    operation->left,
                    operation->right,
                    &queryResult
                );

                operationEnd = currentTimeMilliseconds();
                queryTime += operationEnd - operationStart;

                if (status == STATUS_SUCCESS)
                {
                    addChecksum(
                        &runChecksum,
                        queryResult,
                        operation
                    );
                }
            }
            else
            {
                operationStart = currentTimeMilliseconds();

                status = bitUpdate(
                    &fenwickTree,
                    operation->left,
                    operation->value
                );

                operationEnd = currentTimeMilliseconds();
                updateTime += operationEnd - operationStart;
            }

            if (status != STATUS_SUCCESS)
            {
                bitDestroy(&fenwickTree);
                return status;
            }
        }

        const double totalEnd = currentTimeMilliseconds();

        bitDestroy(&fenwickTree);

        if (run == 0U)
        {
            referenceChecksum = runChecksum;
        }
        else if (runChecksum != referenceChecksum)
        {
            return STATUS_INVALID_ARGUMENT;
        }

        totalBuildTime += buildEnd - buildStart;
        totalQueryTime += queryTime;
        totalUpdateTime += updateTime;
        totalExecutionTime += totalEnd - totalStart;
    }

    const double runCount = (double)config->runCount;

    result->buildTimeMs = totalBuildTime / runCount;
    result->queryTimeMs = totalQueryTime / runCount;
    result->updateTimeMs = totalUpdateTime / runCount;
    result->totalExecutionTimeMs =
        totalExecutionTime / runCount;
    result->memoryUsageBytes =
        config->datasetSize * sizeof(StepCount)
        + (config->datasetSize + 1U) * sizeof(StepCount);
    result->checksum = referenceChecksum;

    return STATUS_SUCCESS;
}

StatusCode benchmarkSegmentTree(
    const StepCount initialData[],
    const Workload *workload,
    const BenchmarkConfig *config,
    BenchmarkResult *result
)
{
    if (
        initialData == NULL
        || result == NULL
        || validateBenchmarkConfig(config) != STATUS_SUCCESS
        || !isValidWorkload(workload, config)
    )
    {
        return STATUS_INVALID_ARGUMENT;
    }

    initializeBenchmarkResult(
        result,
        BENCHMARK_SEGMENT_TREE,
        config,
        workload
    );

    double totalBuildTime = 0.0;
    double totalQueryTime = 0.0;
    double totalUpdateTime = 0.0;
    double totalExecutionTime = 0.0;
    StepCount referenceChecksum = 0U;

    for (DataIndex run = 0U; run < config->runCount; ++run)
    {
        SegmentTree segmentTree = {
            .tree = NULL,
            .data = NULL,
            .size = 0U,
            .capacity = 0U
        };

        StepCount runChecksum = 0U;

        const double totalStart = currentTimeMilliseconds();
        const double buildStart = currentTimeMilliseconds();

        StatusCode status = buildSegmentTree(
            &segmentTree,
            initialData,
            config->datasetSize
        );

        const double buildEnd = currentTimeMilliseconds();

        if (status != STATUS_SUCCESS)
        {
            return status;
        }

        double queryTime = 0.0;
        double updateTime = 0.0;

        for (
            DataIndex index = 0U;
            index < workload->count;
            ++index
        )
        {
            const Operation *operation =
                &workload->operations[index];

            SegmentNode queryResult = {0U, 0U, 0U};
            StepCount checksumValue = 0U;
            double operationStart;
            double operationEnd;

            if (operation->type == OPERATION_UPDATE)
            {
                operationStart = currentTimeMilliseconds();

                status = updateSegmentTree(
                    &segmentTree,
                    operation->left,
                    operation->value
                );

                operationEnd = currentTimeMilliseconds();
                updateTime += operationEnd - operationStart;
            }
            else
            {
                operationStart = currentTimeMilliseconds();

                status = querySegmentTree(
                    &segmentTree,
                    operation->left,
                    operation->right,
                    &queryResult
                );

                operationEnd = currentTimeMilliseconds();
                queryTime += operationEnd - operationStart;

                if (status == STATUS_SUCCESS)
                {
                    switch (operation->type)
                    {
                        case OPERATION_SUM:
                            checksumValue = queryResult.sum;
                            break;

                        case OPERATION_MIN:
                            checksumValue = queryResult.minimum;
                            break;

                        case OPERATION_MAX:
                            checksumValue = queryResult.maximum;
                            break;

                        default:
                            status = STATUS_INVALID_ARGUMENT;
                            break;
                    }

                    if (status == STATUS_SUCCESS)
                    {
                        addChecksum(
                            &runChecksum,
                            checksumValue,
                            operation
                        );
                    }
                }
            }

            if (status != STATUS_SUCCESS)
            {
                destroySegmentTree(&segmentTree);
                return status;
            }
        }

        const double totalEnd = currentTimeMilliseconds();

        destroySegmentTree(&segmentTree);

        if (run == 0U)
        {
            referenceChecksum = runChecksum;
        }
        else if (runChecksum != referenceChecksum)
        {
            return STATUS_INVALID_ARGUMENT;
        }

        totalBuildTime += buildEnd - buildStart;
        totalQueryTime += queryTime;
        totalUpdateTime += updateTime;
        totalExecutionTime += totalEnd - totalStart;
    }

    const double runCount = (double)config->runCount;

    result->buildTimeMs = totalBuildTime / runCount;
    result->queryTimeMs = totalQueryTime / runCount;
    result->updateTimeMs = totalUpdateTime / runCount;
    result->totalExecutionTimeMs =
        totalExecutionTime / runCount;
    result->memoryUsageBytes =
        config->datasetSize * sizeof(StepCount)
        + config->datasetSize * 4U * sizeof(SegmentNode);
    result->checksum = referenceChecksum;

    return STATUS_SUCCESS;
}

StatusCode runAllBenchmarks(
    const BenchmarkConfig *config,
    BenchmarkResult results[BENCHMARK_ALGORITHM_COUNT]
)
{
    if (
        results == NULL
        || validateBenchmarkConfig(config) != STATUS_SUCCESS
    )
    {
        return STATUS_INVALID_ARGUMENT;
    }

    StepCount *dataset = NULL;

    StatusCode status = allocateDataCopy(
        (const StepCount[]){0U},
        1U,
        &dataset
    );

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    free(dataset);
    dataset = NULL;

    dataset = malloc(
        config->datasetSize * sizeof(*dataset)
    );

    if (dataset == NULL)
    {
        return STATUS_ALLOCATION_FAILED;
    }

    status = generateDataset(
        dataset,
        config->datasetSize,
        config->minimumStepCount,
        config->maximumStepCount,
        config->seed
    );

    if (status != STATUS_SUCCESS)
    {
        free(dataset);
        return status;
    }

    Workload workload = {
        .operations = NULL,
        .count = 0U,
        .queryCount = 0U,
        .updateCount = 0U,
        .seed = 0U
    };

    status = generateWorkload(
        &workload,
        config->datasetSize,
        config->operationCount,
        QUERY_PERCENTAGE,
        UPDATE_PERCENTAGE,
        config->minimumStepCount,
        config->maximumStepCount,
        config->seed
    );

    if (status != STATUS_SUCCESS)
    {
        free(dataset);
        return status;
    }

    status = benchmarkArray(
        dataset,
        &workload,
        config,
        &results[BENCHMARK_ARRAY]
    );

    if (status == STATUS_SUCCESS)
    {
        status = benchmarkPrefixSum(
            dataset,
            &workload,
            config,
            &results[BENCHMARK_PREFIX_SUM]
        );
    }

    if (status == STATUS_SUCCESS)
    {
        status = benchmarkFenwickTree(
            dataset,
            &workload,
            config,
            &results[BENCHMARK_FENWICK_TREE]
        );
    }

    if (status == STATUS_SUCCESS)
    {
        status = benchmarkSegmentTree(
            dataset,
            &workload,
            config,
            &results[BENCHMARK_SEGMENT_TREE]
        );
    }

    destroyWorkload(&workload);
    free(dataset);

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    if (
        results[BENCHMARK_PREFIX_SUM].checksum
        != results[BENCHMARK_FENWICK_TREE].checksum
    )
    {
        return STATUS_INVALID_ARGUMENT;
    }

    return STATUS_SUCCESS;
}

StatusCode runStandardBenchmarkSuite(
    BenchmarkResult results[],
    DataIndex resultCapacity,
    DataIndex *resultCount
)
{
    const DataIndex requiredCapacity =
        3U * (DataIndex)BENCHMARK_ALGORITHM_COUNT;

    if (
        results == NULL
        || resultCount == NULL
        || resultCapacity < requiredCapacity
    )
    {
        return STATUS_INVALID_ARGUMENT;
    }

    *resultCount = 0U;

    const DataIndex datasetSizes[] = {
        SMALL_DATASET_SIZE,
        MEDIUM_DATASET_SIZE,
        LARGE_DATASET_SIZE
    };

    const DataIndex datasetCount =
        sizeof(datasetSizes) / sizeof(datasetSizes[0]);

    for (
        DataIndex datasetIndex = 0U;
        datasetIndex < datasetCount;
        ++datasetIndex
    )
    {
        const BenchmarkConfig config =
            createDefaultBenchmarkConfig(
                datasetSizes[datasetIndex]
            );

        BenchmarkResult currentResults[
            BENCHMARK_ALGORITHM_COUNT
        ];

        StatusCode status = runAllBenchmarks(
            &config,
            currentResults
        );

        if (status != STATUS_SUCCESS)
        {
            return status;
        }

        for (
            DataIndex algorithm = 0U;
            algorithm < BENCHMARK_ALGORITHM_COUNT;
            ++algorithm
        )
        {
            results[*resultCount] = currentResults[algorithm];
            ++(*resultCount);
        }
    }

    return STATUS_SUCCESS;
}

void printBenchmarkResult(
    const BenchmarkResult *result
)
{
    if (result == NULL)
    {
        return;
    }

    printf(
        "%-20s | N=%-8zu | Ops=%-8zu | "
        "Build=%10.3f ms | Query=%10.3f ms | "
        "Update=%10.3f ms | Total=%10.3f ms | "
        "Memory=%-12zu | Checksum=%" PRIu64 "\n",
        benchmarkAlgorithmName(result->algorithm),
        result->datasetSize,
        result->operationCount,
        result->buildTimeMs,
        result->queryTimeMs,
        result->updateTimeMs,
        result->totalExecutionTimeMs,
        result->memoryUsageBytes,
        result->checksum
    );
}

void printBenchmarkTable(
    const BenchmarkResult results[],
    DataIndex resultCount
)
{
    if (results == NULL || resultCount == 0U)
    {
        return;
    }

    printf("\nBenchmark results\n");
    printf("=================\n");

    for (DataIndex index = 0U; index < resultCount; ++index)
    {
        printBenchmarkResult(&results[index]);
    }
}

StatusCode writeBenchmarkResultsCsv(
    const char *filePath,
    const BenchmarkResult results[],
    DataIndex resultCount,
    bool append
)
{
    if (
        filePath == NULL
        || results == NULL
        || resultCount == 0U
    )
    {
        return STATUS_INVALID_ARGUMENT;
    }

    FILE *file = fopen(filePath, append ? "a" : "w");

    if (file == NULL)
    {
        return STATUS_FILE_ERROR;
    }

    if (!append)
    {
        fprintf(
            file,
            "algorithm,dataset_size,operation_count,"
            "query_count,update_count,run_count,"
            "build_time_ms,query_time_ms,update_time_ms,"
            "total_execution_time_ms,memory_usage_bytes,"
            "checksum\n"
        );
    }

    for (DataIndex index = 0U; index < resultCount; ++index)
    {
        const BenchmarkResult *result = &results[index];

        fprintf(
            file,
            "\"%s\",%zu,%zu,%zu,%zu,%zu,"
            "%.6f,%.6f,%.6f,%.6f,%zu,%" PRIu64 "\n",
            benchmarkAlgorithmName(result->algorithm),
            result->datasetSize,
            result->operationCount,
            result->queryCount,
            result->updateCount,
            result->runCount,
            result->buildTimeMs,
            result->queryTimeMs,
            result->updateTimeMs,
            result->totalExecutionTimeMs,
            result->memoryUsageBytes,
            result->checksum
        );
    }

    if (fclose(file) != 0)
    {
        return STATUS_FILE_ERROR;
    }

    return STATUS_SUCCESS;
}

StatusCode writeBenchmarkEnvironment(
    const char *filePath,
    const BenchmarkConfig *config
)
{
    if (
        filePath == NULL
        || validateBenchmarkConfig(config) != STATUS_SUCCESS
    )
    {
        return STATUS_INVALID_ARGUMENT;
    }

    FILE *file = fopen(filePath, "w");

    if (file == NULL)
    {
        return STATUS_FILE_ERROR;
    }

#if defined(__clang__)
    fprintf(file, "Compiler: Clang %s\n", __clang_version__);
#elif defined(__GNUC__)
    fprintf(
        file,
        "Compiler: GCC %d.%d.%d\n",
        __GNUC__,
        __GNUC_MINOR__,
        __GNUC_PATCHLEVEL__
    );
#elif defined(_MSC_VER)
    fprintf(file, "Compiler: MSVC %d\n", _MSC_VER);
#else
    fprintf(file, "Compiler: Unknown\n");
#endif

    fprintf(file, "C standard value: %ld\n", (long)__STDC_VERSION__);

#ifdef NDEBUG
    fprintf(file, "Build mode: Release\n");
#else
    fprintf(file, "Build mode: Debug\n");
#endif

    fprintf(file, "sizeof(StepCount): %zu bytes\n", sizeof(StepCount));
    fprintf(file, "sizeof(DataIndex): %zu bytes\n", sizeof(DataIndex));
    fprintf(file, "sizeof(Operation): %zu bytes\n", sizeof(Operation));
    fprintf(file, "sizeof(SegmentNode): %zu bytes\n", sizeof(SegmentNode));
    fprintf(file, "Dataset size: %zu\n", config->datasetSize);
    fprintf(file, "Operation count: %zu\n", config->operationCount);
    fprintf(file, "Benchmark runs: %zu\n", config->runCount);
    fprintf(file, "Random seed: %" PRIu32 "\n", config->seed);
    fprintf(
        file,
        "Step count range: %" PRIu64 " to %" PRIu64 "\n",
        config->minimumStepCount,
        config->maximumStepCount
    );
    fprintf(file, "Query percentage: %u\n", QUERY_PERCENTAGE);
    fprintf(file, "Update percentage: %u\n", UPDATE_PERCENTAGE);
    fprintf(file, "Time unit: milliseconds\n");
    fprintf(file, "Memory metric: estimated bytes\n");

    if (fclose(file) != 0)
    {
        return STATUS_FILE_ERROR;
    }

    return STATUS_SUCCESS;
}