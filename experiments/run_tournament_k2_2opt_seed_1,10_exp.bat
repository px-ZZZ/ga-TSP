@echo off

cd /d "%~dp0.."

for /L %%i in (1,1,10) do (
    "x64\Debug\my_TSP_project.exe" --sel tournament --mut 2opt --seed %%i --save experiment
)
pause