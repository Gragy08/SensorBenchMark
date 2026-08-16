#include "raw_array.h"

StatusCode arrayRangeSum(
    const StepCount data[],
    DataIndex size,
    DataIndex left,
    DataIndex right,
    StepCount *result
)
{
    if (data == NULL || result == NULL)
    {
        return STATUS_INVALID_ARGUMENT;
    }

    if (!isValidRange(left, right, size))
    {
        return STATUS_INDEX_OUT_OF_RANGE;
    }

    StepCount sum = 0U;

    for (DataIndex index = left; index <= right; ++index)
    {
        sum += data[index];
    }

    *result = sum;

    return STATUS_SUCCESS;
}

StatusCode arrayRangeMin(
    const StepCount data[],
    DataIndex size,
    DataIndex left,
    DataIndex right,
    StepCount *result
)
{
    if (data == NULL || result == NULL)
    {
        return STATUS_INVALID_ARGUMENT;
    }

    if (!isValidRange(left, right, size))
    {
        return STATUS_INDEX_OUT_OF_RANGE;
    }

    StepCount minimum = data[left];

    for (DataIndex index = left + 1U; index <= right; ++index)
    {
        if (data[index] < minimum)
        {
            minimum = data[index];
        }
    }

    *result = minimum;

    return STATUS_SUCCESS;
}

StatusCode arrayRangeMax(
    const StepCount data[],
    DataIndex size,
    DataIndex left,
    DataIndex right,
    StepCount *result
)
{
    if (data == NULL || result == NULL)
    {
        return STATUS_INVALID_ARGUMENT;
    }

    if (!isValidRange(left, right, size))
    {
        return STATUS_INDEX_OUT_OF_RANGE;
    }

    StepCount maximum = data[left];

    for (DataIndex index = left + 1U; index <= right; ++index)
    {
        if (data[index] > maximum)
        {
            maximum = data[index];
        }
    }

    *result = maximum;

    return STATUS_SUCCESS;
}

StatusCode arrayUpdate(
    StepCount data[],
    DataIndex size,
    DataIndex index,
    StepCount newValue
)
{
    if (data == NULL)
    {
        return STATUS_INVALID_ARGUMENT;
    }

    if (!isValidIndex(index, size))
    {
        return STATUS_INDEX_OUT_OF_RANGE;
    }

    data[index] = newValue;

    return STATUS_SUCCESS;
}