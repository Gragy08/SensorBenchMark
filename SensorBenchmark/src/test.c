#include "test.h"

#include <inttypes.h>
#include <stdio.h>

#include "fenwick_tree.h"
#include "prefix_sum.h"
#include "raw_array.h"
#include "segment_tree.h"

#define EXPECTED_STATUS_TEST_FAILED ((StatusCode)5)

static const StepCount SAMPLE_STEPS[SAMPLE_DATA_SIZE] = {
    UINT64_C(6000),
    UINT64_C(8500),
    UINT64_C(7200),
    UINT64_C(9000),
    UINT64_C(5500),
    UINT64_C(12000),
    UINT64_C(10000)
};

static void recordTest(
    TestSummary *summary,
    const char *testName,
    bool condition
)
{
    ++summary->total;

    if (condition)
    {
        ++summary->passed;
        printf("[PASSED] %s\n", testName);
    }
    else
    {
        ++summary->failed;
        printf("[FAILED] %s\n", testName);
    }
}

static void mergeTestSummary(
    TestSummary *destination,
    TestSummary source
)
{
    destination->total += source.total;
    destination->passed += source.passed;
    destination->failed += source.failed;
}

static void copySampleData(
    StepCount destination[SAMPLE_DATA_SIZE]
)
{
    for (DataIndex index = 0U; index < SAMPLE_DATA_SIZE; ++index)
    {
        destination[index] = SAMPLE_STEPS[index];
    }
}

TestSummary runArrayTests(void)
{
    TestSummary summary = {0U, 0U, 0U};
    StepCount data[SAMPLE_DATA_SIZE];

    copySampleData(data);

    StepCount result = 0U;
    StatusCode status;

    printf("\nArray tests\n");
    printf("===========\n");

    status = arrayRangeSum(
        data,
        SAMPLE_DATA_SIZE,
        1U,
        5U,
        &result
    );

    recordTest(
        &summary,
        "Array SUM 2 6 equals 42200",
        status == STATUS_SUCCESS
            && result == UINT64_C(42200)
    );

    status = arrayRangeSum(
        data,
        SAMPLE_DATA_SIZE,
        0U,
        6U,
        &result
    );

    recordTest(
        &summary,
        "Array SUM 1 7 equals 58200",
        status == STATUS_SUCCESS
            && result == UINT64_C(58200)
    );

    status = arrayRangeSum(
        data,
        SAMPLE_DATA_SIZE,
        2U,
        2U,
        &result
    );

    recordTest(
        &summary,
        "Array SUM single element equals 7200",
        status == STATUS_SUCCESS
            && result == UINT64_C(7200)
    );

    status = arrayRangeMin(
        data,
        SAMPLE_DATA_SIZE,
        0U,
        6U,
        &result
    );

    recordTest(
        &summary,
        "Array MIN 1 7 equals 5500",
        status == STATUS_SUCCESS
            && result == UINT64_C(5500)
    );

    status = arrayRangeMax(
        data,
        SAMPLE_DATA_SIZE,
        2U,
        6U,
        &result
    );

    recordTest(
        &summary,
        "Array MAX 3 7 equals 12000",
        status == STATUS_SUCCESS
            && result == UINT64_C(12000)
    );

    status = arrayUpdate(
        data,
        SAMPLE_DATA_SIZE,
        2U,
        UINT64_C(10000)
    );

    recordTest(
        &summary,
        "Array UPDATE 3 10000 succeeds",
        status == STATUS_SUCCESS
            && data[2] == UINT64_C(10000)
    );

    status = arrayRangeSum(
        data,
        SAMPLE_DATA_SIZE,
        1U,
        5U,
        &result
    );

    recordTest(
        &summary,
        "Array SUM 2 6 after update equals 45000",
        status == STATUS_SUCCESS
            && result == UINT64_C(45000)
    );

    status = arrayRangeSum(
        data,
        SAMPLE_DATA_SIZE,
        5U,
        1U,
        &result
    );

    recordTest(
        &summary,
        "Array rejects range with left greater than right",
        status == STATUS_INDEX_OUT_OF_RANGE
    );

    status = arrayRangeSum(
        data,
        SAMPLE_DATA_SIZE,
        0U,
        SAMPLE_DATA_SIZE,
        &result
    );

    recordTest(
        &summary,
        "Array rejects index outside data range",
        status == STATUS_INDEX_OUT_OF_RANGE
    );

    status = arrayRangeSum(
        NULL,
        SAMPLE_DATA_SIZE,
        0U,
        1U,
        &result
    );

    recordTest(
        &summary,
        "Array rejects NULL data",
        status == STATUS_INVALID_ARGUMENT
    );

    status = arrayRangeSum(
        data,
        SAMPLE_DATA_SIZE,
        0U,
        1U,
        NULL
    );

    recordTest(
        &summary,
        "Array rejects NULL result",
        status == STATUS_INVALID_ARGUMENT
    );

    return summary;
}

