#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "benchmark.h"
#include "common.h"
#include "demo.h"
#include "test.h"

#define STANDARD_BENCHMARK_RESULT_COUNT \
    ((DataIndex)3U * (DataIndex)BENCHMARK_ALGORITHM_COUNT)

typedef enum
{
    MAIN_COMMAND_AUTOMATIC_DEMO = 1,
    MAIN_COMMAND_INTERACTIVE_DEMO,
    MAIN_COMMAND_TEST_SUITE,
    MAIN_COMMAND_BENCHMARK,
    MAIN_COMMAND_EXIT
} MainCommand;

static void displayMainMenu(void)
{
    printf("\nSensor Benchmark\n");
    printf("================\n");
    printf("1. Run automatic demo\n");
    printf("2. Run interactive demo\n");
    printf("3. Run test suite\n");
    printf("4. Run standard benchmark\n");
    printf("5. Exit\n");
    printf("Select command: ");
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

static void printUsage(const char *programName)
{
    fprintf(
        stderr,
        "Usage: %s [--mode auto|benchmark] [--query sum|min|max|point] [--left <index>] [--right <index>]\n",
        programName
    );
}

static bool parseUnsignedLong(const char *text, unsigned long *value)
{
    char *end = NULL;

    if (text == NULL || value == NULL)
    {
        return false;
    }

    errno = 0;
    *value = strtoul(text, &end, 10);

    if (errno != 0 || end == text || *end != '\0')
    {
        return false;
    }

    return true;
}

static bool parseUnsignedIndex(const char *text, DataIndex *value)
{
    unsigned long parsed = 0UL;

    if (!parseUnsignedLong(text, &parsed))
    {
        return false;
    }

    *value = (DataIndex)parsed;
    return true;
}

static StatusCode loadSampleData(
    StepCount values[],
    size_t capacity,
    size_t *count
)
{
    FILE *csvFile = fopen("data/sample_steps.csv", "r");
    char line[256];
    size_t loaded = 0U;

    if (csvFile == NULL)
    {
        csvFile = fopen("../data/sample_steps.csv", "r");
    }

    if (csvFile == NULL)
    {
        fprintf(stderr, "Failed to open sample_steps.csv\n");
        return STATUS_FILE_ERROR;
    }

    if (fgets(line, sizeof(line), csvFile) == NULL)
    {
        fclose(csvFile);
        return STATUS_FILE_ERROR;
    }

    while (fgets(line, sizeof(line), csvFile) != NULL && loaded < capacity)
    {
        char *comma = strchr(line, ',');
        char *end = NULL;
        unsigned long value = 0UL;

        if (comma == NULL)
        {
            continue;
        }

        *comma = '\0';
        end = comma + 1;

        while (*end == ' ' || *end == '\t')
        {
            ++end;
        }

        while (*end == '\r' || *end == '\n')
        {
            *end = '\0';
            ++end;
        }

        errno = 0;
        value = strtoul(end, &end, 10);

        if (errno != 0 || end == comma + 1)
        {
            continue;
        }

        values[loaded++] = (StepCount)value;
    }

    fclose(csvFile);
    *count = loaded;
    return STATUS_SUCCESS;
}

static int executeRangeQuery(const char *queryType, DataIndex left, DataIndex right)
{
    StepCount values[32];
    size_t count = 0U;
    StepCount result = 0U;
    DataIndex index = 0U;
    const bool isPointUpdate = strcmp(queryType, "point") == 0;

    if (loadSampleData(values, sizeof(values) / sizeof(values[0]), &count) != STATUS_SUCCESS)
    {
        fprintf(stderr, "Failed to load sample csv data\n");
        return 2;
    }

    if (count == 0U)
    {
        fprintf(stderr, "Sample csv contains no rows\n");
        return 2;
    }

    if (left == 0U || right == 0U || left > right || right > count)
    {
        fprintf(stderr, "Invalid range: left=%zu right=%zu count=%zu\n", (size_t)left, (size_t)right, count);
        return 2;
    }

    if (strcmp(queryType, "sum") == 0)
    {
        result = 0U;
        for (index = left - 1U; index < right; ++index)
        {
            result += values[index];
        }
    }
    else if (strcmp(queryType, "min") == 0)
    {
        result = values[left - 1U];
        for (index = left; index < right; ++index)
        {
            if (values[index] < result)
            {
                result = values[index];
            }
        }
    }
    else if (strcmp(queryType, "max") == 0)
    {
        result = values[left - 1U];
        for (index = left; index < right; ++index)
        {
            if (values[index] > result)
            {
                result = values[index];
            }
        }
    }
    else if (isPointUpdate)
    {
        if (left != right)
        {
            fprintf(stderr, "Point update requires left == right\n");
            return 2;
        }

        result = values[left - 1U];
    }
    else
    {
        fprintf(stderr, "Unsupported query type: %s\n", queryType);
        return 2;
    }

    if (isPointUpdate)
    {
        printf("RESULT query_type=point left=%zu right=%zu value=%llu\n",
               (size_t)left,
               (size_t)right,
               (unsigned long long)result);
    }
    else
    {
        printf("RESULT query_type=%s left=%zu right=%zu value=%llu\n",
               queryType,
               (size_t)left,
               (size_t)right,
               (unsigned long long)result);
    }

    return 0;
}

static StatusCode executeTestSuite(void)
{
    TestSummary summary = {
        .total = 0U,
        .passed = 0U,
        .failed = 0U
    };

    const StatusCode status = runAllTests(&summary);

    if (status != STATUS_SUCCESS)
    {
        fprintf(
            stderr,
            "\nTest suite failed: %zu/%zu test(s) failed.\n",
            summary.failed,
            summary.total
        );

        return status;
    }

    printf(
        "\nAll %zu tests completed successfully.\n",
        summary.total
    );

    return STATUS_SUCCESS;
}

static StatusCode executeBenchmarkSuite(void)
{
    BenchmarkResult results[
        STANDARD_BENCHMARK_RESULT_COUNT
    ];

    DataIndex resultCount = 0U;

    printf("\nStandard Benchmark Suite\n");
    printf("========================\n");
    printf("Dataset sizes: 1000, 10000, 100000\n");
    printf("Operations: Q = N\n");
    printf("Workload: 70%% Query, 30%% Update\n");
    printf("Runs per algorithm: %zu\n", DEFAULT_BENCHMARK_RUNS);
    printf("Random seed: %u\n", DEFAULT_RANDOM_SEED);
    printf("\nRunning benchmarks. Please wait...\n");

    StatusCode status = runStandardBenchmarkSuite(
        results,
        STANDARD_BENCHMARK_RESULT_COUNT,
        &resultCount
    );

    if (status != STATUS_SUCCESS)
    {
        fprintf(
            stderr,
            "\nFailed to run benchmark suite with status: %d\n",
            status
        );

        return status;
    }

    if (resultCount != STANDARD_BENCHMARK_RESULT_COUNT)
    {
        fprintf(
            stderr,
            "\nUnexpected benchmark result count: %zu\n",
            resultCount
        );

        return STATUS_INVALID_ARGUMENT;
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
            "\nFailed to write benchmark results to %s "
            "with status: %d\n",
            DEFAULT_BENCHMARK_RESULT_FILE,
            status
        );

        return status;
    }

    const BenchmarkConfig environmentConfig =
        createDefaultBenchmarkConfig(
            LARGE_DATASET_SIZE
        );

    status = writeBenchmarkEnvironment(
        DEFAULT_BENCHMARK_ENVIRONMENT_FILE,
        &environmentConfig
    );

    if (status != STATUS_SUCCESS)
    {
        fprintf(
            stderr,
            "\nFailed to write benchmark environment to %s "
            "with status: %d\n",
            DEFAULT_BENCHMARK_ENVIRONMENT_FILE,
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

    printf("\nBenchmark suite completed successfully.\n");

    return STATUS_SUCCESS;
}

static void reportExecutionFailure(
    const char *taskName,
    StatusCode status
)
{
    fprintf(
        stderr,
        "\n%s failed with status: %d\n",
        taskName,
        status
    );
}

int main(int argc, char *argv[])
{
    const char *queryType = "sum";
    const char *mode = "auto";
    DataIndex left = 1U;
    DataIndex right = 1U;
    int argumentIndex = 1;

    if (argc > 1)
    {
        while (argumentIndex < argc)
        {
            if (strcmp(argv[argumentIndex], "--mode") == 0 && argumentIndex + 1 < argc)
            {
                mode = argv[++argumentIndex];
            }
            else if (strcmp(argv[argumentIndex], "--query") == 0 && argumentIndex + 1 < argc)
            {
                queryType = argv[++argumentIndex];
            }
            else if (strcmp(argv[argumentIndex], "--left") == 0 && argumentIndex + 1 < argc)
            {
                if (!parseUnsignedIndex(argv[++argumentIndex], &left))
                {
                    fprintf(stderr, "Invalid left value: %s\n", argv[argumentIndex]);
                    printUsage(argv[0]);
                    return 2;
                }
            }
            else if (strcmp(argv[argumentIndex], "--right") == 0 && argumentIndex + 1 < argc)
            {
                if (!parseUnsignedIndex(argv[++argumentIndex], &right))
                {
                    fprintf(stderr, "Invalid right value: %s\n", argv[argumentIndex]);
                    printUsage(argv[0]);
                    return 2;
                }
            }
            else if (strcmp(argv[argumentIndex], "--help") == 0 || strcmp(argv[argumentIndex], "-h") == 0)
            {
                printUsage(argv[0]);
                return 0;
            }
            else
            {
                fprintf(stderr, "Unknown argument: %s\n", argv[argumentIndex]);
                printUsage(argv[0]);
                return 2;
            }

            ++argumentIndex;
        }

        if (strcmp(mode, "benchmark") == 0)
        {
            return executeBenchmarkSuite() == STATUS_SUCCESS ? 0 : 1;
        }

        if (strcmp(mode, "auto") != 0 && strcmp(mode, "query") != 0)
        {
            fprintf(stderr, "Unsupported mode: %s\n", mode);
            printUsage(argv[0]);
            return 2;
        }

        return executeRangeQuery(queryType, left, right);
    }

    {
        bool running = true;
        int exitCode = 0;

        printf("Dynamic Range Query Sensor Benchmark\n");
        printf("====================================\n");

        while (running)
        {
            unsigned int commandValue = 0U;

            displayMainMenu();

            if (scanf("%u", &commandValue) != 1)
            {
                printf(
                    "Invalid input. Please enter a number from 1 to 5.\n"
                );

                clearInputBuffer();
                continue;
            }

            clearInputBuffer();

            StatusCode status = STATUS_SUCCESS;

            switch ((MainCommand)commandValue)
            {
                case MAIN_COMMAND_AUTOMATIC_DEMO:
                    status = runAutomaticDemo();
                    destroyDemo();

                    if (status != STATUS_SUCCESS)
                    {
                        reportExecutionFailure(
                            "Automatic demo",
                            status
                        );

                        exitCode = 1;
                    }

                    break;

                case MAIN_COMMAND_INTERACTIVE_DEMO:
                    status = runInteractiveDemo();
                    destroyDemo();

                    if (status != STATUS_SUCCESS)
                    {
                        reportExecutionFailure(
                            "Interactive demo",
                            status
                        );

                        exitCode = 1;
                    }

                    break;

                case MAIN_COMMAND_TEST_SUITE:
                    status = executeTestSuite();

                    if (status != STATUS_SUCCESS)
                    {
                        reportExecutionFailure(
                            "Test suite",
                            status
                        );

                        exitCode = 1;
                    }

                    break;

                case MAIN_COMMAND_BENCHMARK:
                    status = executeBenchmarkSuite();

                    if (status != STATUS_SUCCESS)
                    {
                        reportExecutionFailure(
                            "Benchmark suite",
                            status
                        );

                        exitCode = 1;
                    }

                    break;

                case MAIN_COMMAND_EXIT:
                    running = false;
                    break;

                default:
                    printf(
                        "Invalid command. Please select from 1 to 5.\n"
                    );
                    break;
            }
        }

        destroyDemo();

        printf("\nSensor Benchmark terminated successfully.\n");

        return exitCode;
    }
}