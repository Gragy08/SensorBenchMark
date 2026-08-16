#include "demo.h"

#include <inttypes.h>
#include <stdio.h>

#include "benchmark.h"
#include "fenwick_tree.h"
#include "prefix_sum.h"
#include "raw_array.h"
#include "segment_tree.h"

#define STANDARD_BENCHMARK_RESULT_COUNT \
    ((DataIndex)3U * (DataIndex)BENCHMARK_ALGORITHM_COUNT)

static const StepCount INITIAL_SAMPLE_STEPS[SAMPLE_DATA_SIZE] = {
    UINT64_C(6000),
    UINT64_C(8500),
    UINT64_C(7200),
    UINT64_C(9000),
    UINT64_C(5500),
    UINT64_C(12000),
    UINT64_C(10000)
};

static StepCount demoData[SAMPLE_DATA_SIZE] = {0U};
static StepCount demoPrefix[SAMPLE_DATA_SIZE] = {0U};

static FenwickTree demoFenwickTree = {
    .tree = NULL,
    .data = NULL,
    .size = 0U
};

static SegmentTree demoSegmentTree = {
    .tree = NULL,
    .data = NULL,
    .size = 0U,
    .capacity = 0U
};

static bool demoInitialized = false;

static bool convertDayRangeToIndexes(
    DataIndex leftDay,
    DataIndex rightDay,
    DataIndex *leftIndex,
    DataIndex *rightIndex
)
{
    if (
        leftIndex == NULL
        || rightIndex == NULL
        || leftDay == 0U
        || rightDay == 0U
        || leftDay > rightDay
        || rightDay > SAMPLE_DATA_SIZE
    )
    {
        return false;
    }

    *leftIndex = leftDay - 1U;
    *rightIndex = rightDay - 1U;

    return true;
}

static StatusCode initializeDemo(void)
{
    if (demoInitialized)
    {
        return STATUS_SUCCESS;
    }

    for (DataIndex index = 0U; index < SAMPLE_DATA_SIZE; ++index)
    {
        demoData[index] = INITIAL_SAMPLE_STEPS[index];
    }

    StatusCode status = buildPrefixSum(
        demoData,
        SAMPLE_DATA_SIZE,
        demoPrefix
    );

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    status = bitBuild(
        &demoFenwickTree,
        demoData,
        SAMPLE_DATA_SIZE
    );

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    status = buildSegmentTree(
        &demoSegmentTree,
        demoData,
        SAMPLE_DATA_SIZE
    );

    if (status != STATUS_SUCCESS)
    {
        bitDestroy(&demoFenwickTree);
        return status;
    }

    demoInitialized = true;

    return STATUS_SUCCESS;
}

static StatusCode runDemoBenchmark(void)
{
    BenchmarkResult results[
        STANDARD_BENCHMARK_RESULT_COUNT
    ];

    DataIndex resultCount = 0U;

    printf("\nRunning standard benchmark suite...\n");
    printf("Please wait.\n");

    StatusCode status = runStandardBenchmarkSuite(
        results,
        STANDARD_BENCHMARK_RESULT_COUNT,
        &resultCount
    );

    if (status != STATUS_SUCCESS)
    {
        fprintf(
            stderr,
            "Failed to run benchmark suite with status: %d\n",
            status
        );

        return status;
    }

    printBenchmarkTable(results, resultCount);

    status = writeBenchmarkResultsCsv(
        DEFAULT_BENCHMARK_RESULT_FILE,
        results,
        resultCount,
        false
    );

    if (status != STATUS_SUCCESS)
    {
        fprintf(
            stderr,
            "Failed to write benchmark results with status: %d\n",
            status
        );

        return status;
    }

    const BenchmarkConfig environmentConfig =
        createDefaultBenchmarkConfig(LARGE_DATASET_SIZE);

    status = writeBenchmarkEnvironment(
        DEFAULT_BENCHMARK_ENVIRONMENT_FILE,
        &environmentConfig
    );

    if (status != STATUS_SUCCESS)
    {
        fprintf(
            stderr,
            "Failed to write benchmark environment with status: %d\n",
            status
        );

        return status;
    }

    printf(
        "\nBenchmark results written to: %s\n",
        DEFAULT_BENCHMARK_RESULT_FILE
    );

    printf(
        "Benchmark environment written to: %s\n",
        DEFAULT_BENCHMARK_ENVIRONMENT_FILE
    );

    return STATUS_SUCCESS;
}

static void clearInputBuffer(void)
{
    int character;

    while (
        (character = getchar()) != '\n'
        && character != EOF
    )
    {
    }
}

