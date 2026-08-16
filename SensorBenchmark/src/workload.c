#include "workload.h"

#include <stdlib.h>
#include <string.h>

static uint32_t nextRandom32(uint32_t *state)
{
    uint32_t value = *state;

    if (value == 0U)
    {
        value = UINT32_C(0xA341316C);
    }

    value ^= value << 13U;
    value ^= value >> 17U;
    value ^= value << 5U;

    *state = value;

    return value;
}

static uint64_t nextRandom64(uint32_t *state)
{
    const uint64_t high = (uint64_t)nextRandom32(state);
    const uint64_t low = (uint64_t)nextRandom32(state);

    return (high << 32U) | low;
}

static uint64_t randomUint64InRange(
    uint32_t *state,
    uint64_t minimum,
    uint64_t maximum
)
{
    if (minimum == 0U && maximum == UINT64_MAX)
    {
        return nextRandom64(state);
    }

    const uint64_t range = maximum - minimum + 1U;
    const uint64_t rejectionLimit = (UINT64_MAX - range + 1U) % range;

    uint64_t randomValue;

    do
    {
        randomValue = nextRandom64(state);
    }
    while (randomValue < rejectionLimit);

    return minimum + (randomValue % range);
}

static DataIndex randomIndex(
    uint32_t *state,
    DataIndex size
)
{
    return (DataIndex)randomUint64InRange(
        state,
        0U,
        (uint64_t)(size - 1U)
    );
}

static OperationType randomQueryType(uint32_t *state)
{
    const uint32_t queryType = nextRandom32(state) % 3U;

    switch (queryType)
    {
        case 0U:
            return OPERATION_SUM;

        case 1U:
            return OPERATION_MIN;

        default:
            return OPERATION_MAX;
    }
}

static void swapOperations(
    Operation *left,
    Operation *right
)
{
    const Operation temporary = *left;
    *left = *right;
    *right = temporary;
}

static void shuffleOperations(
    Operation operations[],
    DataIndex count,
    uint32_t *state
)
{
    if (count < 2U)
    {
        return;
    }

    for (DataIndex index = count - 1U; index > 0U; --index)
    {
        const DataIndex selectedIndex = (DataIndex)randomUint64InRange(
            state,
            0U,
            (uint64_t)index
        );

        swapOperations(
            &operations[index],
            &operations[selectedIndex]
        );
    }
}

StatusCode generateDataset(
    StepCount data[],
    DataIndex size,
    StepCount minimum,
    StepCount maximum,
    uint32_t seed
)
{
    if (data == NULL || size == 0U || minimum > maximum)
    {
        return STATUS_INVALID_ARGUMENT;
    }

    uint32_t randomState = seed;

    for (DataIndex index = 0U; index < size; ++index)
    {
        data[index] = randomUint64InRange(
            &randomState,
            minimum,
            maximum
        );
    }

    return STATUS_SUCCESS;
}

StatusCode generateWorkload(
    Workload *workload,
    DataIndex dataSize,
    DataIndex operationCount,
    unsigned int queryPercentage,
    unsigned int updatePercentage,
    StepCount minimumUpdateValue,
    StepCount maximumUpdateValue,
    uint32_t seed
)
{
    if (
        workload == NULL
        || dataSize == 0U
        || operationCount == 0U
        || queryPercentage > 100U
        || updatePercentage > 100U
        || queryPercentage + updatePercentage != 100U
        || minimumUpdateValue > maximumUpdateValue
    )
    {
        return STATUS_INVALID_ARGUMENT;
    }

    if (operationCount > SIZE_MAX / sizeof(Operation))
    {
        return STATUS_ALLOCATION_FAILED;
    }

    workload->operations = NULL;
    workload->count = 0U;
    workload->queryCount = 0U;
    workload->updateCount = 0U;
    workload->seed = seed;

    Operation *operations = malloc(
        operationCount * sizeof(*operations)
    );

    if (operations == NULL)
    {
        return STATUS_ALLOCATION_FAILED;
    }

    const DataIndex queryCount =
        (operationCount / 100U) * queryPercentage
        + ((operationCount % 100U) * queryPercentage) / 100U;

    const DataIndex updateCount = operationCount - queryCount;

    uint32_t randomState = seed;

    for (DataIndex index = 0U; index < queryCount; ++index)
    {
        DataIndex left = randomIndex(
            &randomState,
            dataSize
        );

        DataIndex right = randomIndex(
            &randomState,
            dataSize
        );

        if (left > right)
        {
            const DataIndex temporary = left;
            left = right;
            right = temporary;
        }

        operations[index] = (Operation){
            .type = randomQueryType(&randomState),
            .left = left,
            .right = right,
            .value = 0U
        };
    }

    for (
        DataIndex index = queryCount;
        index < operationCount;
        ++index
    )
    {
        operations[index] = (Operation){
            .type = OPERATION_UPDATE,
            .left = randomIndex(&randomState, dataSize),
            .right = 0U,
            .value = randomUint64InRange(
                &randomState,
                minimumUpdateValue,
                maximumUpdateValue
            )
        };
    }

    shuffleOperations(
        operations,
        operationCount,
        &randomState
    );

    workload->operations = operations;
    workload->count = operationCount;
    workload->queryCount = queryCount;
    workload->updateCount = updateCount;

    return STATUS_SUCCESS;
}

StatusCode generateDefaultWorkload(
    Workload *workload,
    DataIndex dataSize,
    uint32_t seed
)
{
    return generateWorkload(
        workload,
        dataSize,
        dataSize,
        QUERY_PERCENTAGE,
        UPDATE_PERCENTAGE,
        DEFAULT_MIN_STEP_COUNT,
        DEFAULT_MAX_STEP_COUNT,
        seed
    );
}

StatusCode cloneWorkload(
    const Workload *source,
    Workload *destination
)
{
    if (
        source == NULL
        || destination == NULL
        || source == destination
        || source->operations == NULL
        || source->count == 0U
    )
    {
        return STATUS_INVALID_ARGUMENT;
    }

    if (source->count > SIZE_MAX / sizeof(Operation))
    {
        return STATUS_ALLOCATION_FAILED;
    }

    destination->operations = NULL;
    destination->count = 0U;
    destination->queryCount = 0U;
    destination->updateCount = 0U;
    destination->seed = source->seed;

    destination->operations = malloc(
        source->count * sizeof(*destination->operations)
    );

    if (destination->operations == NULL)
    {
        return STATUS_ALLOCATION_FAILED;
    }

    memcpy(
        destination->operations,
        source->operations,
        source->count * sizeof(*source->operations)
    );

    destination->count = source->count;
    destination->queryCount = source->queryCount;
    destination->updateCount = source->updateCount;
    destination->seed = source->seed;

    return STATUS_SUCCESS;
}

void destroyWorkload(Workload *workload)
{
    if (workload == NULL)
    {
        return;
    }

    free(workload->operations);

    workload->operations = NULL;
    workload->count = 0U;
    workload->queryCount = 0U;
    workload->updateCount = 0U;
    workload->seed = 0U;
}