TestSummary runPrefixSumTests(void)
{
    TestSummary summary = {0U, 0U, 0U};
    StepCount data[SAMPLE_DATA_SIZE];
    StepCount prefix[SAMPLE_DATA_SIZE] = {0U};

    copySampleData(data);

    printf("\nPrefix Sum tests\n");
    printf("================\n");

    StatusCode status = buildPrefixSum(
        data,
        SAMPLE_DATA_SIZE,
        prefix
    );

    recordTest(
        &summary,
        "Prefix Sum build succeeds",
        status == STATUS_SUCCESS
    );

    const StepCount expectedPrefix[SAMPLE_DATA_SIZE] = {
        UINT64_C(6000),
        UINT64_C(14500),
        UINT64_C(21700),
        UINT64_C(30700),
        UINT64_C(36200),
        UINT64_C(48200),
        UINT64_C(58200)
    };

    bool prefixIsCorrect = true;

    for (DataIndex index = 0U; index < SAMPLE_DATA_SIZE; ++index)
    {
        if (prefix[index] != expectedPrefix[index])
        {
            prefixIsCorrect = false;
            break;
        }
    }

    recordTest(
        &summary,
        "Prefix Sum values are correct after build",
        prefixIsCorrect
    );

    StepCount result = 0U;

    status = prefixRangeSum(
        prefix,
        SAMPLE_DATA_SIZE,
        1U,
        5U,
        &result
    );

    recordTest(
        &summary,
        "Prefix SUM 2 6 equals 42200",
        status == STATUS_SUCCESS
            && result == UINT64_C(42200)
    );

    status = prefixRangeSum(
        prefix,
        SAMPLE_DATA_SIZE,
        0U,
        6U,
        &result
    );

    recordTest(
        &summary,
        "Prefix SUM 1 7 equals 58200",
        status == STATUS_SUCCESS
            && result == UINT64_C(58200)
    );

    status = prefixUpdate(
        data,
        prefix,
        SAMPLE_DATA_SIZE,
        2U,
        UINT64_C(10000)
    );

    recordTest(
        &summary,
        "Prefix UPDATE 3 10000 succeeds",
        status == STATUS_SUCCESS
            && data[2] == UINT64_C(10000)
    );

    status = prefixRangeSum(
        prefix,
        SAMPLE_DATA_SIZE,
        1U,
        5U,
        &result
    );

    recordTest(
        &summary,
        "Prefix SUM 2 6 after update equals 45000",
        status == STATUS_SUCCESS
            && result == UINT64_C(45000)
    );

    status = prefixUpdate(
        data,
        prefix,
        SAMPLE_DATA_SIZE,
        2U,
        UINT64_C(5000)
    );

    recordTest(
        &summary,
        "Prefix supports update to a smaller value",
        status == STATUS_SUCCESS
            && data[2] == UINT64_C(5000)
    );

    status = prefixRangeSum(
        prefix,
        SAMPLE_DATA_SIZE,
        0U,
        6U,
        &result
    );

    recordTest(
        &summary,
        "Prefix total after smaller update equals 56000",
        status == STATUS_SUCCESS
            && result == UINT64_C(56000)
    );

    status = buildPrefixSum(
        NULL,
        SAMPLE_DATA_SIZE,
        prefix
    );

    recordTest(
        &summary,
        "Prefix build rejects NULL data",
        status == STATUS_INVALID_ARGUMENT
    );

    status = prefixRangeSum(
        prefix,
        SAMPLE_DATA_SIZE,
        4U,
        2U,
        &result
    );

    recordTest(
        &summary,
        "Prefix rejects invalid range",
        status == STATUS_INDEX_OUT_OF_RANGE
    );

    return summary;
}

