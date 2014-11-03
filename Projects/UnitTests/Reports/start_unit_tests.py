#!/bin/python

# start application on device or PC(Mac) 
# then start server inside this script to listen unit test log output
# exemple:
# win32: 
#> cd to dava.framevork/Projects/UnitTests/Reports
#> python start_unit_tests.py
# mac os x: 
# CHANGE app_path in current script
#> cd to dava.framevork/Projects/UnitTests/Reports
#> python start_unit_tests.py
# android:
# deploy application on device first
#> python start_unit_tests.py android
# iOS:
# deploy application on device first
#> python start_unit_tests.py ios

import socket
import subprocess
import sys
import os.path
import time

current_host = socket.gethostname()
current_ip = socket.gethostbyname(current_host)

HOST = current_ip
PORT = 50007

print("host ip: " + current_ip + ":" + str(PORT))

start_on_android = False
start_on_ios = False

if len(sys.argv) > 1:
    if sys.argv[1] == "android":
        start_on_android = True
    elif sys.argv[1] == "ios":
        start_on_ios = True

ponen_obj = None

if start_on_ios:
    app_name = "com.davaconsulting.UnitTests"
    device_name = "iPadMini1FW" # TODO put your device name here
    test_run_file = "testRun.js" # file with content: var target = UIATarget.localTarget(); target.delay( 7200 );
    ponen_obj = subprocess.Popen(["instruments", "-t",
        "/Applications/Xcode.app/Contents/Applications/Instruments.app/Contents/PlugIns/AutomationInstrument.xrplugin/Contents/Resources/Automation.tracetemplate", 
        "-w", device_name, app_name, "-e",
        "UIASCRIPT", test_run_file, "-host", str(HOST), "-port", str(PORT)])
elif start_on_android:
    ponen_obj = subprocess.Popen(["adb", "shell", "am", "start", "-n", "com.dava.unittests/com.dava.unittests.UnitTests",
                      "-e", "-host", str(HOST), "-e", "-port", str(PORT)])
elif sys.platform == 'win32':
    if os.path.isfile("..\\Release\\app\\UnitTestsVS2010.exe"): # run on build server (TeamCity)
        ponen_obj = subprocess.Popen(["..\\Release\\app\\UnitTestsVS2010.exe", "-host",
                      str(HOST), "-port", str(PORT)], cwd="./..")
    else:
        ponen_obj = subprocess.Popen(["..\\Release\\UnitTestsVS2010.exe", "-host", # run on local PC
                      str(HOST), "-port", str(PORT)], cwd="./..")
elif sys.platform == "darwin":
    # TODO set correct path for you
    app_path = "/Users/user123/Library/Developer/Xcode/DerivedData/TemplateProjectMacOS-bpogfklmgukhlmbnpxhfcjhfiwfq/Build/Products/Debug/UnitTests.app"
    ponen_obj = subprocess.Popen(["open", "-a", app_path, "--args", "-host", str(HOST), "-port", str(PORT)])

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
s.listen(1)
print("start listen")
try:
    s.settimeout(5)
    conn, addr = s.accept()
except socket.error, msg:
    print(msg)
    print("can't accept client error: " + str(msg))

    current_exit_code = ponen_obj.poll()
    if current_exit_code is not None:
        print("UnitTests finished with code: " + str(current_exit_code))
    else:
        print("UnitTests still running... may be increase timeout? ")
    sys.exit(-1)

print("connected by:" + str(addr))

try:
    s.setblocking(1)
    s.settimeout(None)
    conn.setblocking(1)
except socket.error, msg:
    print("can't make socket blocking:" + msg)

while 1:
    data = None
    try:
        data = conn.recv(1024)
    except socket.error, msg:
        print("can't receive data course:" + str(msg))
        break
    if not data:
        break
    sys.stdout.write(data)
    sys.stdout.flush() #  want teamcity show output realtime for every test

conn.close()
