SET VCPKG_DIR=C:\Projects\vcpkg
SET PROJECT_DIR=C:\Projects\CompositeScannerArm\

cmake ^
-Wno-dev ^
-G "Visual Studio 16 2019" -A x64 ^
-DCMAKE_BUILD_TYPE=RELWITHDEBINFO ^
-DCMAKE_TOOLCHAIN_FILE=%VCPKG_DIR%/scripts/buildsystems/vcpkg.cmake ^
-DIce_HOME=%VCPKG_DIR%/installed/x64-windows ^
-DCMAKE_PREFIX_PATH=%VCPKG_DIR%/installed/x64-windows/share/fftw3/fftw3;%VCPKG_DIR%installed/x64-windows/share/eigen3;%VCPKG_DIR%/installed/x64-windows/share/pugixml;%VCPKG_DIR%/installed/x64-windows/share/opencv ^
-DUCL_BUILD_DOC=OFF ^
-DUCL_BUILD_PYRTGEN=OFF ^
-DUCL_BUILD_TESTS=OFF ^
-DUCL_BUILD_EXAMPLES=OFF ^
-DUCL_BUILD_PLOTVIEW_PLUGINS=ON ^
-DDEVTALK_BUILD_DRIVER_MicroMech=OFF ^
-DDEVTALK_BUILD_DRIVER_Unitest=ON ^
-DDEVTALK_BUILD_SERVANT_AudioDataCollector=ON ^
-DDEVTALK_BUILD_SERVANT_EmptySignalSource=OFF ^
-DDEVTALK_BUILD_SERVANT_FTDISerialDevice=OFF ^
-DDEVTALK_BUILD_SERVANT_LightIndicator=OFF ^
-DDEVTALK_BUILD_SERVANT_MicroMechM022=OFF ^
-DDEVTALK_BUILD_SERVANT_TechnoCoordinateBoundaryTrigger=OFF ^
-DDEVTALK_BUILD_SERVANT_UnitestActuator=OFF ^
-DDEVTALK_BUILD_SERVANT_UnitestCoil=ON ^
-DDEVTALK_BUILD_SERVANT_UnitestDemagnetizer=OFF ^
-DDEVTALK_BUILD_SERVANT_UnitestEddyCurrentConverter=OFF ^
-DDEVTALK_BUILD_SERVANT_UnitestSerialStepMotor=ON ^
-DDEVTALK_BUILD_SERVANT_WindowsSerialDevice=ON ^
-DDEVTALK_CAND_BUILD_Coil=ON ^
-DDEVTALK_CAND_BUILD_Demagnetizer=OFF ^
-DDEVTALK_CAND_BUILD_DisplacementSensorM022=OFF ^
-DDEVTALK_CAND_BUILD_EddyCurrentConverter=OFF ^
-DDEVTALK_CAND_BUILD_Riftec=OFF ^
%PROJECT_DIR%
pause
COPY %PROJECT_DIR%\.editorconfig .
