import { useMemo, useState } from 'react'
import { motion } from 'framer-motion'
import {
  Bar,
  BarChart,
  CartesianGrid,
  Legend,
  ResponsiveContainer,
  Tooltip,
  XAxis,
  YAxis,
} from 'recharts'
import ArrayDemo from './components/ArrayDemo'
import PrefixSumDemo from './components/PrefixSumDemo'
import BITDemo from './components/BITDemo'
import SegmentTreeDemo from './components/SegmentTreeDemo'

const initialSteps = [
  6200, 8400, 7100, 9100, 8000, 9600, 7500,
  8800, 10200, 9300, 7900, 8700, 9900, 7400,
]

const queryOptions = [
  { id: 'sum', label: 'Range Sum [L, R]', accent: 'cyan' },
  { id: 'min', label: 'Range Min [L, R]', accent: 'cyan' },
  { id: 'max', label: 'Range Max [L, R]', accent: 'cyan' },
  { id: 'point', label: 'Point Update', accent: 'orange' },
]

const realBenchmarkData = {
  'N=1000': [
    { name: 'Array', query: 0.260, update: 0.019 },
    { name: 'Prefix Sum', query: 0.014, update: 0.098 },
    { name: 'BIT', query: 0.016, update: 0.020 },
    { name: 'Segment Tree', query: 0.260, update: 0.073 },
  ],
  'N=10000': [
    { name: 'Array', query: 21.556, update: 0.193 },
    { name: 'Prefix Sum', query: 0.143, update: 6.775 },
    { name: 'BIT', query: 0.170, update: 0.319 },
    { name: 'Segment Tree', query: 3.282, update: 1.038 },
  ],
  'N=100000': [
    { name: 'Array', query: 2013.588, update: 2.175 },
    { name: 'Prefix Sum', query: 1.509, update: 568.914 },
    { name: 'BIT', query: 1.912, update: 3.545 },
    { name: 'Segment Tree', query: 51.282, update: 16.273 },
  ],
}

const benchmarkProfiles = realBenchmarkData

const API_BASE_URL = import.meta.env.VITE_API_BASE_URL || 'http://localhost:8000'

const clamp = (value, min, max) => Math.min(Math.max(value, min), max)

const buildPrefixValues = (values) => {
  const prefix = []
  let runningSum = 0

  values.forEach((value) => {
    runningSum += value
    prefix.push(runningSum)
  })

  return prefix
}

const buildSegmentLevels = (values) => {
  const levels = [values]
  let current = [...values]

  while (current.length > 1) {
    const next = []
    for (let index = 0; index < current.length; index += 2) {
      next.push((current[index] ?? 0) + (current[index + 1] ?? 0))
    }
    current = next
    levels.push(current)
  }

  return levels
}

const buildBitLevels = (values) => {
  const levels = []
  const total = values.reduce((sum, value) => sum + value, 0)

  levels.push([total])
  levels.push([
    values.slice(0, 7).reduce((sum, value) => sum + value, 0),
    values.slice(7).reduce((sum, value) => sum + value, 0),
  ])
  levels.push([
    values.slice(0, 3).reduce((sum, value) => sum + value, 0),
    values.slice(3, 7).reduce((sum, value) => sum + value, 0),
    values.slice(7, 11).reduce((sum, value) => sum + value, 0),
    values.slice(11).reduce((sum, value) => sum + value, 0),
  ])
  levels.push([...values])

  return levels
}

const tabs = [
  { id: 'overview', label: 'Overview' },
  { id: 'array', label: 'Case Study 1: Array' },
  { id: 'prefix-sum', label: 'Case Study 2: Prefix Sum' },
  { id: 'bit', label: 'Case Study 3: BIT' },
  { id: 'segment-tree', label: 'Case Study 4: Segment Tree' },
]

