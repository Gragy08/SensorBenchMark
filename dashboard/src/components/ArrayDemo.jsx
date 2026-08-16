import { useMemo, useState } from 'react'
import { motion } from 'framer-motion'

const INITIAL_VALUES = [
  6200, 8400, 7100, 9100, 8000, 9600, 7500,
  8800, 10200, 9300, 7900, 8700, 9900, 7400,
]

const delay = (ms) => new Promise((resolve) => setTimeout(resolve, ms))

const clampDay = (value) => Math.min(Math.max(value, 1), INITIAL_VALUES.length)

export default function ArrayDemo() {
  const [values, setValues] = useState(INITIAL_VALUES)
  const [updateDay, setUpdateDay] = useState(6)
  const [queryStart, setQueryStart] = useState(2)
  const [queryEnd, setQueryEnd] = useState(9)
  const [flashIndex, setFlashIndex] = useState(null)
  const [activeRange, setActiveRange] = useState([])
  const [runningTotal, setRunningTotal] = useState(0)
  const [isQuerying, setIsQuerying] = useState(false)
  const [lastUpdated, setLastUpdated] = useState(null)

  const fullRangeTotal = useMemo(
    () => values.reduce((sum, value) => sum + value, 0),
    [values],
  )

  const currentRangeTotal = useMemo(() => {
    const start = Math.min(queryStart, queryEnd) - 1
    const end = Math.max(queryStart, queryEnd) - 1
    return values.slice(start, end + 1).reduce((sum, value) => sum + value, 0)
  }, [queryEnd, queryStart, values])

  const handlePointUpdate = () => {
    const userDay = clampDay(Number(updateDay) || 1)
    const zeroIndex = userDay - 1

    setValues((previousValues) =>
      previousValues.map((value, index) =>
        index === zeroIndex ? value + 1500 : value,
      ),
    )

    setLastUpdated(zeroIndex)
    setActiveRange([])
    setFlashIndex(zeroIndex)

    window.setTimeout(() => setFlashIndex(null), 700)
  }

  const handleRangeQuery = async () => {
    if (isQuerying) return

    const start = Math.min(queryStart, queryEnd) - 1
    const end = Math.max(queryStart, queryEnd) - 1

    setIsQuerying(true)
    setRunningTotal(0)
    setActiveRange([])

    let total = 0

    for (let index = start; index <= end; index += 1) {
      setActiveRange([index])
      await delay(500)
      total += values[index]
      setRunningTotal(total)
    }

    setActiveRange([])
    setIsQuerying(false)
  }

  return (
    <div className="min-h-screen bg-slate-100 px-4 py-10 text-slate-900 antialiased">
      <div className="grid grid-cols-12 gap-6 w-full max-w-7xl mx-auto px-4">
        <div className="col-span-9">
          <div className="bg-white rounded-2xl shadow-xl p-8 w-full">
            <header className="mb-8 text-center">
              <p className="text-xs font-semibold uppercase tracking-[0.35em] text-cyan-500">
                Case Study 1
              </p>
              <h2 className="mt-3 text-3xl font-bold text-slate-900 sm:text-4xl">
                Raw Array
              </h2>
              <p className="mx-auto mt-3 max-w-2xl text-sm text-slate-600 sm:text-base">
                Point update is <span className="font-semibold text-slate-900">O(1)</span> because it changes one indexed value directly.
                Range sum is <span className="font-semibold text-slate-900">O(n)</span> because it must visit every value from L to R.
              </p>
            </header>

            <div className="space-y-6">
              <div className="mb-4 flex items-center justify-between gap-3">
                <span className="text-xs font-semibold uppercase tracking-[0.28em] text-slate-500">
                  Days 1–14
                </span>
                <span className="rounded-full border border-cyan-200 bg-cyan-50 px-3 py-1 text-xs font-semibold text-cyan-700">
                  Total = {fullRangeTotal.toLocaleString()}
                </span>
              </div>

              <div className="w-full overflow-x-auto py-2 custom-scrollbar rounded-2xl border border-slate-200 bg-slate-50 p-4">
                <div className="flex min-w-max items-center justify-center gap-3 mx-auto px-4">
                  {values.map((value, index) => {
                    const isFlash = flashIndex === index
                    const isActive = activeRange.includes(index)

                    return (
                      <motion.div
                        key={`${index}-${value}`}
                        layout
                        animate={{
                          scale: isFlash ? 1.12 : isActive ? 1.05 : 1,
                          y: isActive ? -8 : 0,
                          backgroundColor: isFlash
                            ? '#f97316'
                            : isActive
                              ? '#22d3ee'
                              : '#ffffff',
                          borderColor: isFlash
                            ? '#f97316'
                            : isActive
                              ? '#06b6d4'
                              : '#cbd5e1',
                          boxShadow: isFlash
                            ? '0 0 0 2px rgba(249,115,22,0.12), 0 20px 28px rgba(249,115,22,0.18)'
                            : isActive
                              ? '0 0 0 2px rgba(34,211,238,0.12), 0 16px 24px rgba(34,211,238,0.18)'
                              : '0 10px 18px rgba(15,23,42,0.04)',
                        }}
                        transition={{
                          type: 'spring',
                          stiffness: 260,
                          damping: 18,
                          mass: 0.8,
                        }}
                        className="flex min-w-[70px] flex-1 shrink-0 flex-col items-center rounded-2xl border bg-white px-2 py-2 text-center shadow-sm"
                      >
                        <span className="mb-2 text-[10px] font-bold uppercase tracking-[0.2em] text-slate-500">
                          D{index + 1}
                        </span>
                        <div className="flex h-24 w-full items-end justify-center rounded-xl bg-slate-100/80 px-1 py-2">
                          <span className="text-[11px] font-bold text-slate-900 sm:text-sm">
                            {value.toLocaleString()}
                          </span>
                        </div>
                      </motion.div>
                    )
                  })}
                </div>
              </div>

              <div className="mt-6 rounded-2xl border border-cyan-100 bg-cyan-50 p-4">
                <div className="flex items-center justify-between gap-3">
                  <span className="text-[10px] font-bold uppercase tracking-[0.28em] text-cyan-700">
                    Running total
                  </span>
                  <span className="text-2xl font-bold text-slate-900">
                    {runningTotal.toLocaleString()}
                  </span>
                </div>

                <p className="mt-2 text-sm text-slate-600">
                  {isQuerying
                    ? `Scanning days ${Math.min(queryStart, queryEnd)} through ${Math.max(queryStart, queryEnd)}...`
                    : `Range sum for [${Math.min(queryStart, queryEnd)}, ${Math.max(queryStart, queryEnd)}] = ${currentRangeTotal.toLocaleString()}`}
                </p>
              </div>
            </div>
          </div>
        </div>

        <div className="col-span-3">
          <div className="bg-white rounded-2xl shadow-xl p-5 w-full h-full">
            <div className="space-y-5">
              <div>
                <label className="mb-2 block text-xs font-semibold uppercase tracking-[0.25em] text-slate-500">
                  Point Update
                </label>
                <div className="flex gap-2">
                  <input
                    type="number"
                    min="1"
                    max="14"
                    value={updateDay}
                    onChange={(event) => setUpdateDay(Math.min(14, Math.max(1, Number(event.target.value) || 1)))}
                    className="w-20 rounded-xl border border-slate-300 bg-white px-3 py-2 text-sm font-medium text-slate-900 outline-none ring-0 transition focus:border-cyan-400"
                  />
                  <button
                    type="button"
                    onClick={handlePointUpdate}
                    className="flex-1 rounded-xl bg-orange-500 px-3 py-2 text-sm font-semibold text-white shadow-sm transition hover:bg-orange-600"
                  >
                    Update Day {Number(updateDay)}
                  </button>
                </div>
                <p className="mt-2 text-xs text-slate-500">
                  {lastUpdated !== null ? `Last update: Day ${lastUpdated + 1} (+1500 steps)` : 'No update yet'}
                </p>
              </div>

              <div className="border-t border-slate-200 pt-5">
                <label className="mb-2 block text-xs font-semibold uppercase tracking-[0.25em] text-slate-500">
                  Range Sum Query
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
                  onClick={handleRangeQuery}
                  disabled={isQuerying}
                  className="mt-3 w-full rounded-xl bg-cyan-500 px-3 py-2 text-sm font-semibold text-white shadow-sm transition hover:bg-cyan-600 disabled:cursor-not-allowed disabled:bg-cyan-300"
                >
                  {isQuerying ? 'Running Query...' : 'Run Query'}
                </button>

                <p className="mt-2 text-xs text-slate-500">
                  Traversal from Day {Math.min(queryStart, queryEnd)} to Day {Math.max(queryStart, queryEnd)}
                </p>
              </div>

              <div className="border-t border-slate-200 pt-5">
                <div className="mb-2 text-xs font-semibold uppercase tracking-[0.25em] text-slate-500">
                  Complexity
                </div>
                <div className="space-y-2 text-sm text-slate-700">
                  <div className="rounded-xl border border-orange-200 bg-orange-50 px-3 py-2">
                    <span className="font-semibold text-orange-700">Point update:</span> O(1)
                  </div>
                  <div className="rounded-xl border border-cyan-200 bg-cyan-50 px-3 py-2">
                    <span className="font-semibold text-cyan-700">Range sum:</span> O(n)
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  )
}
