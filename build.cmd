SET VCPKG_DIR=C:\vcpkg
SET PROJECT_DIR=C:\projects\CompositeScannerArm\

%VCPKG_DIR%\downloads\tools\cmake-3.25.1-windows\cmake-3.25.1-windows-i386\bin\cmake ^
-Wno-dev ^
-G "Visual Studio 17 2022" -A x64 ^
-DCMAKE_BUILD_TYPE=RELWITHDEBINFO ^
-DCMAKE_TOOLCHAIN_FILE=%VCPKG_DIR%/scripts/buildsystems/vcpkg.cmake ^
-DIce_SLICE_DIR=%VCPKG_DIR%/installed/x64-windows/share/ice/slice ^
-DIce_LIBRARY=%VCPKG_DIR%/installed/x64-windows/lib ^
-DIce_HOME=%VCPKG_DIR%/installed/x64-windows ^
-DPython3_EXECUTABLE=%VCPKG_DIR%/installed/x64-windows/tools/python3/python.exe ^
-DCMAKE_PREFIX_PATH=%VCPKG_DIR%/installed/x64-windows/share/fftw3/fftw3;%VCPKG_DIR%installed/x64-windows/share/eigen3;%VCPKG_DIR%/installed/x64-windows/share/pugixml;%VCPKG_DIR%/installed/x64-windows/share/opencv; ^
-DDEVTALK_BUILD_CORE=ON ^
-DUCL_BUILD_PLOTVIEW_PLUGINS=ON ^
-DDEVTALK_BUILD_CORE_SERVER=ON ^
-DDEVTALK_BUILD_DRIVER_NetworkDevice=ON ^
-DDEVTALK_BUILD_DRIVER_Unitest=ON ^
-DDEVTALK_BUILD_DRIVER_APLSystem=ON ^
-DDEVTALK_BUILD_DRIVER_APLCoil=ON ^
-DDEVTALK_BUILD_SERVANT_APLSystem=ON ^
-DDEVTALK_BUILD_SERVANT_AudioDataCollector=ON ^
-DDEVTALK_BUILD_SERVANT_APLCoil=ON ^
-DCMAKE_CXX_STANDARD=17 ^
-DCMAKE_CXX_STANDARD_REQUIRED=ON ^
-DCMAKE_CXX_EXTENSIONS=OFF ^
%PROJECT_DIR%
pause
COPY %PROJECT_DIR%\.editorconfig .
