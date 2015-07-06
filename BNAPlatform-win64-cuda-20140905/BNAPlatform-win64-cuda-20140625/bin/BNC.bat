@echo *******************************************************************************
@echo *           Welcome to use ParBNA by NICS, EE, Tsinghua University            *
@echo *                            Brain Network Construction                       *
@echo *          This function constructs brain networks from BOLD signals          *
@echo *******************************************************************************

@echo -Please enter a directory containing your .nii files for BOLD signals.
@echo -The directory should contain a "mask.nii".
@echo -All the .nii file(s) in the directory will be processed.
@set /p out=^>^><nul
@set /p directory=
@echo.

@echo -Please input a threshold for "mask.nii" to select the valid voxel [0~1].
@echo -e.g. 0.5
@set /p out=^>^><nul
@set /p threshold_mask= 
@echo.

@echo -Please input threshold(s) for correlation coefficients to binarize the network(s) [0~1].
@echo -e.g. 0.5 0.6 0.7
@set /p out=^>^><nul
@set /p threshold_corr= 
@echo.

@echo -Are the correlation metrices to be averaged [Y, N, B]? 
@echo -Y: The output is a network averaged from all the corelation matrices.
@echo -N: The output are individual networks.
@echo -B: The output are both of above.
@set /p out=^>^><nul
@set /p average= 
@echo.

@echo .\CUCorMat.exe %directory% %threshold_mask% %average% %threshold_corr%>BNC_execute.bat
@echo @pause>>BNC_execute.bat
@.\BNC_execute.bat
@pause