void displayStepData(
    const StepCount data[],
    DataIndex size
)
{
    if (data == NULL || size == 0U)
    {
        printf("No step data available.\n");
        return;
    }

    printf("\nCurrent step data\n");
    printf("=================\n");

    printf("Day:   ");

    for (DataIndex index = 0U; index < size; ++index)
    {
        printf("%10zu", index + 1U);
    }

    printf("\nSteps: ");

    for (DataIndex index = 0U; index < size; ++index)
    {
        printf("%10" PRIu64, data[index]);
    }

    printf("\n");
}

void displayDemoMenu(void)
{
    printf("\nSensor Benchmark Demo\n");
    printf("=====================\n");
    printf("1. Show step data\n");
    printf("2. Range Sum\n");
    printf("3. Range Minimum\n");
    printf("4. Range Maximum\n");
    printf("5. Update step count\n");
    printf("6. Validate all structures\n");
    printf("7. Run benchmark\n");
    printf("8. Reset sample data\n");
    printf("9. Exit\n");
    printf("Select command: ");
}

StatusCode demoRangeSum(
    DataIndex leftDay,
    DataIndex rightDay
)
{
    StatusCode status = initializeDemo();

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    DataIndex leftIndex;
    DataIndex rightIndex;

    if (!convertDayRangeToIndexes(
            leftDay,
            rightDay,
            &leftIndex,
            &rightIndex
        ))
    {
        return STATUS_INDEX_OUT_OF_RANGE;
    }

    StepCount arrayResult = 0U;
    StepCount prefixResult = 0U;
    StepCount bitResult = 0U;
    SegmentNode segmentResult = {0U, 0U, 0U};

    status = arrayRangeSum(
        demoData,
        SAMPLE_DATA_SIZE,
        leftIndex,
        rightIndex,
        &arrayResult
    );

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    status = prefixRangeSum(
        demoPrefix,
        SAMPLE_DATA_SIZE,
        leftIndex,
        rightIndex,
        &prefixResult
    );

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    status = bitRangeSum(
        &demoFenwickTree,
        leftIndex,
        rightIndex,
        &bitResult
    );

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    status = querySegmentTree(
        &demoSegmentTree,
        leftIndex,
        rightIndex,
        &segmentResult
    );

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    printf("\nSUM %zu %zu\n", leftDay, rightDay);
    printf("============================\n");
    printf("Array:               %" PRIu64 "\n", arrayResult);
    printf("Prefix Sum:          %" PRIu64 "\n", prefixResult);
    printf("Binary Indexed Tree: %" PRIu64 "\n", bitResult);
    printf("Segment Tree:        %" PRIu64 "\n", segmentResult.sum);

    if (
        arrayResult != prefixResult
        || arrayResult != bitResult
        || arrayResult != segmentResult.sum
    )
    {
        printf("Validation: FAILED\n");
        return STATUS_INVALID_ARGUMENT;
    }

    printf("Validation: PASSED\n");

    return STATUS_SUCCESS;
}

StatusCode demoRangeMin(
    DataIndex leftDay,
    DataIndex rightDay
)
{
    StatusCode status = initializeDemo();

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    DataIndex leftIndex;
    DataIndex rightIndex;

    if (!convertDayRangeToIndexes(
            leftDay,
            rightDay,
            &leftIndex,
            &rightIndex
        ))
    {
        return STATUS_INDEX_OUT_OF_RANGE;
    }

    StepCount arrayResult = 0U;
    SegmentNode segmentResult = {0U, 0U, 0U};

    status = arrayRangeMin(
        demoData,
        SAMPLE_DATA_SIZE,
        leftIndex,
        rightIndex,
        &arrayResult
    );

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    status = querySegmentTree(
        &demoSegmentTree,
        leftIndex,
        rightIndex,
        &segmentResult
    );

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    printf("\nMIN %zu %zu\n", leftDay, rightDay);
    printf("====================\n");
    printf("Array:        %" PRIu64 "\n", arrayResult);
    printf("Segment Tree: %" PRIu64 "\n", segmentResult.minimum);

    if (arrayResult != segmentResult.minimum)
    {
        printf("Validation: FAILED\n");
        return STATUS_INVALID_ARGUMENT;
    }

    printf("Validation: PASSED\n");

    return STATUS_SUCCESS;
}

StatusCode demoRangeMax(
    DataIndex leftDay,
    DataIndex rightDay
)
{
    StatusCode status = initializeDemo();

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    DataIndex leftIndex;
    DataIndex rightIndex;

    if (!convertDayRangeToIndexes(
            leftDay,
            rightDay,
            &leftIndex,
            &rightIndex
        ))
    {
        return STATUS_INDEX_OUT_OF_RANGE;
    }

    StepCount arrayResult = 0U;
    SegmentNode segmentResult = {0U, 0U, 0U};

    status = arrayRangeMax(
        demoData,
        SAMPLE_DATA_SIZE,
        leftIndex,
        rightIndex,
        &arrayResult
    );

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    status = querySegmentTree(
        &demoSegmentTree,
        leftIndex,
        rightIndex,
        &segmentResult
    );

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    printf("\nMAX %zu %zu\n", leftDay, rightDay);
    printf("====================\n");
    printf("Array:        %" PRIu64 "\n", arrayResult);
    printf("Segment Tree: %" PRIu64 "\n", segmentResult.maximum);

    if (arrayResult != segmentResult.maximum)
    {
        printf("Validation: FAILED\n");
        return STATUS_INVALID_ARGUMENT;
    }

    printf("Validation: PASSED\n");

    return STATUS_SUCCESS;
}

