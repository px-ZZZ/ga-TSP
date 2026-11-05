@echo off
for /L %%i in (1,1,10) do (
    "x64\Debug\my_TSP_project.exe" --sel tournament --mut swap --seed %%i --save experiment
)
pause