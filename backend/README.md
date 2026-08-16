# SensorBenchmark Backend API

This lightweight FastAPI app synchronizes the step CSV file and invokes the compiled C benchmark binary.

## Run

```bash
cd backend
python -m venv .venv
source .venv/bin/activate  # Linux/macOS
# or .venv\Scripts\activate  # Windows PowerShell
pip install -r requirements.txt
uvicorn main:app --host 0.0.0.0 --port 8000 --reload
```

## Endpoint

### POST /api/sync-and-query

Request body:

```json
{
  "days": [6200, 8400, 7100, 9100, 8000, 9600, 7500, 8800, 10200, 9300, 7900, 8700, 9900, 7400],
  "query_type": "sum",
  "left": 1,
  "right": 7,
  "mode": "auto"
}
```

The server will:

1. overwrite `SensorBenchmark/data/sample_steps.csv`
2. invoke `SensorBenchmark/build-release/sensor_benchmark.exe`
3. return the stdout/stderr + JSON metadata