StatusCode demoUpdate(
    DataIndex day,
    StepCount newValue
)
{
    StatusCode status = initializeDemo();

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    if (day == 0U || day > SAMPLE_DATA_SIZE)
    {
        return STATUS_INDEX_OUT_OF_RANGE;
    }

    const DataIndex index = day - 1U;
    const StepCount oldValue = demoData[index];

    status = prefixUpdate(
        demoData,
        demoPrefix,
        SAMPLE_DATA_SIZE,
        index,
        newValue
    );

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    status = bitUpdate(
        &demoFenwickTree,
        index,
        newValue
    );

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    status = updateSegmentTree(
        &demoSegmentTree,
        index,
        newValue
    );

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    printf(
        "\nUPDATE %zu %" PRIu64 "\n",
        day,
        newValue
    );

    printf("====================\n");
    printf("Old value: %" PRIu64 "\n", oldValue);
    printf("New value: %" PRIu64 "\n", newValue);
    printf("Update completed successfully.\n");

    return STATUS_SUCCESS;
}

StatusCode validateDemoStructures(void)
{
    StatusCode status = initializeDemo();

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    printf("\nValidating all query ranges\n");
    printf("===========================\n");

    for (DataIndex left = 0U; left < SAMPLE_DATA_SIZE; ++left)
    {
        for (
            DataIndex right = left;
            right < SAMPLE_DATA_SIZE;
            ++right
        )
        {
            StepCount arraySum = 0U;
            StepCount arrayMinimum = 0U;
            StepCount arrayMaximum = 0U;
            StepCount prefixSum = 0U;
            StepCount bitSum = 0U;
            SegmentNode segmentResult = {0U, 0U, 0U};

            status = arrayRangeSum(
                demoData,
                SAMPLE_DATA_SIZE,
                left,
                right,
                &arraySum
            );

            if (status != STATUS_SUCCESS)
            {
                return status;
            }

            status = arrayRangeMin(
                demoData,
                SAMPLE_DATA_SIZE,
                left,
                right,
                &arrayMinimum
            );

            if (status != STATUS_SUCCESS)
            {
                return status;
            }

            status = arrayRangeMax(
                demoData,
                SAMPLE_DATA_SIZE,
                left,
                right,
                &arrayMaximum
            );

            if (status != STATUS_SUCCESS)
            {
                return status;
            }

            status = prefixRangeSum(
                demoPrefix,
                SAMPLE_DATA_SIZE,
                left,
                right,
                &prefixSum
            );

            if (status != STATUS_SUCCESS)
            {
                return status;
            }

            status = bitRangeSum(
                &demoFenwickTree,
                left,
                right,
                &bitSum
            );

            if (status != STATUS_SUCCESS)
            {
                return status;
            }

            status = querySegmentTree(
                &demoSegmentTree,
                left,
                right,
                &segmentResult
            );

            if (status != STATUS_SUCCESS)
            {
                return status;
            }

            if (
                arraySum != prefixSum
                || arraySum != bitSum
                || arraySum != segmentResult.sum
                || arrayMinimum != segmentResult.minimum
                || arrayMaximum != segmentResult.maximum
            )
            {
                printf(
                    "[FAILED] Range [%zu, %zu]\n",
                    left + 1U,
                    right + 1U
                );

                return STATUS_INVALID_ARGUMENT;
            }
        }
    }

    printf(
        "[PASSED] All structures return consistent results "
        "for every valid range.\n"
    );

    return STATUS_SUCCESS;
}

StatusCode resetDemoData(void)
{
    destroyDemo();

    StatusCode status = initializeDemo();

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    printf("\nDemo data reset successfully.\n");
    displayStepData(demoData, SAMPLE_DATA_SIZE);

    return STATUS_SUCCESS;
}

void destroyDemo(void)
{
    bitDestroy(&demoFenwickTree);
    destroySegmentTree(&demoSegmentTree);

    for (DataIndex index = 0U; index < SAMPLE_DATA_SIZE; ++index)
    {
        demoData[index] = 0U;
        demoPrefix[index] = 0U;
    }

    demoInitialized = false;
}

