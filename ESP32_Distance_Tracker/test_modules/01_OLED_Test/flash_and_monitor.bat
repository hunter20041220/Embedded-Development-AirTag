@echo off
REM ESP32 OLED测试 - 烧录和监视脚本

echo ========================================
echo ESP32 OLED测试程序 - 烧录工具
echo ========================================
echo.

REM 设置环境变量
set IDF_PATH=D:\progress\esp\esp-idf\v5.5.1\esp-idf
set PATH=D:\progress\esp\espressif\tools\xtensa-esp-elf\esp-14.2.0_20241119\xtensa-esp-elf\bin;D:\progress\esp\espressif\tools\ninja\1.12.1;%PATH%

REM Python路径
set PYTHON=D:\progress\esp\espressif\python_env\idf5.5_py3.11_env\Scripts\python.exe

REM 检查COM端口
echo [1/3] 检查ESP32连接...
%PYTHON% -m serial.tools.list_ports
echo.

REM 烧录固件
echo [2/3] 烧录固件到ESP32 (COM3)...
echo 请确保：
echo   - ESP32已通过USB连接
echo   - 没有其他程序占用COM3端口
echo   - OLED已正确接线 (VCC-3.3V, GND-GND, SCL-GPIO22, SDA-GPIO21)
echo.
pause

%PYTHON% %IDF_PATH%\tools\idf.py -p COM3 flash

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ❌ 烧录失败！
    echo 可能原因：
    echo   1. COM3端口被占用（关闭Arduino IDE、串口监视器等）
    echo   2. ESP32未连接或驱动未安装
    echo   3. USB线缆问题
    echo.
    pause
    exit /b 1
)

echo.
echo ✅ 烧录成功！
echo.

REM 启动串口监视器
echo [3/3] 启动串口监视器...
echo 按 Ctrl+] 退出监视器
echo.
timeout /t 2 /nobreak >nul

%PYTHON% %IDF_PATH%\tools\idf.py -p COM3 monitor
