#include "prefix_sum.h"

StatusCode buildPrefixSum(
    const StepCount data[],
    DataIndex size,
    StepCount prefix[]
)
{
    if (data == NULL || prefix == NULL)
    {
        return STATUS_INVALID_ARGUMENT;
    }

    if (size == 0U)
    {
        return STATUS_INVALID_ARGUMENT;
    }

    prefix[0] = data[0];

    for (DataIndex index = 1U; index < size; ++index)
    {
        prefix[index] = prefix[index - 1U] + data[index];
    }

    return STATUS_SUCCESS;
}

StatusCode prefixRangeSum(
    const StepCount prefix[],
    DataIndex size,
    DataIndex left,
    DataIndex right,
    StepCount *result
)
{
    if (prefix == NULL || result == NULL)
    {
        return STATUS_INVALID_ARGUMENT;
    }

    if (!isValidRange(left, right, size))
    {
        return STATUS_INDEX_OUT_OF_RANGE;
    }

    if (left == 0U)
    {
        *result = prefix[right];
    }
    else
    {
        *result = prefix[right] - prefix[left - 1U];
    }

    return STATUS_SUCCESS;
}

StatusCode prefixUpdate(
    StepCount data[],
    StepCount prefix[],
    DataIndex size,
    DataIndex index,
    StepCount newValue
)
{
    if (data == NULL || prefix == NULL)
    {
        return STATUS_INVALID_ARGUMENT;
    }

    if (!isValidIndex(index, size))
    {
        return STATUS_INDEX_OUT_OF_RANGE;
    }

    const StepCount oldValue = data[index];

    if (newValue >= oldValue)
    {
        const StepCount difference = newValue - oldValue;

        for (DataIndex current = index; current < size; ++current)
        {
            prefix[current] += difference;
        }
    }
    else
    {
        const StepCount difference = oldValue - newValue;

        for (DataIndex current = index; current < size; ++current)
        {
            prefix[current] -= difference;
        }
    }

    data[index] = newValue;

    return STATUS_SUCCESS;
}