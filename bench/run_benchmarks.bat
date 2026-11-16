@echo off
setlocal

REM Always run from the project root, regardless of where this script is launched.
set "ROOT=%~dp0.."
pushd "%ROOT%"
if errorlevel 1 (
    echo ERROR: Could not open project directory.
    exit /b 1
)

if not exist "results" mkdir "results"

 echo [1/3] Compiling benchmark...
g++ -std=c++17 -O2 -DNDEBUG -Wall -Wextra ^
  "bench\benchmark.cpp" ^
  "Structure A\avl.cpp" ^
  "Structure B\bst.cpp" ^
  -o "bench\benchmark.exe"
if errorlevel 1 goto :error

 echo [2/3] Running benchmark...
"bench\benchmark.exe" "%ROOT%\results\benchmark_raw.csv"
if errorlevel 1 goto :error

if not exist "results\benchmark_raw.csv" (
    echo ERROR: Benchmark finished but results\benchmark_raw.csv was not created.
    goto :error
)

 echo [3/3] Generating plots...
python "bench\plot_results.py" "%ROOT%\results\benchmark_raw.csv"
if errorlevel 1 goto :error

echo.
echo Benchmark and plots completed successfully.
echo Results: %ROOT%\results
popd
exit /b 0

:error
echo.
echo Benchmark pipeline failed. Check the error above.
popd
exit /b 1
