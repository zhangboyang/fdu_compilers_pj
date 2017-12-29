@echo off
for %%f in (*.java) do (
  echo %%f
  javac %%f
  java %%~nf > %%~nf.ans
)
pause
