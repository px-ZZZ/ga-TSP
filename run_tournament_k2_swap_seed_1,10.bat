@echo off
for /L %%i in (1,1,10) do (
    "E:\CodeTools\Project\my_TSP_project\x64\Debug\my_TSP_project.exe" --sel tournament --mut swap --seed %%i
)
pause