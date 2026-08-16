#include "segment_tree.h"

#include <stdlib.h>

static bool isValidSegmentTree(const SegmentTree *segmentTree)
{
    return segmentTree != NULL
        && segmentTree->tree != NULL
        && segmentTree->data != NULL
        && segmentTree->size > 0U
        && segmentTree->capacity > 0U;
}

static SegmentNode createLeafNode(StepCount value)
{
    SegmentNode node = {
        .sum = value,
        .minimum = value,
        .maximum = value
    };

    return node;
}

SegmentNode mergeNodes(
    SegmentNode leftNode,
    SegmentNode rightNode
)
{
    SegmentNode mergedNode = {
        .sum = leftNode.sum + rightNode.sum,
        .minimum = (
            leftNode.minimum < rightNode.minimum
                ? leftNode.minimum
                : rightNode.minimum
        ),
        .maximum = (
            leftNode.maximum > rightNode.maximum
                ? leftNode.maximum
                : rightNode.maximum
        )
    };

    return mergedNode;
}

static void buildRecursive(
    SegmentTree *segmentTree,
    DataIndex nodeIndex,
    DataIndex segmentLeft,
    DataIndex segmentRight
)
{
    if (segmentLeft == segmentRight)
    {
        segmentTree->tree[nodeIndex] = createLeafNode(
            segmentTree->data[segmentLeft]
        );

        return;
    }

    const DataIndex middle =
        segmentLeft + (segmentRight - segmentLeft) / 2U;

    const DataIndex leftChild = nodeIndex * 2U + 1U;
    const DataIndex rightChild = nodeIndex * 2U + 2U;

    buildRecursive(
        segmentTree,
        leftChild,
        segmentLeft,
        middle
    );

    buildRecursive(
        segmentTree,
        rightChild,
        middle + 1U,
        segmentRight
    );

    segmentTree->tree[nodeIndex] = mergeNodes(
        segmentTree->tree[leftChild],
        segmentTree->tree[rightChild]
    );
}

StatusCode buildSegmentTree(
    SegmentTree *segmentTree,
    const StepCount data[],
    DataIndex size
)
{
    if (segmentTree == NULL || data == NULL || size == 0U)
    {
        return STATUS_INVALID_ARGUMENT;
    }

    if (size > SIZE_MAX / 4U)
    {
        return STATUS_ALLOCATION_FAILED;
    }

    segmentTree->tree = NULL;
    segmentTree->data = NULL;
    segmentTree->size = 0U;
    segmentTree->capacity = 0U;

    const DataIndex capacity = size * 4U;

    segmentTree->tree = calloc(
        capacity,
        sizeof(*segmentTree->tree)
    );

    if (segmentTree->tree == NULL)
    {
        return STATUS_ALLOCATION_FAILED;
    }

    segmentTree->data = malloc(
        size * sizeof(*segmentTree->data)
    );

    if (segmentTree->data == NULL)
    {
        free(segmentTree->tree);
        segmentTree->tree = NULL;

        return STATUS_ALLOCATION_FAILED;
    }

    for (DataIndex index = 0U; index < size; ++index)
    {
        segmentTree->data[index] = data[index];
    }

    segmentTree->size = size;
    segmentTree->capacity = capacity;

    buildRecursive(
        segmentTree,
        0U,
        0U,
        size - 1U
    );

    return STATUS_SUCCESS;
}

void destroySegmentTree(SegmentTree *segmentTree)
{
    if (segmentTree == NULL)
    {
        return;
    }

    free(segmentTree->tree);
    free(segmentTree->data);

    segmentTree->tree = NULL;
    segmentTree->data = NULL;
    segmentTree->size = 0U;
    segmentTree->capacity = 0U;
}

static SegmentNode queryRecursive(
    const SegmentTree *segmentTree,
    DataIndex nodeIndex,
    DataIndex segmentLeft,
    DataIndex segmentRight,
    DataIndex queryLeft,
    DataIndex queryRight
)
{
    if (
        queryLeft <= segmentLeft
        && segmentRight <= queryRight
    )
    {
        return segmentTree->tree[nodeIndex];
    }

    const DataIndex middle =
        segmentLeft + (segmentRight - segmentLeft) / 2U;

    if (queryRight <= middle)
    {
        return queryRecursive(
            segmentTree,
            nodeIndex * 2U + 1U,
            segmentLeft,
            middle,
            queryLeft,
            queryRight
        );
    }

    if (queryLeft > middle)
    {
        return queryRecursive(
            segmentTree,
            nodeIndex * 2U + 2U,
            middle + 1U,
            segmentRight,
            queryLeft,
            queryRight
        );
    }

    const SegmentNode leftResult = queryRecursive(
        segmentTree,
        nodeIndex * 2U + 1U,
        segmentLeft,
        middle,
        queryLeft,
        queryRight
    );

    const SegmentNode rightResult = queryRecursive(
        segmentTree,
        nodeIndex * 2U + 2U,
        middle + 1U,
        segmentRight,
        queryLeft,
        queryRight
    );

    return mergeNodes(leftResult, rightResult);
}

StatusCode querySegmentTree(
    const SegmentTree *segmentTree,
    DataIndex left,
    DataIndex right,
    SegmentNode *result
)
{
    if (!isValidSegmentTree(segmentTree) || result == NULL)
    {
        return STATUS_INVALID_ARGUMENT;
    }

    if (!isValidRange(left, right, segmentTree->size))
    {
        return STATUS_INDEX_OUT_OF_RANGE;
    }

    *result = queryRecursive(
        segmentTree,
        0U,
        0U,
        segmentTree->size - 1U,
        left,
        right
    );

    return STATUS_SUCCESS;
}

static void updateRecursive(
    SegmentTree *segmentTree,
    DataIndex nodeIndex,
    DataIndex segmentLeft,
    DataIndex segmentRight,
    DataIndex updateIndex,
    StepCount newValue
)
{
    if (segmentLeft == segmentRight)
    {
        segmentTree->tree[nodeIndex] = createLeafNode(newValue);
        return;
    }

    const DataIndex middle =
        segmentLeft + (segmentRight - segmentLeft) / 2U;

    const DataIndex leftChild = nodeIndex * 2U + 1U;
    const DataIndex rightChild = nodeIndex * 2U + 2U;

    if (updateIndex <= middle)
    {
        updateRecursive(
            segmentTree,
            leftChild,
            segmentLeft,
            middle,
            updateIndex,
            newValue
        );
    }
    else
    {
        updateRecursive(
            segmentTree,
            rightChild,
            middle + 1U,
            segmentRight,
            updateIndex,
            newValue
        );
    }

    segmentTree->tree[nodeIndex] = mergeNodes(
        segmentTree->tree[leftChild],
        segmentTree->tree[rightChild]
    );
}

StatusCode updateSegmentTree(
    SegmentTree *segmentTree,
    DataIndex index,
    StepCount newValue
)
{
    if (!isValidSegmentTree(segmentTree))
    {
        return STATUS_INVALID_ARGUMENT;
    }

    if (!isValidIndex(index, segmentTree->size))
    {
        return STATUS_INDEX_OUT_OF_RANGE;
    }

    if (segmentTree->data[index] == newValue)
    {
        return STATUS_SUCCESS;
    }

    segmentTree->data[index] = newValue;

    updateRecursive(
        segmentTree,
        0U,
        0U,
        segmentTree->size - 1U,
        index,
        newValue
    );

    return STATUS_SUCCESS;
}