TestSummary runFenwickTreeTests(void)
{
    TestSummary summary = {0U, 0U, 0U};
    StepCount data[SAMPLE_DATA_SIZE];

    copySampleData(data);

    FenwickTree fenwickTree = {
        .tree = NULL,
        .data = NULL,
        .size = 0U
    };

    printf("\nBinary Indexed Tree tests\n");
    printf("=========================\n");

    StatusCode status = bitBuild(
        &fenwickTree,
        data,
        SAMPLE_DATA_SIZE
    );

    recordTest(
        &summary,
        "BIT build succeeds",
        status == STATUS_SUCCESS
    );

    if (status != STATUS_SUCCESS)
    {
        return summary;
    }

    StepCount result = 0U;

    status = bitPrefixSum(
        &fenwickTree,
        5U,
        &result
    );

    recordTest(
        &summary,
        "BIT prefix sum through day 6 equals 48200",
        status == STATUS_SUCCESS
            && result == UINT64_C(48200)
    );

    status = bitRangeSum(
        &fenwickTree,
        1U,
        5U,
        &result
    );

    recordTest(
        &summary,
        "BIT SUM 2 6 equals 42200",
        status == STATUS_SUCCESS
            && result == UINT64_C(42200)
    );

    status = bitRangeSum(
        &fenwickTree,
        0U,
        6U,
        &result
    );

    recordTest(
        &summary,
        "BIT SUM 1 7 equals 58200",
        status == STATUS_SUCCESS
            && result == UINT64_C(58200)
    );

    status = bitUpdate(
        &fenwickTree,
        2U,
        UINT64_C(10000)
    );

    recordTest(
        &summary,
        "BIT UPDATE 3 10000 succeeds",
        status == STATUS_SUCCESS
            && fenwickTree.data[2] == UINT64_C(10000)
    );

    status = bitRangeSum(
        &fenwickTree,
        1U,
        5U,
        &result
    );

    recordTest(
        &summary,
        "BIT SUM 2 6 after update equals 45000",
        status == STATUS_SUCCESS
            && result == UINT64_C(45000)
    );

    status = bitUpdate(
        &fenwickTree,
        2U,
        UINT64_C(5000)
    );

    recordTest(
        &summary,
        "BIT supports update to a smaller value",
        status == STATUS_SUCCESS
            && fenwickTree.data[2] == UINT64_C(5000)
    );

    status = bitRangeSum(
        &fenwickTree,
        0U,
        6U,
        &result
    );

    recordTest(
        &summary,
        "BIT total after smaller update equals 56000",
        status == STATUS_SUCCESS
            && result == UINT64_C(56000)
    );

    status = bitRangeSum(
        &fenwickTree,
        5U,
        1U,
        &result
    );

    recordTest(
        &summary,
        "BIT rejects invalid range",
        status == STATUS_INDEX_OUT_OF_RANGE
    );

    status = bitUpdate(
        &fenwickTree,
        SAMPLE_DATA_SIZE,
        UINT64_C(1000)
    );

    recordTest(
        &summary,
        "BIT rejects update outside data range",
        status == STATUS_INDEX_OUT_OF_RANGE
    );

    bitDestroy(&fenwickTree);

    recordTest(
        &summary,
        "BIT destroy resets structure",
        fenwickTree.tree == NULL
            && fenwickTree.data == NULL
            && fenwickTree.size == 0U
    );

    return summary;
}