function PlaceholderView({ title }) {
  return (
    <div className="flex min-h-[60vh] items-center justify-center bg-slate-100 px-4">
      <div className="rounded-2xl border border-slate-200 bg-white px-8 py-12 text-center shadow-sm">
        <p className="text-xs font-semibold uppercase tracking-[0.3em] text-cyan-500">Coming Soon</p>
        <h2 className="mt-3 text-2xl font-bold text-slate-900">{title}</h2>
        <p className="mt-2 text-sm text-slate-600">
          Placeholder view ready for the next DSA case study implementation.
        </p>
      </div>
    </div>
  )
}

function OverviewDashboard() {
  const [steps, setSteps] = useState(initialSteps)
  const [queryMode, setQueryMode] = useState('sum')
  const [range, setRange] = useState([3, 9])
  const [selectedDay, setSelectedDay] = useState(6)
  const [benchmarkScale, setBenchmarkScale] = useState('N=10000')
  const [activePath, setActivePath] = useState([6, 3, 1])
  const [isSyncing, setIsSyncing] = useState(false)
  const [backendData, setBackendData] = useState(null)
  const [backendError, setBackendError] = useState('')

  const prefixValues = useMemo(() => buildPrefixValues(steps), [steps])
  const segmentLevels = useMemo(() => buildSegmentLevels(steps), [steps])
  const bitLevels = useMemo(() => buildBitLevels(steps), [steps])

  const chartData = useMemo(() => {
    const candidate = backendData?.result?.benchmark?.[benchmarkScale]
    return Array.isArray(candidate) ? candidate : benchmarkProfiles[benchmarkScale]
  }, [backendData, benchmarkScale])

  const querySummary = useMemo(() => {
    const backendResult = backendData?.result?.query_result
    if (backendResult) {
      return backendResult
    }

    const start = Math.min(range[0], range[1])
    const end = Math.max(range[0], range[1])
    const selected = steps.slice(start, end + 1)

    if (queryMode === 'sum') {
      return `Sum = ${selected.reduce((sum, value) => sum + value, 0).toLocaleString()}`
    }
    if (queryMode === 'min') {
      return `Min = ${Math.min(...selected).toLocaleString()}`
    }
    if (queryMode === 'max') {
      return `Max = ${Math.max(...selected).toLocaleString()}`
    }
    return `Updated day ${selectedDay + 1} → ${steps[selectedDay].toLocaleString()}`
  }, [backendData, queryMode, range, selectedDay, steps])

  const handleQuery = (mode, start, end) => {
    setQueryMode(mode)
    setRange([start, end])
    setActivePath([])
    void syncToBackend(mode, [start, end])
  }

  const syncToBackend = async (mode = queryMode, nextRange = range, customDays = steps) => {
    const start = Math.min(nextRange[0], nextRange[1])
    const end = Math.max(nextRange[0], nextRange[1])

    const payload = {
      days: customDays,
      query_type: mode,
      left: start + 1,
      right: end + 1,
      mode: 'auto',
    }

    setIsSyncing(true)
    setBackendError('')

    try {
      const response = await fetch(`${API_BASE_URL}/api/sync-and-query`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload),
      })

      const data = await response.json()

      if (!response.ok || !data.success) {
        throw new Error(data.message || 'Failed to sync with backend')
      }

      setBackendData(data)

      if (Array.isArray(data.days) && data.days.length === 14) {
        setSteps(data.days.map(Number))
      }

      if (data.result?.left && data.result?.right) {
        setRange([data.result.left - 1, data.result.right - 1])
      }
    } catch (error) {
      setBackendError(error.message || 'Unable to reach backend')
    } finally {
      setIsSyncing(false)
    }
  }

  const handlePointUpdate = async () => {
    const nextSelectedDay = clamp(selectedDay + 1, 0, steps.length - 1)
    const updatedSteps = steps.map((value, index) =>
      index === nextSelectedDay ? value + 1500 : value,
    )

    setSelectedDay(nextSelectedDay)
    setSteps(updatedSteps)
    setActivePath([nextSelectedDay, Math.floor(nextSelectedDay / 2), Math.floor((nextSelectedDay / 2) / 2), 0])

    await syncToBackend('point', [nextSelectedDay, nextSelectedDay], updatedSteps)
  }

  const activeRangeMap = useMemo(() => {
    const start = Math.min(range[0], range[1])
    const end = Math.max(range[0], range[1])
    return new Set(Array.from({ length: end - start + 1 }, (_, offset) => start + offset))
  }, [range])

  return (
    <div className="min-h-screen bg-slate-100 text-slate-900 antialiased">
      <div className="mx-auto max-w-7xl px-4 py-6 sm:px-6 lg:px-8">
        <header className="mb-6 flex flex-col gap-4 rounded-2xl border border-slate-200 bg-white p-5 shadow-sm">
          <div className="flex items-center justify-between gap-3">
            <div>
              <p className="text-xs font-semibold uppercase tracking-[0.25em] text-cyan-500">
                Daily Step Tracking
              </p>
              <h1 className="mt-2 text-3xl font-bold text-slate-900">
                DSA Presentation Dashboard
              </h1>
            </div>
            <div className="rounded-full border border-cyan-200 bg-cyan-50 px-3 py-1 text-sm font-medium text-cyan-700">
              {querySummary}
            </div>
          </div>
        </header>

        <main className="space-y-8">
          <section className="rounded-2xl border border-slate-200 bg-white p-5 shadow-sm">
            <div className="mb-4 flex items-center justify-between gap-3">
              <div>
                <p className="text-sm font-semibold uppercase tracking-[0.2em] text-slate-500">
                  Input Section
                </p>
                <h2 className="mt-1 text-xl font-semibold text-slate-800">14-Day Step Log</h2>
              </div>
              <div className="flex items-center gap-2">
                <button
                  type="button"
                  className="rounded-full border border-cyan-200 bg-cyan-50 px-3 py-1.5 text-xs font-semibold uppercase tracking-[0.15em] text-cyan-700 disabled:opacity-60"
                  onClick={() => void syncToBackend()}
                  disabled={isSyncing}
                >
                  {isSyncing ? 'Syncing...' : 'Sync Data'}
                </button>
                <button
                  type="button"
                  className="rounded-full border border-orange-200 bg-orange-50 px-3 py-1.5 text-xs font-semibold uppercase tracking-[0.15em] text-orange-600"
                  onClick={() => void handlePointUpdate()}
                >
                  Simulate Point Update
                </button>
              </div>
            </div>

            {backendError ? (
              <div className="mb-4 rounded-xl border border-red-200 bg-red-50 px-3 py-2 text-sm text-red-700">
                {backendError}
              </div>
            ) : null}

            <div className="flex gap-3 overflow-x-auto pb-2">
              {steps.map((value, index) => (
                <motion.div
                  key={`day-${index}`}
                  layout
                  className={`min-w-[110px] rounded-2xl border p-3 shadow-sm transition-all ${
                    selectedDay === index
                      ? 'border-orange-400 bg-orange-50 shadow-orange-100'
                      : 'border-slate-200 bg-slate-50'
                  }`}
                >
                  <div className="mb-3 flex items-center justify-between text-xs font-medium text-slate-500">
                    <span>Day {index + 1}</span>
                    <span className="rounded-full bg-slate-200 px-2 py-0.5">#{index + 1}</span>
                  </div>
                  <input
                    type="number"
                    value={value}
                    onChange={(event) => {
                      const next = Number(event.target.value || 0)
                      setSteps((current) => current.map((item, itemIndex) => (itemIndex === index ? next : item)))
                    }}
                    className="w-full rounded-xl border border-slate-200 bg-white px-2 py-2 text-lg font-semibold text-slate-800 outline-none ring-0 focus:border-cyan-400"
                  />
                  <div className="mt-2 flex items-center justify-between text-[11px] text-slate-400">
                    <span>Steps</span>
                    <button
                      type="button"
                      className="text-cyan-600 underline-offset-2 hover:underline"
                      onClick={() => setSelectedDay(index)}
                    >
                      Inspect
                    </button>
                  </div>
                </motion.div>
              ))}
            </div>
          </section>

          <section className="rounded-2xl border border-slate-200 bg-white p-5 shadow-sm">
            <div className="mb-4 flex flex-wrap items-center gap-3">
              {queryOptions.map((option) => (
                <button
                  key={option.id}
                  type="button"
                  className={`rounded-full border px-4 py-2 text-sm font-medium transition ${
                    queryMode === option.id
                      ? 'border-cyan-500 bg-cyan-500 text-white shadow-lg shadow-cyan-500/20'
                      : 'border-slate-200 bg-slate-50 text-slate-700 hover:border-cyan-300 hover:bg-cyan-50'
                  }`}
                  onClick={() => {
                    if (option.id === 'point') {
                      handlePointUpdate()
                    } else {
                      handleQuery(option.id, 2, 10)
                    }
                  }}
                >
                  {option.label}
                </button>
              ))}
            </div>

            <div className="grid gap-6 xl:grid-cols-[1.2fr_1fr]">
              <div className="rounded-2xl border border-slate-200 bg-slate-50 p-4">
                <div className="mb-3 flex items-center justify-between">
                  <h3 className="text-sm font-semibold uppercase tracking-[0.2em] text-slate-500">
                    Array + Prefix Sum
                  </h3>
                  <span className="text-xs text-cyan-600">Active query: {queryMode}</span>
                </div>

                <div className="space-y-4">
                  <div className="grid grid-cols-7 gap-2 sm:grid-cols-7">
                    {steps.map((value, index) => {
                      const isActive = activeRangeMap.has(index)
                      return (
                        <motion.div
                          key={`array-${index}`}
                          layout
                          className={`flex min-h-[74px] flex-col items-center justify-center rounded-xl border text-center text-sm font-semibold transition ${
                            isActive
                              ? 'border-cyan-400 bg-cyan-500 text-white shadow-md shadow-cyan-500/30'
                              : 'border-slate-200 bg-white text-slate-700'
                          }`}
                        >
                          <span className="text-[10px] uppercase tracking-[0.2em]">D{index + 1}</span>
                          <span className="mt-1 text-base">{value.toLocaleString()}</span>
                        </motion.div>
                      )
                    })}
                  </div>

                  <div className="grid grid-cols-7 gap-2 sm:grid-cols-7">
                    {prefixValues.map((value, index) => {
                      const isActive = activeRangeMap.has(index)
                      return (
                        <motion.div
                          key={`prefix-${index}`}
                          layout
                          className={`flex min-h-[58px] flex-col items-center justify-center rounded-xl border text-center text-xs font-semibold transition ${
                            isActive
                              ? 'border-cyan-400 bg-cyan-500 text-white'
                              : 'border-slate-200 bg-slate-100 text-slate-600'
                          }`}
                        >
                          <span className="text-[10px] uppercase tracking-[0.2em]">P{index + 1}</span>
                          <span className="mt-1">{value.toLocaleString()}</span>
                        </motion.div>
                      )
                    })}
                  </div>
                </div>
              </div>

              <div className="rounded-2xl border border-slate-200 bg-slate-50 p-4">
                <h3 className="text-sm font-semibold uppercase tracking-[0.2em] text-slate-500">
                  Query Result
                </h3>
                <div className="mt-4 space-y-4">
                  <div className="rounded-xl border border-cyan-200 bg-cyan-50 p-3">
                    <p className="text-xs font-medium uppercase tracking-[0.2em] text-cyan-700">Range</p>
                    <p className="mt-2 text-lg font-bold text-cyan-900">
                      [{range[0] + 1}, {range[1] + 1}]
                    </p>
                  </div>
                  <div className="rounded-xl border border-slate-200 bg-white p-3">
                    <p className="text-xs font-medium uppercase tracking-[0.2em] text-slate-500">Result</p>
                    <p className="mt-2 text-xl font-bold text-slate-800">{querySummary}</p>
                  </div>
                  <div className="rounded-xl border border-slate-200 bg-white p-3">
                    <p className="text-xs font-medium uppercase tracking-[0.2em] text-slate-500">Update</p>
                    <p className="mt-2 text-lg font-semibold text-orange-600">
                      Day {selectedDay + 1} → +1500 steps
                    </p>
                  </div>
                </div>
              </div>
            </div>
          </section>

          <section className="grid gap-6 xl:grid-cols-2">
            <div className="rounded-2xl border border-slate-200 bg-slate-950 p-5 shadow-lg shadow-slate-950/20">
              <div className="mb-5 flex items-center justify-between">
                <div>
                  <p className="text-xs font-semibold uppercase tracking-[0.2em] text-cyan-400">
                    Binary Indexed Tree
                  </p>
                  <h3 className="mt-1 text-xl font-semibold text-white">Fenwick Tree</h3>
                </div>
                <span className="rounded-full border border-cyan-700 bg-cyan-500/10 px-2 py-1 text-xs text-cyan-300">
                  O(log n)
                </span>
              </div>

              <div className="space-y-4">
                {bitLevels.map((level, levelIndex) => (
                  <div key={`bit-level-${levelIndex}`} className="flex justify-center gap-3">
                    {level.map((value, valueIndex) => {
                      const nodeId = `${levelIndex}-${valueIndex}`
                      const isActive = activePath.includes(valueIndex) || activePath.includes(0)
                      return (
                        <motion.div
                          key={nodeId}
                          initial={{ opacity: 0.5, scale: 0.9 }}
                          animate={{
                            scale: isActive ? 1.08 : 1,
                            backgroundColor: isActive ? '#f97316' : '#0f172a',
                            borderColor: isActive ? '#fdba74' : '#67e8f9',
                            color: '#e2e8f0',
                          }}
                          transition={{ duration: 0.35 }}
                          className="flex h-12 w-12 items-center justify-center rounded-full border text-xs font-bold shadow-inner shadow-cyan-500/20"
                        >
                          {value}
                        </motion.div>
                      )
                    })}
                  </div>
                ))}
              </div>
            </div>

            <div className="rounded-2xl border border-slate-200 bg-slate-950 p-5 shadow-lg shadow-slate-950/20">
              <div className="mb-5 flex items-center justify-between">
                <div>
                  <p className="text-xs font-semibold uppercase tracking-[0.2em] text-cyan-400">
                    Segment Tree
                  </p>
                  <h3 className="mt-1 text-xl font-semibold text-white">Range Query Tree</h3>
                </div>
                <span className="rounded-full border border-cyan-700 bg-cyan-500/10 px-2 py-1 text-xs text-cyan-300">
                  O(log n)
                </span>
              </div>

              <div className="space-y-4">
                {segmentLevels.map((level, levelIndex) => (
                  <div key={`segment-level-${levelIndex}`} className="flex justify-center gap-3">
                    {level.map((value, valueIndex) => {
                      const isActive = activePath.includes(valueIndex) || activePath.includes(0)
                      return (
                        <motion.div
                          key={`segment-${levelIndex}-${valueIndex}`}
                          initial={{ opacity: 0.6 }}
                          animate={{
                            scale: isActive ? 1.1 : 1,
                            backgroundColor: isActive ? '#f97316' : '#0f172a',
                            borderColor: isActive ? '#fdba74' : '#67e8f9',
                            color: '#e2e8f0',
                          }}
                          transition={{ duration: 0.35 }}
                          className="flex h-12 w-12 items-center justify-center rounded-full border text-xs font-bold shadow-inner shadow-cyan-500/20"
                        >
                          {value}
                        </motion.div>
                      )
                    })}
                  </div>
                ))}
              </div>
            </div>
          </section>

          <section className="rounded-2xl border border-slate-200 bg-white p-5 shadow-sm">
            <div className="mb-4 flex items-center justify-between gap-3">
              <div>
                <p className="text-sm font-semibold uppercase tracking-[0.2em] text-slate-500">
                  Performance Benchmark
                </p>
                <h2 className="mt-1 text-xl font-semibold text-slate-800">Query vs Update Cost</h2>
              </div>

              <div className="flex items-center gap-2 rounded-full border border-slate-200 bg-slate-100 p-1">
                {Object.keys(benchmarkProfiles).map((profile) => (
                  <button
                    key={profile}
                    type="button"
                    className={`rounded-full px-3 py-1.5 text-xs font-semibold uppercase tracking-[0.15em] transition ${
                      benchmarkScale === profile
                        ? 'bg-cyan-500 text-white shadow-md shadow-cyan-500/20'
                        : 'text-slate-600 hover:bg-slate-200'
                    }`}
                    onClick={() => setBenchmarkScale(profile)}
                  >
                    {profile}
                  </button>
                ))}
              </div>
            </div>

            <div className="h-80 w-full">
              <ResponsiveContainer width="100%" height="100%">
                <BarChart data={chartData}>
                  <CartesianGrid strokeDasharray="3 3" stroke="#cbd5e1" />
                  <XAxis dataKey="name" stroke="#475569" />
                  <YAxis stroke="#475569" domain={['auto', 'auto']} />
                  <Tooltip />
                  <Legend />
                  <Bar dataKey="query" name="Query Time (ms)" fill="#22d3ee" radius={[8, 8, 0, 0]} />
                  <Bar dataKey="update" name="Update Time (ms)" fill="#f97316" radius={[8, 8, 0, 0]} />
                </BarChart>
              </ResponsiveContainer>
            </div>
          </section>
        </main>
      </div>
    </div>
  )
}