StatusCode runAutomaticDemo(void)
{
    StatusCode status = resetDemoData();

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    printf("\nAutomatic demo\n");
    printf("==============\n");

    displayStepData(demoData, SAMPLE_DATA_SIZE);

    status = demoRangeSum(2U, 6U);

    if (status != STATUS_SUCCESS)
    {
        destroyDemo();
        return status;
    }

    status = demoRangeMin(1U, 7U);

    if (status != STATUS_SUCCESS)
    {
        destroyDemo();
        return status;
    }

    status = demoRangeMax(3U, 7U);

    if (status != STATUS_SUCCESS)
    {
        destroyDemo();
        return status;
    }

    status = demoUpdate(
        3U,
        UINT64_C(10000)
    );

    if (status != STATUS_SUCCESS)
    {
        destroyDemo();
        return status;
    }

    displayStepData(demoData, SAMPLE_DATA_SIZE);

    status = demoRangeSum(2U, 6U);

    if (status != STATUS_SUCCESS)
    {
        destroyDemo();
        return status;
    }

    status = demoRangeMin(1U, 7U);

    if (status != STATUS_SUCCESS)
    {
        destroyDemo();
        return status;
    }

    status = demoRangeMax(3U, 7U);

    if (status != STATUS_SUCCESS)
    {
        destroyDemo();
        return status;
    }

    status = validateDemoStructures();

    if (status != STATUS_SUCCESS)
    {
        destroyDemo();
        return status;
    }

    printf("\nAutomatic demo completed successfully.\n");

    return STATUS_SUCCESS;
}

StatusCode runInteractiveDemo(void)
{
    StatusCode status = initializeDemo();

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    bool running = true;

    while (running)
    {
        unsigned int commandValue = 0U;

        displayDemoMenu();

        if (scanf("%u", &commandValue) != 1)
        {
            printf("Invalid command. Please enter a number from 1 to 9.\n");
            clearInputBuffer();
            continue;
        }

        clearInputBuffer();

        switch ((DemoCommand)commandValue)
        {
            case DEMO_COMMAND_SHOW_DATA:
                displayStepData(
                    demoData,
                    SAMPLE_DATA_SIZE
                );
                break;

            case DEMO_COMMAND_SUM:
            case DEMO_COMMAND_MIN:
            case DEMO_COMMAND_MAX:
            {
                size_t leftDay;
                size_t rightDay;

                printf("Enter left day: ");

                if (scanf("%zu", &leftDay) != 1)
                {
                    printf("Invalid left day.\n");
                    clearInputBuffer();
                    break;
                }

                printf("Enter right day: ");

                if (scanf("%zu", &rightDay) != 1)
                {
                    printf("Invalid right day.\n");
                    clearInputBuffer();
                    break;
                }

                clearInputBuffer();

                if (commandValue == DEMO_COMMAND_SUM)
                {
                    status = demoRangeSum(leftDay, rightDay);
                }
                else if (commandValue == DEMO_COMMAND_MIN)
                {
                    status = demoRangeMin(leftDay, rightDay);
                }
                else
                {
                    status = demoRangeMax(leftDay, rightDay);
                }

                if (status != STATUS_SUCCESS)
                {
                    printf(
                        "Query failed with status: %d\n",
                        status
                    );
                }

                break;
            }

            case DEMO_COMMAND_UPDATE:
            {
                size_t day;
                StepCount newValue;

                printf("Enter day: ");

                if (scanf("%zu", &day) != 1)
                {
                    printf("Invalid day.\n");
                    clearInputBuffer();
                    break;
                }

                printf("Enter new step count: ");

                if (scanf("%" SCNu64, &newValue) != 1)
                {
                    printf("Invalid step count.\n");
                    clearInputBuffer();
                    break;
                }

                clearInputBuffer();

                status = demoUpdate(day, newValue);

                if (status != STATUS_SUCCESS)
                {
                    printf(
                        "Update failed with status: %d\n",
                        status
                    );
                }

                break;
            }

            case DEMO_COMMAND_VALIDATE:
                status = validateDemoStructures();

                if (status != STATUS_SUCCESS)
                {
                    printf(
                        "Validation failed with status: %d\n",
                        status
                    );
                }

                break;

            case DEMO_COMMAND_BENCHMARK:
                status = runDemoBenchmark();

                if (status != STATUS_SUCCESS)
                {
                    printf(
                        "Benchmark failed with status: %d\n",
                        status
                    );
                }

                break;

            case DEMO_COMMAND_RESET:
                status = resetDemoData();

                if (status != STATUS_SUCCESS)
                {
                    printf(
                        "Reset failed with status: %d\n",
                        status
                    );
                }

                break;

            case DEMO_COMMAND_EXIT:
                running = false;
                break;

            default:
                printf(
                    "Invalid command. Please select from 1 to 9.\n"
                );
                break;
        }
    }

    destroyDemo();

    printf("\nDemo terminated successfully.\n");

    return STATUS_SUCCESS;
}