TestSummary runSegmentTreeTests(void)
{
    TestSummary summary = {0U, 0U, 0U};
    StepCount data[SAMPLE_DATA_SIZE];

    copySampleData(data);

    SegmentTree segmentTree = {
        .tree = NULL,
        .data = NULL,
        .size = 0U,
        .capacity = 0U
    };

    printf("\nSegment Tree tests\n");
    printf("==================\n");

    StatusCode status = buildSegmentTree(
        &segmentTree,
        data,
        SAMPLE_DATA_SIZE
    );

    recordTest(
        &summary,
        "Segment Tree build succeeds",
        status == STATUS_SUCCESS
    );

    if (status != STATUS_SUCCESS)
    {
        return summary;
    }

    SegmentNode result = {0U, 0U, 0U};

    status = querySegmentTree(
        &segmentTree,
        1U,
        5U,
        &result
    );

    recordTest(
        &summary,
        "Segment Tree SUM 2 6 equals 42200",
        status == STATUS_SUCCESS
            && result.sum == UINT64_C(42200)
    );

    status = querySegmentTree(
        &segmentTree,
        0U,
        6U,
        &result
    );

    recordTest(
        &summary,
        "Segment Tree MIN 1 7 equals 5500",
        status == STATUS_SUCCESS
            && result.minimum == UINT64_C(5500)
    );

    recordTest(
        &summary,
        "Segment Tree MAX 1 7 equals 12000",
        status == STATUS_SUCCESS
            && result.maximum == UINT64_C(12000)
    );

    status = querySegmentTree(
        &segmentTree,
        2U,
        2U,
        &result
    );

    recordTest(
        &summary,
        "Segment Tree single element query is correct",
        status == STATUS_SUCCESS
            && result.sum == UINT64_C(7200)
            && result.minimum == UINT64_C(7200)
            && result.maximum == UINT64_C(7200)
    );

    status = updateSegmentTree(
        &segmentTree,
        2U,
        UINT64_C(10000)
    );

    recordTest(
        &summary,
        "Segment Tree UPDATE 3 10000 succeeds",
        status == STATUS_SUCCESS
            && segmentTree.data[2] == UINT64_C(10000)
    );

    status = querySegmentTree(
        &segmentTree,
        1U,
        5U,
        &result
    );

    recordTest(
        &summary,
        "Segment Tree SUM 2 6 after update equals 45000",
        status == STATUS_SUCCESS
            && result.sum == UINT64_C(45000)
    );

    status = querySegmentTree(
        &segmentTree,
        2U,
        6U,
        &result
    );

    recordTest(
        &summary,
        "Segment Tree MIN 3 7 after update equals 5500",
        status == STATUS_SUCCESS
            && result.minimum == UINT64_C(5500)
    );

    recordTest(
        &summary,
        "Segment Tree MAX 3 7 after update equals 12000",
        status == STATUS_SUCCESS
            && result.maximum == UINT64_C(12000)
    );

    status = querySegmentTree(
        &segmentTree,
        6U,
        2U,
        &result
    );

    recordTest(
        &summary,
        "Segment Tree rejects invalid range",
        status == STATUS_INDEX_OUT_OF_RANGE
    );

    status = updateSegmentTree(
        &segmentTree,
        SAMPLE_DATA_SIZE,
        UINT64_C(1000)
    );

    recordTest(
        &summary,
        "Segment Tree rejects update outside data range",
        status == STATUS_INDEX_OUT_OF_RANGE
    );

    destroySegmentTree(&segmentTree);

    recordTest(
        &summary,
        "Segment Tree destroy resets structure",
        segmentTree.tree == NULL
            && segmentTree.data == NULL
            && segmentTree.size == 0U
            && segmentTree.capacity == 0U
    );

    return summary;
}

