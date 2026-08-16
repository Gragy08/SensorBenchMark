import { useMemo, useState } from 'react'
import { motion } from 'framer-motion'

const INITIAL_STEPS = [
  6200, 8400, 7100, 9100, 8000, 9600, 7500,
  8800, 10200, 9300, 7900, 8700, 9900, 7400,
]

const delay = (ms) => new Promise((resolve) => setTimeout(resolve, ms))

export default function PrefixSumDemo() {
  const [steps, setSteps] = useState(INITIAL_STEPS)
  const [updateDay, setUpdateDay] = useState(4)
  const [queryStart, setQueryStart] = useState(2)
  const [queryEnd, setQueryEnd] = useState(9)
  const [highlightedOriginal, setHighlightedOriginal] = useState(null)
  const [highlightedPrefix, setHighlightedPrefix] = useState([])
  const [runningResult, setRunningResult] = useState(0)
  const [isUpdating, setIsUpdating] = useState(false)
  const [message, setMessage] = useState('Ready to query a range.')

  const prefixValues = useMemo(() => {
    let running = 0
    return steps.map((value) => {
      running += value
      return running
    })
  }, [steps])

  const computeRangeSum = (left, right) => {
    const safeLeft = Math.max(0, left)
    const safeRight = Math.min(prefixValues.length - 1, right)

    if (safeLeft === 0) {
      return prefixValues[safeRight]
    }

    return prefixValues[safeRight] - prefixValues[safeLeft - 1]
  }

  const handleRangeQuery = () => {
    const left = Math.min(queryStart, queryEnd)
    const right = Math.max(queryStart, queryEnd)

    setHighlightedPrefix([
      right,
      left > 0 ? left - 1 : null,
    ].filter(Boolean))
    setHighlightedOriginal([])
    setRunningResult(computeRangeSum(left, right))
    setMessage(`prefix[${right}] - prefix[${left - 1 < 0 ? '0' : left - 1}] = ${computeRangeSum(left, right)}`)
  }

  const handlePointUpdate = async () => {
    const targetIndex = Math.min(Math.max(Number(updateDay) || 0, 0), steps.length - 1)
    setIsUpdating(true)
    setHighlightedOriginal([targetIndex])
    setHighlightedPrefix([])
    setMessage(`Updating original value at Day ${targetIndex + 1}...`)

    const adjustedSteps = steps.map((value, index) =>
      index === targetIndex ? value + 1500 : value,
    )

    setSteps(adjustedSteps)
    setTimeout(() => setHighlightedOriginal(null), 500)

    await delay(200)

    for (let index = targetIndex; index < adjustedSteps.length; index += 1) {
      setHighlightedPrefix([index])
      setMessage(`Updating prefix sum at index ${index}... ($O(n)$ ripple)`) 
      await delay(500)
    }

    setHighlightedPrefix([])
    setMessage(`Updating all subsequent prefix sums... ($O(n)$)`) 
    setIsUpdating(false)
  }

  const leftBound = Math.min(queryStart, queryEnd)
  const rightBound = Math.max(queryStart, queryEnd)
  const formulaResult = computeRangeSum(leftBound, rightBound)

  return (
    <div className="min-h-screen bg-slate-100 px-4 py-8 text-slate-900 antialiased">
      <div className="mx-auto max-w-6xl rounded-[28px] border border-slate-200 bg-white p-6 shadow-[0_20px_70px_rgba(15,23,42,0.08)] sm:p-8">
        <header className="mb-8 text-center">
          <p className="text-xs font-semibold uppercase tracking-[0.35em] text-cyan-500">
            Case Study 2
          </p>
          <h2 className="mt-3 text-3xl font-bold text-slate-900 sm:text-4xl">
            Prefix Sum
          </h2>
          <p className="mx-auto mt-3 max-w-2xl text-sm text-slate-600 sm:text-base">
            Range sum is <span className="font-semibold text-slate-900">O(1)</span> with direct prefix subtraction,
            while a point update forces a full suffix recomputation in <span className="font-semibold text-slate-900">O(n)</span>.
          </p>
        </header>

        <div className="grid gap-8 xl:grid-cols-[1.4fr_0.6fr]">
          <div className="space-y-8">
            <section className="rounded-2xl border border-slate-200 bg-slate-50 p-4">
              <div className="mb-4 flex items-center justify-between">
                <h3 className="text-sm font-semibold uppercase tracking-[0.25em] text-slate-500">
                  Original Array (Steps)
                </h3>
                <span className="text-xs text-slate-500">14 days</span>
              </div>

              <div className="flex gap-3 overflow-x-auto pb-2">
                {steps.map((value, index) => {
                  const isHighlighted = highlightedOriginal === index

                  return (
                    <motion.div
                      key={`original-${index}`}
                      animate={{
                        scale: isHighlighted ? 1.12 : 1,
                        backgroundColor: isHighlighted ? '#f97316' : '#ffffff',
                        borderColor: isHighlighted ? '#f97316' : '#cbd5e1',
                        boxShadow: isHighlighted
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

            <section className="rounded-2xl border border-slate-200 bg-slate-50 p-4">
              <div className="mb-4 flex items-center justify-between">
                <h3 className="text-sm font-semibold uppercase tracking-[0.25em] text-slate-500">
                  Prefix Sum Array
                </h3>
                <span className="text-xs text-cyan-700">prefix[i] = sum(steps[0..i])</span>
              </div>

              <div className="flex gap-3 overflow-x-auto pb-2">
                {prefixValues.map((value, index) => {
                  const isHighlighted = highlightedPrefix.includes(index)

                  return (
                    <motion.div
                      key={`prefix-${index}`}
                      animate={{
                        scale: isHighlighted ? 1.08 : 1,
                        backgroundColor: isHighlighted ? '#22d3ee' : '#ffffff',
                        borderColor: isHighlighted ? '#06b6d4' : '#cbd5e1',
                        boxShadow: isHighlighted
                          ? '0 0 0 2px rgba(34,211,238,0.12), 0 16px 24px rgba(34,211,238,0.18)'
                          : '0 10px 14px rgba(15,23,42,0.04)',
                      }}
                      transition={{ type: 'spring', stiffness: 260, damping: 18 }}
                      className="flex w-14 min-w-[3.5rem] flex-col items-center rounded-2xl border p-2"
                    >
                      <span className="mb-2 text-[10px] font-bold uppercase tracking-[0.22em] text-slate-500">
                        P{index + 1}
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

            <div className="rounded-2xl border border-cyan-100 bg-cyan-50 p-4">
              <div className="flex items-center justify-between gap-3">
                <span className="text-[10px] font-bold uppercase tracking-[0.28em] text-cyan-700">
                  Formula
                </span>
                <span className="text-sm font-semibold text-slate-700">
                  {message}
                </span>
              </div>

              <div className="mt-4 text-center text-xl font-bold text-slate-900 sm:text-2xl">
                prefix[{rightBound}] - prefix[{leftBound > 0 ? leftBound - 1 : 0}] = {formulaResult.toLocaleString()}
              </div>
            </div>
          </div>

          <div className="space-y-5 rounded-2xl border border-slate-200 bg-slate-50 p-4 sm:p-5">
            <div>
              <label className="mb-2 block text-xs font-semibold uppercase tracking-[0.25em] text-slate-500">
                Range Sum Query
              </label>
              <div className="grid grid-cols-2 gap-2">
                <input
                  type="number"
                  min="0"
                  max="13"
                  value={queryStart}
                  onChange={(event) => setQueryStart(Math.min(13, Math.max(0, Number(event.target.value) || 0)))}
                  className="rounded-xl border border-slate-300 bg-white px-3 py-2 text-sm font-medium text-slate-900 outline-none transition focus:border-cyan-400"
                  placeholder="L"
                />
                <input
                  type="number"
                  min="0"
                  max="13"
                  value={queryEnd}
                  onChange={(event) => setQueryEnd(Math.min(13, Math.max(0, Number(event.target.value) || 0)))}
                  className="rounded-xl border border-slate-300 bg-white px-3 py-2 text-sm font-medium text-slate-900 outline-none transition focus:border-cyan-400"
                  placeholder="R"
                />
              </div>

              <button
                type="button"
                onClick={handleRangeQuery}
                className="mt-3 w-full rounded-xl bg-cyan-500 px-3 py-2 text-sm font-semibold text-white shadow-sm transition hover:bg-cyan-600"
              >
                Run Query
              </button>
            </div>

            <div className="border-t border-slate-200 pt-5">
              <label className="mb-2 block text-xs font-semibold uppercase tracking-[0.25em] text-slate-500">
                Point Update
              </label>
              <div className="flex gap-2">
                <input
                  type="number"
                  min="0"
                  max="13"
                  value={updateDay}
                  onChange={(event) => setUpdateDay(Math.min(13, Math.max(0, Number(event.target.value) || 0)))}
                  className="w-20 rounded-xl border border-slate-300 bg-white px-3 py-2 text-sm font-medium text-slate-900 outline-none transition focus:border-cyan-400"
                />
                <button
                  type="button"
                  disabled={isUpdating}
                  onClick={() => void handlePointUpdate()}
                  className="flex-1 rounded-xl bg-orange-500 px-3 py-2 text-sm font-semibold text-white shadow-sm transition hover:bg-orange-600 disabled:cursor-not-allowed disabled:bg-orange-300"
                >
                  {isUpdating ? 'Updating...' : `Update Day ${Number(updateDay) + 1}`}
                </button>
              </div>

              <p className="mt-3 text-xs text-slate-500">
                {isUpdating
                  ? 'Updating all subsequent prefix sums... (O(n))'
                  : 'Single day update triggers a full suffix recomputation.'}
              </p>
            </div>

            <div className="border-t border-slate-200 pt-5">
              <div className="mb-2 text-xs font-semibold uppercase tracking-[0.25em] text-slate-500">
                Complexity
              </div>
              <div className="space-y-2 text-sm text-slate-700">
                <div className="rounded-xl border border-cyan-200 bg-cyan-50 px-3 py-2">
                  <span className="font-semibold text-cyan-700">Range sum:</span> O(1)
                </div>
                <div className="rounded-xl border border-orange-200 bg-orange-50 px-3 py-2">
                  <span className="font-semibold text-orange-700">Point update:</span> O(n)
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  )
}
