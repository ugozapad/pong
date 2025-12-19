echo off
echo compiling vertex shaders (VS_1_1)

for %%I in (*.vs) do fxc /Tvs_1_1 /EVS /Zi /Fo%%~nI.vso %%I

echo compiling pixel shaders (PS_1_1)

for %%I in (*.ps) do fxc /Tps_1_1 /EPS /Zi /Fo%%~nI.pso %%I
