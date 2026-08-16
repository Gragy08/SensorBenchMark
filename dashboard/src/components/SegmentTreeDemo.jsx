import { useMemo, useState } from 'react'
import { motion } from 'framer-motion'

const INITIAL_STEPS = [
  6200, 8400, 7100, 9100, 8000, 9600, 7500,
  8800, 10200, 9300, 7900, 8700, 9900, 7400,
]

const delay = (ms) => new Promise((resolve) => setTimeout(resolve, ms))
const clamp = (value, min, max) => Math.min(Math.max(value, min), max)
const getNextPowerOfTwo = (value) => (value <= 1 ? 1 : 1 << Math.ceil(Math.log2(value)))

const formatMetricLabel = (value, metric) => {
  const label = metric === 'SUM' ? 'Sum' : metric === 'MIN' ? 'Min' : 'Max'
  const abs = Math.abs(value)

  if (abs >= 1000000) {
    return `${label} = ${(value / 1000000).toFixed(1).replace(/\.0$/, '')}M`
  }
  if (abs >= 1000) {
    return `${label} = ${(value / 1000).toFixed(1).replace(/\.0$/, '')}K`
  }

  return `${label} = ${Math.round(value)}`
}

const buildPaddedValues = (values) => {
  const paddedLength = getNextPowerOfTwo(values.length)
  const padded = [...values]
  while (padded.length < paddedLength) {
    padded.push(0)
  }
  return padded
}

const buildTreeState = (values, metric) => {
  const paddedValues = buildPaddedValues(values)
  const nodes = []
  let nodeCounter = 0

  const build = (left, right) => {
    const nodeId = `v${nodeCounter}`
    nodeCounter += 1

    const node = {
      id: nodeId,
      left,
      right,
      isVirtual: left >= values.length,
      children: [],
      value: 0,
      range: `[${left}, ${right + 1})`,
      x: 0,
      y: 0,
    }

    if (left === right) {
      node.value = paddedValues[left] || 0
      node.range = `[${left}, ${left + 1})`
      nodes.push(node)
      return node
    }

    const mid = Math.floor((left + right) / 2)
    const leftChild = build(left, mid)
    const rightChild = build(mid + 1, right)

    node.children = [leftChild, rightChild]
    const leftValue = leftChild.value
    const rightValue = rightChild.value

    if (metric === 'MIN') {
      node.value = Math.min(leftValue, rightValue)
    } else if (metric === 'MAX') {
      node.value = Math.max(leftValue, rightValue)
    } else {
      node.value = leftValue + rightValue
    }

    nodes.push(node)
    return node
  }

  const root = build(0, paddedValues.length - 1)

  const levels = []
  const collectLevels = (currentNode, depth) => {
    if (!levels[depth]) {
      levels[depth] = []
    }
    levels[depth].push(currentNode)
    currentNode.children.forEach((child) => collectLevels(child, depth + 1))
  }

  collectLevels(root, 0)

  const drawWidth = 1200
  const drawHeight = 700
  const startY = 90
  const verticalGap = 120

  levels.forEach((levelNodes, depth) => {
    const stepX = drawWidth / (levelNodes.length + 1)

    levelNodes.forEach((node, index) => {
      node.x = stepX * (index + 1)
      node.y = startY + depth * verticalGap
    })
  })

  const edges = []
  const collectEdges = (currentNode) => {
    currentNode.children.forEach((child) => {
      edges.push({
        x1: currentNode.x,
        y1: currentNode.y,
        x2: child.x,
        y2: child.y,
      })
      collectEdges(child)
    })
  }

  collectEdges(root)

  return {
    root,
    nodes,
    edges,
    paddedValues,
    leafCount: paddedValues.length,
  }
}

const queryRange = (node, left, right, queryLeft, queryRight, metric) => {
  if (queryLeft <= left && right <= queryRight) {
    return node.value
  }
  if (right < queryLeft || left > queryRight) {
    if (metric === 'MIN') return Number.POSITIVE_INFINITY
    if (metric === 'MAX') return Number.NEGATIVE_INFINITY
    return 0
  }

  const mid = Math.floor((left + right) / 2)

  const leftPart = queryRange(node.children[0], left, mid, queryLeft, queryRight, metric)
  const rightPart = queryRange(node.children[1], mid + 1, right, queryLeft, queryRight, metric)

  if (metric === 'MIN') return Math.min(leftPart, rightPart)
  if (metric === 'MAX') return Math.max(leftPart, rightPart)
  return leftPart + rightPart
}

