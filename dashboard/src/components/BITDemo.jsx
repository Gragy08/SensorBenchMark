import { useMemo, useState } from 'react'
import { motion } from 'framer-motion'

const INITIAL_STEPS = [
  6200, 8400, 7100, 9100, 8000, 9600, 7500,
  8800, 10200, 9300, 7900, 8700, 9900, 7400,
]

const delay = (ms) => new Promise((resolve) => setTimeout(resolve, ms))

const lowbit = (value) => value & -value

const buildBIT = (values) => {
  const bit = Array(values.length + 1).fill(0)
  for (let i = 1; i <= values.length; i += 1) {
    bit[i] += values[i - 1]
    const j = i + lowbit(i)
    if (j <= values.length) {
      bit[j] += bit[i]
    }
  }
  return bit
}

const getBITRange = (tree, index) => {
  let sum = 0
  let current = index
  while (current > 0) {
    sum += tree[current]
    current -= lowbit(current)
  }
  return sum
}

export default function BITDemo() {
  const [steps, setSteps] = useState(INITIAL_STEPS)
  const [updateIndex, setUpdateIndex] = useState(6)
  const [queryStart, setQueryStart] = useState(2)
  const [queryEnd, setQueryEnd] = useState(9)
  const [activeArrayIndex, setActiveArrayIndex] = useState(null)
  const [activeBitNodes, setActiveBitNodes] = useState([])
  const [currentLowbit, setCurrentLowbit] = useState('')
  const [currentQuery, setCurrentQuery] = useState('')
  const [isAnimating, setIsAnimating] = useState(false)

  const bitTree = useMemo(() => buildBIT(steps), [steps])

  const rangeResult = useMemo(() => {
    const left = Math.min(queryStart, queryEnd)
    const right = Math.max(queryStart, queryEnd)
    const prefixR = getBITRange(bitTree, right)
    const prefixL = getBITRange(bitTree, left - 1)
    return prefixR - prefixL
  }, [bitTree, queryEnd, queryStart])

  const renderBITLayout = () => {
    const rows = [
      [8],
      [4, 12],
      [2, 6, 10, 14],
      [1, 3, 5, 7, 9, 11, 13],
    ]

    return rows.map((row, rowIndex) => (
      <div key={`bit-row-${rowIndex}`} className="flex justify-center gap-4">
        {row.map((nodeIndex) => {
          const bitValue = bitTree[nodeIndex] || 0
          const isActive = activeBitNodes.includes(nodeIndex)

          return (
            <motion.div
              key={`node-${nodeIndex}`}
              animate={{
                scale: isActive ? 1.18 : 1,
                boxShadow: isActive
                  ? '0 0 0 2px rgba(34,211,238,0.18), 0 0 20px rgba(34,211,238,0.28)'
                  : '0 0 0 0 rgba(15,23,42,0)',
                backgroundColor: isActive ? '#22d3ee' : '#0f172a',
                borderColor: isActive ? '#67e8f9' : '#1e293b',
              }}
              transition={{ duration: 0.3 }}
              className="flex h-12 w-12 items-center justify-center rounded-full border text-xs font-bold text-slate-100"
            >
              {nodeIndex}
              <span className="sr-only">{bitValue}</span>
            </motion.div>
          )
        })}
      </div>
    ))
  }

  const runUpdateAnimation = async () => {
    setIsAnimating(true)
    setCurrentQuery('')

    const userDay = Math.min(Math.max(Number(updateIndex) || 1, 1), steps.length)
    const arrayIndex = userDay - 1
    const bitIndex = userDay

    setActiveArrayIndex(arrayIndex)
    setSteps((previous) => previous.map((value, index) => (index === arrayIndex ? value + 1500 : value)))
    setCurrentLowbit(`Update day ${userDay}: array index ${arrayIndex} matches BIT index ${bitIndex}, lowbit(${bitIndex}) = ${lowbit(bitIndex)}`)

    let current = bitIndex
    while (current <= steps.length) {
      setActiveBitNodes([current])
      setCurrentLowbit(`i = ${current}, lowbit(i) = ${lowbit(current)}; next = i + lowbit(i) = ${current + lowbit(current)}`)
      await delay(500)
      current += lowbit(current)
    }

    setActiveBitNodes([])
    setCurrentLowbit('Cascading update complete. Each affected Fenwick node was refreshed in O(log n).')
    setIsAnimating(false)
  }

  const runPrefixQueryAnimation = async () => {
    setIsAnimating(true)
    setCurrentLowbit('')

    const left = Math.min(queryStart, queryEnd)
    const right = Math.max(queryStart, queryEnd)
    const queryRight = right
    const queryLeft = left - 1

    const buildPath = async (index, label) => {
      const path = []
      let current = index
      while (current > 0) {
        path.push(current)
        setCurrentLowbit(`${label}: lowbit(${current}) = ${lowbit(current)} → jump to ${current - lowbit(current)}`)
        current -= lowbit(current)
        if (current > 0) {
          setActiveBitNodes([...path])
          setCurrentQuery(`${label} = prefix(${index}) = sum(${path.join(' + ')})`)
        }
        await delay(450)
      }
      return path
    }

    setActiveBitNodes([])
    setCurrentQuery(`Querying prefix(${queryRight}) and prefix(${queryLeft}) ...`)
    const rightPath = await buildPath(queryRight, 'prefix(R)')
    const leftPath = await buildPath(queryLeft, 'prefix(L-1)')

    const result = getBITRange(bitTree, queryRight) - getBITRange(bitTree, queryLeft)
    setCurrentQuery(`prefix(${queryRight}) - prefix(${queryLeft}) = ${result}`)
    setCurrentLowbit(`Fenwick traversal uses lowbit to jump logarithmically through the tree: O(log n).`)
    setActiveBitNodes([...new Set([...rightPath, ...leftPath])])
    setIsAnimating(false)
  }

  return (
    <div className="min-h-screen bg-slate-100 px-4 py-8 text-slate-900 antialiased">
      <div className="mx-auto max-w-6xl rounded-[28px] border border-slate-200 bg-white p-6 shadow-[0_20px_70px_rgba(15,23,42,0.08)] sm:p-8">
        <header className="mb-8 text-center">
          <p className="text-xs font-semibold uppercase tracking-[0.35em] text-cyan-500">
            Case Study 3
          </p>
          <h2 className="mt-3 text-3xl font-bold text-slate-900 sm:text-4xl">
            Binary Indexed Tree (Fenwick Tree)
          </h2>
          <p className="mx-auto mt-3 max-w-2xl text-sm text-slate-600 sm:text-base">
            Fenwick trees support point updates and prefix sums in <span className="font-semibold text-slate-900">O(log n)</span>
            by repeatedly jumping using <span className="font-semibold text-slate-900">lowbit(i) = i & -i</span>.
          </p>
        </header>

        <div className="grid grid-cols-1 gap-4 xl:grid-cols-12">
          <div className="space-y-6 xl:col-span-9">
            <section className="rounded-2xl border border-slate-200 bg-slate-50 p-4">
              <div className="mb-4 flex items-center justify-between">
                <h3 className="text-sm font-semibold uppercase tracking-[0.25em] text-slate-500">
                  Original Array
                </h3>
                <span className="text-xs text-slate-500">14 days</span>
              </div>

              <div className="flex gap-3 overflow-x-auto pb-2">
                {steps.map((value, index) => {
                  const isActive = activeArrayIndex === index

                  return (
                    <motion.div
                      key={`array-${index}`}
                      animate={{
                        scale: isActive ? 1.12 : 1,
                        backgroundColor: isActive ? '#f97316' : '#ffffff',
                        borderColor: isActive ? '#f97316' : '#cbd5e1',
                        boxShadow: isActive
                          ? '0 0 0 2px rgba(249,115,22,0.12), 0 20px 28px rgba(249,115,22,0.18)'
                          : '0 10px 16px rgba(15,23,42,0.04)',
                      }}
                      transition={{ type: 'spring', stiffness: 260, damping: 18 }}
                      className="flex w-14 min-w-[3.5rem] flex-col items-center rounded-2xl border p-2"
                    >
                      <span className="mb-2 text-[10px] font-bold uppercase tracking-[0.22em] text-slate-500">
                        D{index + 1}
                      </span>
                      <div className="flex h-20 w-full items-center justify-center rounded-xl bg-slate-100/80">
                        <span className="text-[11px] font-bold text-slate-900 sm:text-sm">
                          {value.toLocaleString()}
                        </span>
                      </div>
                    </motion.div>
                  )
                })}
              </div>
            </section>

            <section className="rounded-2xl bg-slate-900 p-4 shadow-inner shadow-slate-950/60">
              <div className="mb-4 flex items-center justify-between">
                <h3 className="text-sm font-semibold uppercase tracking-[0.25em] text-cyan-400">
                  Fenwick Tree Canvas
                </h3>
                <span className="text-xs text-slate-300">lowbit(i) = i & (-i)</span>
              </div>

              <div className="space-y-4">
                {renderBITLayout()}
              </div>
            </section>
          </div>

          <div className="space-y-5 rounded-2xl border border-slate-200 bg-slate-50 p-4 sm:p-5 xl:col-span-3">
            <div>
              <label className="mb-2 block text-xs font-semibold uppercase tracking-[0.25em] text-slate-500">
                Point Update
              </label>
              <div className="flex gap-2">
                <input
                  type="number"
                  min="1"
                  max="14"
                  value={updateIndex}
                  onChange={(event) => setUpdateIndex(Math.min(14, Math.max(1, Number(event.target.value) || 1)))}
                  className="w-20 rounded-xl border border-slate-300 bg-white px-3 py-2 text-sm font-medium text-slate-900 outline-none transition focus:border-cyan-400"
                />
                <button
                  type="button"
                  onClick={() => void runUpdateAnimation()}
                  disabled={isAnimating}
                  className="flex-1 rounded-xl bg-orange-500 px-3 py-2 text-sm font-semibold text-white shadow-sm transition hover:bg-orange-600 disabled:cursor-not-allowed disabled:bg-orange-300"
                >
                  Update Day {Number(updateIndex)}
                </button>
              </div>
            </div>

            <div className="border-t border-slate-200 pt-5">
              <label className="mb-2 block text-xs font-semibold uppercase tracking-[0.25em] text-slate-500">
                Prefix Query
              </label>
              <div className="grid grid-cols-2 gap-2">
                <input
                  type="number"
                  min="1"
                  max="14"
                  value={queryStart}
                  onChange={(event) => setQueryStart(Math.min(14, Math.max(1, Number(event.target.value) || 1)))}
                  className="rounded-xl border border-slate-300 bg-white px-3 py-2 text-sm font-medium text-slate-900 outline-none transition focus:border-cyan-400"
                  placeholder="L"
                />
                <input
                  type="number"
                  min="1"
                  max="14"
                  value={queryEnd}
                  onChange={(event) => setQueryEnd(Math.min(14, Math.max(1, Number(event.target.value) || 1)))}
                  className="rounded-xl border border-slate-300 bg-white px-3 py-2 text-sm font-medium text-slate-900 outline-none transition focus:border-cyan-400"
                  placeholder="R"
                />
              </div>

              <button
                type="button"
                onClick={() => void runPrefixQueryAnimation()}
                disabled={isAnimating}
                className="mt-3 w-full rounded-xl bg-cyan-500 px-3 py-2 text-sm font-semibold text-white shadow-sm transition hover:bg-cyan-600 disabled:cursor-not-allowed disabled:bg-cyan-300"
              >
                Run Prefix Query
              </button>
            </div>

            <div className="border-t border-slate-200 pt-5">
              <div className="mb-2 text-xs font-semibold uppercase tracking-[0.25em] text-slate-500">
                Live Explanation
              </div>

              <div className="rounded-2xl border border-slate-200 bg-white p-3 text-sm text-slate-700">
                <p className="font-semibold text-cyan-700">Current lowbit:</p>
                <p className="mt-1 break-words">{currentLowbit || 'Ready to animate a Fenwick jump.'}</p>

                <p className="mt-4 font-semibold text-cyan-700">Current query:</p>
                <p className="mt-1 break-words">{currentQuery || 'prefix(R) - prefix(L - 1)'}</p>
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
                  <span className="font-semibold text-cyan-700">Prefix query:</span> O(log n)
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  )
}
