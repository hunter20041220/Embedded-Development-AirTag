#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ESP32距离追踪器 - 电脑监控程序

功能：
- 实时接收ESP32发送的距离和传感器数据
- 在终端显示格式化输出
- 可选图形界面显示曲线
- 数据记录和导出

作者: GitHub Copilot
日期: 2025-12-14
"""

import serial
import serial.tools.list_ports
import json
import time
import argparse
from datetime import datetime
from colorama import init, Fore, Back, Style

# 初始化colorama（Windows彩色输出）
init(autoreset=True)

class ESP32Monitor:
    """ESP32监控器类"""
    
    def __init__(self, port=None, baudrate=115200, timeout=1):
        """
        初始化监控器
        
        Args:
            port: 串口号（如'COM3'）
            baudrate: 波特率
            timeout: 超时时间（秒）
        """
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.ser = None
        self.running = False
        self.data_buffer = []
        
    def list_ports(self):
        """列出可用串口"""
        ports = serial.tools.list_ports.comports()
        available_ports = []
        
        print(f"\n{Fore.CYAN}可用串口列表：{Style.RESET_ALL}")
        for port in ports:
            print(f"  {Fore.GREEN}[{port.device}]{Style.RESET_ALL} {port.description}")
            available_ports.append(port.device)
        
        return available_ports
    
    def connect(self):
        """连接串口"""
        try:
            if self.port is None:
                ports = self.list_ports()
                if not ports:
                    print(f"{Fore.RED}❌ 未找到任何串口设备！{Style.RESET_ALL}")
                    return False
                
                # 自动选择第一个CH340/CP2102设备
                for port in serial.tools.list_ports.comports():
                    if 'CH340' in port.description or 'CP2102' in port.description:
                        self.port = port.device
                        break
                
                if self.port is None:
                    self.port = ports[0]
            
            print(f"\n{Fore.YELLOW}正在连接 {self.port}...{Style.RESET_ALL}")
            self.ser = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                timeout=self.timeout
            )
            time.sleep(2)  # 等待ESP32重启
            
            print(f"{Fore.GREEN}✓ 串口连接成功！{Style.RESET_ALL}")
            print(f"  端口: {self.port}")
            print(f"  波特率: {self.baudrate}")
            self.running = True
            return True
            
        except serial.SerialException as e:
            print(f"{Fore.RED}❌ 串口连接失败: {e}{Style.RESET_ALL}")
            print(f"\n{Fore.YELLOW}请检查：{Style.RESET_ALL}")
            print("  1. ESP32是否已连接")
            print("  2. 驱动程序是否安装")
            print("  3. 其他程序是否占用串口")
            return False
    
    def disconnect(self):
        """断开连接"""
        self.running = False
        if self.ser and self.ser.is_open:
            self.ser.close()
            print(f"\n{Fore.YELLOW}串口已关闭{Style.RESET_ALL}")
    
    def read_line(self):
        """读取一行数据"""
        if not self.ser or not self.ser.is_open:
            return None
        
        try:
            if self.ser.in_waiting > 0:
                line = self.ser.readline().decode('utf-8', errors='ignore').strip()
                return line
        except Exception as e:
            print(f"{Fore.RED}读取错误: {e}{Style.RESET_ALL}")
        
        return None
    
    def parse_data(self, line):
        """解析数据行"""
        # 尝试解析JSON格式
        if line.startswith('{') and line.endswith('}'):
            try:
                data = json.loads(line)
                return data
            except json.JSONDecodeError:
                pass
        
        # 尝试解析键值对格式（如 "DISTANCE:5.2,RSSI:-68"）
        data = {}
        try:
            pairs = line.split(',')
            for pair in pairs:
                if ':' in pair:
                    key, value = pair.split(':', 1)
                    key = key.strip().lower()
                    value = value.strip()
                    
                    # 尝试转换为数字
                    try:
                        if '.' in value:
                            data[key] = float(value)
                        else:
                            data[key] = int(value)
                    except ValueError:
                        data[key] = value
        except Exception:
            return None
        
        return data if data else None
    
    def format_output(self, data):
        """格式化输出"""
        timestamp = datetime.now().strftime("%H:%M:%S")
        
        # 提取关键数据
        distance = data.get('distance', 0)
        rssi = data.get('rssi', 0)
        status = data.get('status', 'unknown')
        temp = data.get('temperature', 0)
        humidity = data.get('humidity', 0)
        light = data.get('light', 0)
        
        # 根据状态选择颜色
        if status == 'very_close':
            status_color = Fore.GREEN
            status_icon = '✓'
        elif status == 'normal':
            status_color = Fore.CYAN
            status_icon = '✓'
        elif status == 'warning':
            status_color = Fore.YELLOW
            status_icon = '⚠️'
        elif status == 'danger':
            status_color = Fore.RED
            status_icon = '❌'
        else:
            status_color = Fore.WHITE
            status_icon = '?'
        
        # 输出主信息
        print(f"\n[{Fore.BLUE}{timestamp}{Style.RESET_ALL}] "
              f"{status_color}{status_icon} 距离: {distance:.1f}m "
              f"| RSSI: {rssi}dBm "
              f"| 状态: {status.upper()}{Style.RESET_ALL}")
        
        # 输出传感器数据
        print(f"           "
              f"温度: {Fore.GREEN}{temp:.1f}°C{Style.RESET_ALL} | "
              f"湿度: {Fore.CYAN}{humidity:.1f}%{Style.RESET_ALL} | "
              f"光照: {Fore.YELLOW}{light:.0f} lux{Style.RESET_ALL}")
        
        # 加速度数据（如果有）
        if 'accel_x' in data:
            print(f"           "
                  f"加速度: X={data['accel_x']:.2f}g "
                  f"Y={data['accel_y']:.2f}g "
                  f"Z={data['accel_z']:.2f}g")
        
        # 其他传感器
        extras = []
        if 'ultrasonic' in data:
            extras.append(f"超声波: {data['ultrasonic']:.2f}m")
        if 'pir' in data:
            pir_status = "检测到" if data['pir'] else "无"
            extras.append(f"人体: {pir_status}")
        if 'rfid_uid' in data and data['rfid_uid']:
            extras.append(f"RFID: {data['rfid_uid']}")
        
        if extras:
            print(f"           {' | '.join(extras)}")
    
    def monitor(self, log_file=None):
        """开始监控"""
        print(f"\n{Fore.GREEN}{'=' * 50}{Style.RESET_ALL}")
        print(f"{Fore.CYAN}ESP32距离追踪器 - 实时监控{Style.RESET_ALL}")
        print(f"{Fore.GREEN}{'=' * 50}{Style.RESET_ALL}")
        print(f"串口: {self.port} | 波特率: {self.baudrate}")
        print(f"开始时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        print(f"{Fore.GREEN}{'=' * 50}{Style.RESET_ALL}")
        print(f"\n{Fore.YELLOW}按 Ctrl+C 停止监控{Style.RESET_ALL}\n")
        
        # 打开日志文件
        log_fp = None
        if log_file:
            log_fp = open(log_file, 'a', encoding='utf-8')
            log_fp.write(f"\n{'=' * 60}\n")
            log_fp.write(f"监控开始时间: {datetime.now()}\n")
            log_fp.write(f"{'=' * 60}\n")
        
        try:
            while self.running:
                line = self.read_line()
                if line:
                    # 直接显示原始数据（用于调试）
                    if line.startswith('I (') or line.startswith('E ('):
                        # ESP-IDF日志格式
                        print(f"{Fore.LIGHTBLACK_EX}{line}{Style.RESET_ALL}")
                        continue
                    
                    # 解析数据
                    data = self.parse_data(line)
                    if data:
                        self.format_output(data)
                        self.data_buffer.append(data)
                        
                        # 写入日志
                        if log_fp:
                            log_fp.write(f"{datetime.now()} | {json.dumps(data)}\n")
                            log_fp.flush()
                
                time.sleep(0.1)
        
        except KeyboardInterrupt:
            print(f"\n\n{Fore.YELLOW}收到中断信号，正在停止...{Style.RESET_ALL}")
        
        finally:
            if log_fp:
                log_fp.write(f"\n监控结束时间: {datetime.now()}\n")
                log_fp.close()
                print(f"\n{Fore.GREEN}✓ 日志已保存到: {log_file}{Style.RESET_ALL}")

def main():
    """主函数"""
    parser = argparse.ArgumentParser(description='ESP32距离追踪器监控程序')
    parser.add_argument('-p', '--port', help='串口号（如COM3）')
    parser.add_argument('-b', '--baud', type=int, default=115200, help='波特率（默认115200）')
    parser.add_argument('-l', '--log', help='日志文件路径')
    parser.add_argument('--list', action='store_true', help='列出可用串口')
    
    args = parser.parse_args()
    
    # 创建监控器
    monitor = ESP32Monitor(port=args.port, baudrate=args.baud)
    
    # 列出串口
    if args.list:
        monitor.list_ports()
        return
    
    # 连接并开始监控
    if monitor.connect():
        try:
            monitor.monitor(log_file=args.log)
        finally:
            monitor.disconnect()

if __name__ == '__main__':
    main()
