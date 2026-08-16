#include "fenwick_tree.h"

#include <stdlib.h>

static DataIndex lowbit(DataIndex index)
{
    return index & (~index + 1U);
}

static bool isValidFenwickTree(const FenwickTree *fenwickTree)
{
    return fenwickTree != NULL
        && fenwickTree->tree != NULL
        && fenwickTree->data != NULL
        && fenwickTree->size > 0U;
}

StatusCode bitBuild(
    FenwickTree *fenwickTree,
    const StepCount data[],
    DataIndex size
)
{
    if (fenwickTree == NULL || data == NULL || size == 0U)
    {
        return STATUS_INVALID_ARGUMENT;
    }

    fenwickTree->tree = NULL;
    fenwickTree->data = NULL;
    fenwickTree->size = 0U;

    fenwickTree->tree = calloc(
        size + 1U,
        sizeof(*fenwickTree->tree)
    );

    if (fenwickTree->tree == NULL)
    {
        return STATUS_ALLOCATION_FAILED;
    }

    fenwickTree->data = calloc(
        size,
        sizeof(*fenwickTree->data)
    );

    if (fenwickTree->data == NULL)
    {
        free(fenwickTree->tree);
        fenwickTree->tree = NULL;

        return STATUS_ALLOCATION_FAILED;
    }

    fenwickTree->size = size;

    for (DataIndex index = 0U; index < size; ++index)
    {
        StatusCode status = bitAdd(
            fenwickTree,
            index,
            data[index]
        );

        if (status != STATUS_SUCCESS)
        {
            bitDestroy(fenwickTree);
            return status;
        }
    }

    return STATUS_SUCCESS;
}

void bitDestroy(FenwickTree *fenwickTree)
{
    if (fenwickTree == NULL)
    {
        return;
    }

    free(fenwickTree->tree);
    free(fenwickTree->data);

    fenwickTree->tree = NULL;
    fenwickTree->data = NULL;
    fenwickTree->size = 0U;
}

StatusCode bitAdd(
    FenwickTree *fenwickTree,
    DataIndex index,
    StepCount delta
)
{
    if (!isValidFenwickTree(fenwickTree))
    {
        return STATUS_INVALID_ARGUMENT;
    }

    if (!isValidIndex(index, fenwickTree->size))
    {
        return STATUS_INDEX_OUT_OF_RANGE;
    }

    fenwickTree->data[index] += delta;

    DataIndex treeIndex = index + 1U;

    while (treeIndex <= fenwickTree->size)
    {
        fenwickTree->tree[treeIndex] += delta;
        treeIndex += lowbit(treeIndex);
    }

    return STATUS_SUCCESS;
}

StatusCode bitUpdate(
    FenwickTree *fenwickTree,
    DataIndex index,
    StepCount newValue
)
{
    if (!isValidFenwickTree(fenwickTree))
    {
        return STATUS_INVALID_ARGUMENT;
    }

    if (!isValidIndex(index, fenwickTree->size))
    {
        return STATUS_INDEX_OUT_OF_RANGE;
    }

    const StepCount oldValue = fenwickTree->data[index];

    if (newValue == oldValue)
    {
        return STATUS_SUCCESS;
    }

    DataIndex treeIndex = index + 1U;

    if (newValue > oldValue)
    {
        const StepCount delta = newValue - oldValue;

        while (treeIndex <= fenwickTree->size)
        {
            fenwickTree->tree[treeIndex] += delta;
            treeIndex += lowbit(treeIndex);
        }
    }
    else
    {
        const StepCount delta = oldValue - newValue;

        while (treeIndex <= fenwickTree->size)
        {
            fenwickTree->tree[treeIndex] -= delta;
            treeIndex += lowbit(treeIndex);
        }
    }

    fenwickTree->data[index] = newValue;

    return STATUS_SUCCESS;
}

StatusCode bitPrefixSum(
    const FenwickTree *fenwickTree,
    DataIndex index,
    StepCount *result
)
{
    if (!isValidFenwickTree(fenwickTree) || result == NULL)
    {
        return STATUS_INVALID_ARGUMENT;
    }

    if (!isValidIndex(index, fenwickTree->size))
    {
        return STATUS_INDEX_OUT_OF_RANGE;
    }

    StepCount sum = 0U;
    DataIndex treeIndex = index + 1U;

    while (treeIndex > 0U)
    {
        sum += fenwickTree->tree[treeIndex];
        treeIndex -= lowbit(treeIndex);
    }

    *result = sum;

    return STATUS_SUCCESS;
}

StatusCode bitRangeSum(
    const FenwickTree *fenwickTree,
    DataIndex left,
    DataIndex right,
    StepCount *result
)
{
    if (!isValidFenwickTree(fenwickTree) || result == NULL)
    {
        return STATUS_INVALID_ARGUMENT;
    }

    if (!isValidRange(left, right, fenwickTree->size))
    {
        return STATUS_INDEX_OUT_OF_RANGE;
    }

    StepCount rightPrefix = 0U;

    StatusCode status = bitPrefixSum(
        fenwickTree,
        right,
        &rightPrefix
    );

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    if (left == 0U)
    {
        *result = rightPrefix;
        return STATUS_SUCCESS;
    }

    StepCount leftPrefix = 0U;

    status = bitPrefixSum(
        fenwickTree,
        left - 1U,
        &leftPrefix
    );

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    *result = rightPrefix - leftPrefix;

    return STATUS_SUCCESS;
}