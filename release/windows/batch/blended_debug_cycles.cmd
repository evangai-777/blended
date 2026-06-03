@echo off
echo Starting Blended with Cycles logging options, log files will be created
echo in your temp folder, windows explorer will open after you close Blended
echo to help you find them.
echo.
echo If you report a bug on https://github.com/EvangAI-777/Blended/issues you can attach these files
echo by dragging them into the text area of your bug report, please include both
echo blended_debug_output.txt and blended_system_info.txt in your report.
echo.
pause
echo.
echo Starting Blended and waiting for it to exit....
setlocal

set PYTHONPATH=
set DEBUGLOGS="%temp%\blended\debug_logs"
mkdir "%DEBUGLOGS%" > NUL 2>&1

"%~dp0\Blended" --debug --log cycles --log-level 4 --python-expr "import bpy; bpy.context.preferences.filepaths.temporary_directory=r'%DEBUGLOGS%'; bpy.ops.wm.sysinfo(filepath=r'%DEBUGLOGS%\blended_system_info.txt')" > "%DEBUGLOGS%\blended_debug_output.txt" 2>&1 < %0
explorer "%DEBUGLOGS%"