TestSummary runCrossValidationTests(void)
{
    TestSummary summary = {0U, 0U, 0U};
    StepCount data[SAMPLE_DATA_SIZE];
    StepCount prefix[SAMPLE_DATA_SIZE] = {0U};

    copySampleData(data);

    FenwickTree fenwickTree = {
        .tree = NULL,
        .data = NULL,
        .size = 0U
    };

    SegmentTree segmentTree = {
        .tree = NULL,
        .data = NULL,
        .size = 0U,
        .capacity = 0U
    };

    printf("\nCross-validation tests\n");
    printf("======================\n");

    StatusCode prefixStatus = buildPrefixSum(
        data,
        SAMPLE_DATA_SIZE,
        prefix
    );

    StatusCode bitStatus = bitBuild(
        &fenwickTree,
        data,
        SAMPLE_DATA_SIZE
    );

    StatusCode segmentStatus = buildSegmentTree(
        &segmentTree,
        data,
        SAMPLE_DATA_SIZE
    );

    bool buildSucceeded =
        prefixStatus == STATUS_SUCCESS
        && bitStatus == STATUS_SUCCESS
        && segmentStatus == STATUS_SUCCESS;

    recordTest(
        &summary,
        "All structures build successfully",
        buildSucceeded
    );

    if (!buildSucceeded)
    {
        bitDestroy(&fenwickTree);
        destroySegmentTree(&segmentTree);
        return summary;
    }

    bool allRangesMatch = true;

    for (
        DataIndex left = 0U;
        left < SAMPLE_DATA_SIZE && allRangesMatch;
        ++left
    )
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

            StatusCode arraySumStatus = arrayRangeSum(
                data,
                SAMPLE_DATA_SIZE,
                left,
                right,
                &arraySum
            );

            StatusCode arrayMinStatus = arrayRangeMin(
                data,
                SAMPLE_DATA_SIZE,
                left,
                right,
                &arrayMinimum
            );

            StatusCode arrayMaxStatus = arrayRangeMax(
                data,
                SAMPLE_DATA_SIZE,
                left,
                right,
                &arrayMaximum
            );

            prefixStatus = prefixRangeSum(
                prefix,
                SAMPLE_DATA_SIZE,
                left,
                right,
                &prefixSum
            );

            bitStatus = bitRangeSum(
                &fenwickTree,
                left,
                right,
                &bitSum
            );

            segmentStatus = querySegmentTree(
                &segmentTree,
                left,
                right,
                &segmentResult
            );

            if (
                arraySumStatus != STATUS_SUCCESS
                || arrayMinStatus != STATUS_SUCCESS
                || arrayMaxStatus != STATUS_SUCCESS
                || prefixStatus != STATUS_SUCCESS
                || bitStatus != STATUS_SUCCESS
                || segmentStatus != STATUS_SUCCESS
                || arraySum != prefixSum
                || arraySum != bitSum
                || arraySum != segmentResult.sum
                || arrayMinimum != segmentResult.minimum
                || arrayMaximum != segmentResult.maximum
            )
            {
                allRangesMatch = false;
                break;
            }
        }
    }

    recordTest(
        &summary,
        "All query ranges match before updates",
        allRangesMatch
    );

    const DataIndex updateIndexes[] = {
        2U,
        0U,
        6U,
        4U
    };

    const StepCount updateValues[] = {
        UINT64_C(10000),
        UINT64_C(7500),
        UINT64_C(8000),
        UINT64_C(15000)
    };

    const DataIndex updateCount =
        sizeof(updateIndexes) / sizeof(updateIndexes[0]);

    bool updatesSucceeded = true;

    for (
        DataIndex update = 0U;
        update < updateCount;
        ++update
    )
    {
        const DataIndex index = updateIndexes[update];
        const StepCount newValue = updateValues[update];

        prefixStatus = prefixUpdate(
            data,
            prefix,
            SAMPLE_DATA_SIZE,
            index,
            newValue
        );

        bitStatus = bitUpdate(
            &fenwickTree,
            index,
            newValue
        );

        segmentStatus = updateSegmentTree(
            &segmentTree,
            index,
            newValue
        );

        if (
            prefixStatus != STATUS_SUCCESS
            || bitStatus != STATUS_SUCCESS
            || segmentStatus != STATUS_SUCCESS
        )
        {
            updatesSucceeded = false;
            break;
        }
    }

    recordTest(
        &summary,
        "All structures accept the same updates",
        updatesSucceeded
    );

    allRangesMatch = updatesSucceeded;

    for (
        DataIndex left = 0U;
        left < SAMPLE_DATA_SIZE && allRangesMatch;
        ++left
    )
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

            arrayRangeSum(
                data,
                SAMPLE_DATA_SIZE,
                left,
                right,
                &arraySum
            );

            arrayRangeMin(
                data,
                SAMPLE_DATA_SIZE,
                left,
                right,
                &arrayMinimum
            );

            arrayRangeMax(
                data,
                SAMPLE_DATA_SIZE,
                left,
                right,
                &arrayMaximum
            );

            prefixRangeSum(
                prefix,
                SAMPLE_DATA_SIZE,
                left,
                right,
                &prefixSum
            );

            bitRangeSum(
                &fenwickTree,
                left,
                right,
                &bitSum
            );

            querySegmentTree(
                &segmentTree,
                left,
                right,
                &segmentResult
            );

            if (
                arraySum != prefixSum
                || arraySum != bitSum
                || arraySum != segmentResult.sum
                || arrayMinimum != segmentResult.minimum
                || arrayMaximum != segmentResult.maximum
            )
            {
                allRangesMatch = false;
                break;
            }
        }
    }

    recordTest(
        &summary,
        "All query ranges match after updates",
        allRangesMatch
    );

    bitDestroy(&fenwickTree);
    destroySegmentTree(&segmentTree);

    return summary;
}

StatusCode runAllTests(TestSummary *summary)
{
    if (summary == NULL)
    {
        return STATUS_INVALID_ARGUMENT;
    }

    *summary = (TestSummary){0U, 0U, 0U};

    printf("Sensor Benchmark Test Suite\n");
    printf("===========================\n");

    mergeTestSummary(summary, runArrayTests());
    mergeTestSummary(summary, runPrefixSumTests());
    mergeTestSummary(summary, runFenwickTreeTests());
    mergeTestSummary(summary, runSegmentTreeTests());
    mergeTestSummary(summary, runCrossValidationTests());

    printf("\nTest summary\n");
    printf("============\n");
    printf("Total:  %zu\n", summary->total);
    printf("Passed: %zu\n", summary->passed);
    printf("Failed: %zu\n", summary->failed);

    if (summary->failed > 0U)
    {
        return EXPECTED_STATUS_TEST_FAILED;
    }

    return STATUS_SUCCESS;
}