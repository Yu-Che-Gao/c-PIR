#!/usr/bin/python
#!-*- coding: utf-8 -*-

import time
import sys
import RPi.GPIO as GPIO

# 使用 BCM GPIO 取代實際接腳號碼
GPIO.setmode(GPIO.BCM)
# 定義樹莓派連接到 PIR 的接腳
PIN_PIR = 18
STB_TIME = 3

try:
    
    GPIO.setup(PIN_PIR, GPIO.IN)
    print "\n",
    for i in range(0, STB_TIME):
        print "      Waitting for PIR to stable, %d sec\r" %(STB_TIME - i),
        sys.stdout.flush()
        time.sleep(1)
    
    print "\n"
    print "  READY ...",
    print "\n"
    
    # 按下 Ctrl + C 離開
    while True:
        if(GPIO.input(PIN_PIR)):
            print "THE PIR SENSOR DETECTED MOVEMENT           \r",
            sys.stdout.flush()
            time.sleep(3)   # 此時間的設定必須要大於 PIR 的延遲時間
        else:
            print "PIR sensor get ready for movement detection\r",            
    
except KeyboardInterrupt:    
    print "\n\n  Quit!  \n"