function App() {
  const [activeTab, setActiveTab] = useState('overview')

  const renderTabContent = () => {
    switch (activeTab) {
      case 'overview':
        return <OverviewDashboard />
      case 'array':
        return <ArrayDemo />
      case 'prefix-sum':
        return <PrefixSumDemo />
      case 'bit':
        return <BITDemo />
      case 'segment-tree':
        return <SegmentTreeDemo />
      default:
        return <OverviewDashboard />
    }
  }

  return (
    <div className="min-h-screen bg-slate-100 text-slate-900 antialiased">
      <header className="sticky top-0 z-50 border-b border-slate-200 bg-slate-950/95 backdrop-blur-md">
        <div className="mx-auto flex max-w-7xl items-center justify-between gap-3 px-4 py-3 sm:px-6 lg:px-8">
          <div className="flex items-center gap-2">
            <div className="h-2.5 w-2.5 rounded-full bg-cyan-400" />
            <span className="text-xs font-semibold uppercase tracking-[0.25em] text-cyan-300">
              DSA Dashboard
            </span>
          </div>

          <nav className="flex flex-wrap items-center justify-center gap-2">
            {tabs.map((tab) => {
              const isActive = activeTab === tab.id

              return (
                <button
                  key={tab.id}
                  type="button"
                  onClick={() => setActiveTab(tab.id)}
                  className={`rounded-full px-3 py-2 text-xs font-semibold transition sm:text-sm ${
                    isActive
                      ? 'bg-cyan-400 text-slate-950 shadow-lg shadow-cyan-500/30'
                      : 'bg-slate-800 text-slate-200 hover:bg-slate-700'
                  }`}
                >
                  {tab.label}
                </button>
              )
            })}
          </nav>
        </div>
      </header>

      <main>{renderTabContent()}</main>
    </div>
  )
}

export default App
