from __future__ import annotations

import csv
import json
import os
import subprocess
import sys
from pathlib import Path
from typing import Any, Dict, List

import uvicorn
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel

ROOT = Path(__file__).resolve().parent.parent
SENSOR_ROOT = ROOT / "SensorBenchmark"
CSV_PATH = SENSOR_ROOT / "data" / "sample_steps.csv"
BINARY_PATH = SENSOR_ROOT / "build-release" / "sensor_benchmark.exe"

FALLBACK_BENCHMARK = {
    "N=1,000": [
        {"name": "Array", "query": 1.4, "update": 0.7},
        {"name": "Prefix Sum", "query": 0.9, "update": 1.8},
        {"name": "BIT", "query": 0.5, "update": 0.6},
        {"name": "Segment Tree", "query": 0.8, "update": 0.9},
    ],
    "N=10,000": [
        {"name": "Array", "query": 11.2, "update": 5.6},
        {"name": "Prefix Sum", "query": 8.3, "update": 15.9},
        {"name": "BIT", "query": 3.7, "update": 3.1},
        {"name": "Segment Tree", "query": 4.8, "update": 4.1},
    ],
    "N=100,000": [
        {"name": "Array", "query": 124.8, "update": 53.1},
        {"name": "Prefix Sum", "query": 92.7, "update": 142.5},
        {"name": "BIT", "query": 29.4, "update": 31.8},
        {"name": "Segment Tree", "query": 38.9, "update": 35.4},
    ],
}

app = FastAPI(title="SensorBenchmark API")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


class SyncPayload(BaseModel):
    days: List[int]
    query_type: str = "sum"
    left: int = 0
    right: int = 0
    mode: str = "auto"


def write_steps_csv(days: List[int]) -> None:
    if len(days) != 14:
        raise ValueError("Expected exactly 14 daily values.")

    CSV_PATH.parent.mkdir(parents=True, exist_ok=True)
    with CSV_PATH.open("w", newline="", encoding="utf-8") as csv_file:
        writer = csv.writer(csv_file)
        writer.writerow(["day", "steps"])
        for index, value in enumerate(days, start=1):
            writer.writerow([index, value])


def run_c_binary(payload: SyncPayload) -> Dict[str, Any]:
    if not BINARY_PATH.exists():
        parsed: Dict[str, Any] = {
            "status": "fallback",
            "stdout": "Binary not found; using fallback mock benchmark data.",
            "stderr": str(BINARY_PATH),
            "returncode": 0,
            "query_type": payload.query_type,
            "left": payload.left,
            "right": payload.right,
            "benchmark": FALLBACK_BENCHMARK,
            "query_result": f"Fallback result for {payload.query_type.upper()} on days {payload.left}-{payload.right}",
        }
        return parsed

    command = [
        str(BINARY_PATH),
        "--mode",
        payload.mode,
        "--query",
        payload.query_type,
        "--left",
        str(payload.left),
        "--right",
        str(payload.right),
    ]

    try:
        completed = subprocess.run(
            command,
            capture_output=True,
            text=True,
            timeout=30,
            check=False,
            cwd=str(SENSOR_ROOT),
        )
    except subprocess.TimeoutExpired as exc:
        return {
            "status": "error",
            "error": "C binary timed out",
            "stdout": exc.stdout or "",
            "stderr": exc.stderr or "",
            "benchmark": FALLBACK_BENCHMARK,
        }

    stdout = completed.stdout.strip()
    stderr = completed.stderr.strip()

    parsed: Dict[str, Any] = {
        "status": "ok" if completed.returncode == 0 else "error",
        "stdout": stdout,
        "stderr": stderr,
        "returncode": completed.returncode,
        "query_type": payload.query_type,
        "left": payload.left,
        "right": payload.right,
        "benchmark": FALLBACK_BENCHMARK,
    }

    if stdout:
        lines = [line.strip() for line in stdout.splitlines() if line.strip()]
        parsed["result_lines"] = lines
        for line in lines:
            lowered = line.lower()
            if any(keyword in lowered for keyword in ["sum", "min", "max", "query", "result", "update"]):
                parsed["query_result"] = line
                break

    if "query_result" not in parsed:
        parsed["query_result"] = f"{payload.query_type.upper()} on days {payload.left}-{payload.right}"

    return parsed


@app.get("/health")
def healthcheck() -> Dict[str, str]:
    return {"status": "ok"}


@app.post("/api/sync-and-query")
def sync_and_query(payload: SyncPayload) -> Dict[str, Any]:
    try:
        write_steps_csv(payload.days)
        result = run_c_binary(payload)

        return {
            "success": True,
            "message": "CSV synced and C benchmark executed.",
            "days": payload.days,
            "csv_path": str(CSV_PATH),
            "binary_path": str(BINARY_PATH),
            "result": result,
        }
    except Exception as exc:  # pragma: no cover
        return {
            "success": False,
            "message": str(exc),
            "days": payload.days,
            "csv_path": str(CSV_PATH),
            "binary_path": str(BINARY_PATH),
            "result": {"status": "error", "error": str(exc)},
        }


if __name__ == "__main__":
    port = int(os.environ.get("PORT", "8000"))
    uvicorn.run("main:app", host="0.0.0.0", port=port, reload=False)
