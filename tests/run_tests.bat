@echo off
for %%f in (*.java) do (
  echo testing %%f
  ..\src\minijavac\Release\minijavac.exe %%f > %%~nf.log
  if errorlevel 1 (
    echo   COMPILE ERROR
  ) else (
    out.exe > %%~nf.out
    if errorlevel 1 (
      echo   RUNTIME ERROR
    ) else (
      fc %%~nf.out %%~nf.ans > %%~nf.diff
      if errorlevel 1 (
        echo   WRONG ANSWER
      ) else (
        echo   OK
      )
    )
  )
)
pause