const collectCoveredNodes = (node, left, right, queryLeft, queryRight, activeSet) => {
  if (queryLeft <= left && right <= queryRight) {
    activeSet.add(node.id)
    return
  }

  if (right < queryLeft || left > queryRight) {
    return
  }

  const mid = Math.floor((left + right) / 2)
  collectCoveredNodes(node.children[0], left, mid, queryLeft, queryRight, activeSet)
  collectCoveredNodes(node.children[1], mid + 1, right, queryLeft, queryRight, activeSet)
}

const findLeafPath = (node, left, right, targetIndex) => {
  if (left === right) {
    return [node.id]
  }

  const mid = Math.floor((left + right) / 2)
  if (targetIndex <= mid) {
    return [node.id, ...findLeafPath(node.children[0], left, mid, targetIndex)]
  }
  return [node.id, ...findLeafPath(node.children[1], mid + 1, right, targetIndex)]
}

export default function SegmentTreeDemo() {
  const [steps, setSteps] = useState(INITIAL_STEPS)
  const [metric, setMetric] = useState('SUM')
  const [updateDay, setUpdateDay] = useState(6)
  const [updateValue, setUpdateValue] = useState(8500)
  const [queryStart, setQueryStart] = useState(2)
  const [queryEnd, setQueryEnd] = useState(9)
  const [activeNodes, setActiveNodes] = useState([])
  const [liveMessage, setLiveMessage] = useState('Ready to query the segment tree.')
  const [isAnimating, setIsAnimating] = useState(false)

  const treeState = useMemo(() => buildTreeState(steps, metric), [steps, metric])

  const queryResult = useMemo(() => {
    const left = Math.min(queryStart, queryEnd) - 1
    const right = Math.max(queryStart, queryEnd) - 1
    const leafCount = treeState.paddedValues.length
    const result = queryRange(treeState.root, 0, leafCount - 1, left, right, metric)
    return result
  }, [metric, queryEnd, queryStart, steps, treeState])

  const handleUpdate = async () => {
    setIsAnimating(true)
    const userDay = clamp(Number(updateDay) || 1, 1, steps.length)
    const arrayIndex = userDay - 1
    const nextValue = Number(updateValue) || steps[arrayIndex]

    setLiveMessage(`Updating Day ${userDay} from ${steps[arrayIndex]} to ${nextValue}.`)
    setSteps((previous) => previous.map((value, index) => (index === arrayIndex ? nextValue : value)))

    const targetLeafIndex = arrayIndex
    const path = findLeafPath(treeState.root, 0, treeState.paddedValues.length - 1, targetLeafIndex)
    const pathSet = new Set(path)

    setActiveNodes([...pathSet])
    await delay(300)

    for (let i = 1; i < path.length; i += 1) {
      const nextActive = path.slice(0, i + 1)
      setActiveNodes(nextActive)
      setLiveMessage(`Propagating update from ${path[i - 1]} to ${path[i]}...`)
      await delay(450)
    }

    setLiveMessage(`Update propagated to the root. The tree recomputed its ${metric.toLowerCase()} aggregate.`)
    setActiveNodes([])
    setIsAnimating(false)
  }

  const handleQuery = async () => {
    setIsAnimating(true)
    const left = Math.min(queryStart, queryEnd) - 1
    const right = Math.max(queryStart, queryEnd) - 1
    const activeSet = new Set()

    collectCoveredNodes(treeState.root, 0, treeState.paddedValues.length - 1, left, right, activeSet)
    setActiveNodes([])
    await delay(150)
    setActiveNodes([...activeSet])

    setLiveMessage(`Query range [${left + 1}, ${right + 1}] is covered by the highlighted nodes.`)
    await delay(250)

    if (metric === 'SUM') {
      setLiveMessage(`Range ${metric}: ${queryResult.toLocaleString()} = sum of covered nodes.`)
    } else if (metric === 'MIN') {
      setLiveMessage(`Range ${metric}: ${queryResult.toLocaleString()} = minimum of covered nodes.`)
    } else {
      setLiveMessage(`Range ${metric}: ${queryResult.toLocaleString()} = maximum of covered nodes.`)
    }

    setIsAnimating(false)
  }

  const renderTreeSvg = () => {
    const activeSet = new Set(activeNodes)

    return (
      <svg viewBox="0 0 1200 700" className="h-[620px] w-full overflow-hidden" preserveAspectRatio="xMidYMid meet">
        <g>
          {treeState.edges.map((edge, index) => (
            <line
              key={`line-${index}`}
              x1={edge.x1}
              y1={edge.y1}
              x2={edge.x2}
              y2={edge.y2}
              stroke="#64748b"
              strokeWidth="1.5"
              opacity="0.8"
            />
          ))}
        </g>

        <g>
          {treeState.nodes.map((node) => {
            const isActive = activeSet.has(node.id)
            const isVirtual = node.isVirtual
            const circleFill = isVirtual ? '#374151' : isActive ? '#22d3ee' : '#0f172a'
            const circleStroke = isVirtual ? '#6b7280' : isActive ? '#67e8f9' : '#1e293b'
            const textColor = isVirtual ? '#a1a1aa' : '#e2e8f0'

            return (
              <g key={node.id} transform={`translate(${node.x}, ${node.y})`}>
                <motion.circle
                  cx={0}
                  cy={0}
                  r={42}
                  fill={circleFill}
                  stroke={circleStroke}
                  strokeWidth={isActive ? 3 : 1.5}
                  animate={{
                    scale: isActive ? 1.08 : 1,
                    opacity: isVirtual ? 0.35 : 1,
                  }}
                  transition={{ duration: 0.28 }}
                />

                <text x={0} y={-14} textAnchor="middle" fontSize="11" fill={textColor} fontWeight="700">
                  {node.id}
                </text>
                <text x={0} y={6} textAnchor="middle" fontSize="10" fill={textColor} opacity={0.9}>
                  {node.range}
                </text>
                <text x={0} y={28} textAnchor="middle" fontSize="10" fill={isActive ? '#e0f2fe' : textColor} fontWeight="600">
                  {formatMetricLabel(node.value, metric)}
                </text>
              </g>
            )
          })}
        </g>
      </svg>
    )
  }

  return (
    <div className="min-h-screen bg-slate-100 px-4 py-8 text-slate-900 antialiased">
      <div className="mx-auto max-w-6xl rounded-[28px] border border-slate-200 bg-white p-6 shadow-[0_20px_70px_rgba(15,23,42,0.08)] sm:p-8">
        <header className="mb-8 text-center">
          <p className="text-xs font-semibold uppercase tracking-[0.35em] text-cyan-500">
            Case Study 4
          </p>
          <h2 className="mt-3 text-3xl font-bold text-slate-900 sm:text-4xl">
            Segment Tree
          </h2>
          <p className="mx-auto mt-3 max-w-2xl text-sm text-slate-600 sm:text-base">
            Segment trees reduce range operations to <span className="font-semibold text-slate-900">O(log n)</span>
            by descending to relevant segments and merging them back up.
          </p>
        </header>

        <div className="grid grid-cols-1 gap-4 xl:grid-cols-12">
          <div className="xl:col-span-9">
            <section className="rounded-2xl bg-slate-900 p-4 shadow-inner shadow-slate-950/60">
              <div className="mb-4 flex items-center justify-between">
                <h3 className="text-sm font-semibold uppercase tracking-[0.25em] text-cyan-400">
                  Segment Tree Canvas
                </h3>
                <div className="rounded-full border border-cyan-700 bg-cyan-500/10 px-2 py-1 text-[10px] font-semibold uppercase tracking-[0.2em] text-cyan-300">
                  {metric} mode
                </div>
              </div>

              {renderTreeSvg()}
            </section>
          </div>

          <div className="space-y-5 rounded-2xl border border-slate-200 bg-slate-50 p-4 sm:p-5 xl:col-span-3">
            <div>
              <label className="mb-2 block text-xs font-semibold uppercase tracking-[0.25em] text-slate-500">
                Metric Mode
              </label>
              <select
                value={metric}
                onChange={(event) => setMetric(event.target.value)}
                className="w-full rounded-xl border border-slate-300 bg-white px-3 py-2 text-sm font-medium text-slate-900 outline-none transition focus:border-cyan-400"
              >
                <option value="SUM">SUM</option>
                <option value="MIN">MIN</option>
                <option value="MAX">MAX</option>
              </select>
            </div>

            <div className="border-t border-slate-200 pt-5">
              <label className="mb-2 block text-xs font-semibold uppercase tracking-[0.25em] text-slate-500">
                Point Update
              </label>
              <div className="space-y-2">
                <input
                  type="number"
                  min="1"
                  max="14"
                  value={updateDay}
                  onChange={(event) => setUpdateDay(clamp(Number(event.target.value) || 1, 1, 14))}
                  className="w-full rounded-xl border border-slate-300 bg-white px-3 py-2 text-sm font-medium text-slate-900 outline-none transition focus:border-cyan-400"
                  placeholder="Day"
                />
                <input
                  type="number"
                  value={updateValue}
                  onChange={(event) => setUpdateValue(Number(event.target.value) || 0)}
                  className="w-full rounded-xl border border-slate-300 bg-white px-3 py-2 text-sm font-medium text-slate-900 outline-none transition focus:border-cyan-400"
                  placeholder="New value"
                />
                <button
                  type="button"
                  onClick={() => void handleUpdate()}
                  disabled={isAnimating}
                  className="w-full rounded-xl bg-orange-500 px-3 py-2 text-sm font-semibold text-white shadow-sm transition hover:bg-orange-600 disabled:cursor-not-allowed disabled:bg-orange-300"
                >
                  Update Day {Number(updateDay)}
                </button>
              </div>
            </div>

            <div className="border-t border-slate-200 pt-5">
              <label className="mb-2 block text-xs font-semibold uppercase tracking-[0.25em] text-slate-500">
                Range Query
              </label>
              <div className="grid grid-cols-2 gap-2">
                <input
                  type="number"
                  min="1"
                  max="14"
                  value={queryStart}
                  onChange={(event) => setQueryStart(clamp(Number(event.target.value) || 1, 1, 14))}
                  className="rounded-xl border border-slate-300 bg-white px-3 py-2 text-sm font-medium text-slate-900 outline-none transition focus:border-cyan-400"
                  placeholder="L"
                />
                <input
                  type="number"
                  min="1"
                  max="14"
                  value={queryEnd}
                  onChange={(event) => setQueryEnd(clamp(Number(event.target.value) || 1, 1, 14))}
                  className="rounded-xl border border-slate-300 bg-white px-3 py-2 text-sm font-medium text-slate-900 outline-none transition focus:border-cyan-400"
                  placeholder="R"
                />
              </div>

              <button
                type="button"
                onClick={() => void handleQuery()}
                disabled={isAnimating}
                className="mt-3 w-full rounded-xl bg-cyan-500 px-3 py-2 text-sm font-semibold text-white shadow-sm transition hover:bg-cyan-600 disabled:cursor-not-allowed disabled:bg-cyan-300"
              >
                Run Query
              </button>
            </div>

            <div className="border-t border-slate-200 pt-5">
              <div className="mb-2 text-xs font-semibold uppercase tracking-[0.25em] text-slate-500">
                Live Explanation
              </div>

              <div className="rounded-2xl border border-slate-200 bg-white p-3 text-sm text-slate-700">
                <p className="break-words">{liveMessage}</p>
                <p className="mt-3 text-cyan-700">
                  Query result: <span className="font-semibold">{queryResult.toLocaleString()}</span>
                </p>
              </div>
            </div>

            <div className="border-t border-slate-200 pt-5">
              <div className="mb-2 text-xs font-semibold uppercase tracking-[0.25em] text-slate-500">
                Complexity
              </div>
              <div className="space-y-2 text-sm text-slate-700">
                <div className="rounded-xl border border-orange-200 bg-orange-50 px-3 py-2">
                  <span className="font-semibold text-orange-700">Update:</span> O(log n)
                </div>
                <div className="rounded-xl border border-cyan-200 bg-cyan-50 px-3 py-2">
                  <span className="font-semibold text-cyan-700">Query:</span> O(log n)
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  )
}
