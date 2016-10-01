#!/usr/bin/python
#!-*- coding: utf-8 -*-

import time
import sys              # for sys.stdout.flush()
import smtplib
import RPi.GPIO as GPIO

# 使用 BCM GPIO 取代實際接腳號碼
GPIO.setmode(GPIO.BCM)
# 定義樹莓派連接到 PIR 的接腳
PIN_PIR = 18
STB_TIME = 3

# 電子郵件帳號輸入 (請使用 GMail 帳號)
# 非 GMail 帳號要修改 send_mail 的郵件伺服器
TO = 'putyour@email.here'           # 在單引號裡輸入你的電子郵件信箱
GMAIL_USER = 'putyourusername'      # 在單引號裡輸入你的使用者名稱
GMAIL_PASS = 'putyourpasswordhere'  # 在單引號裡輸入您的密碼

SUBJECT = 'Intrusion!!'
TEXT = 'Your PIR sensor detected movement'
  
def send_email():
    print "\n"
    print "Sending Email"
    smtpserver = smtplib.SMTP("smtp.gmail.com",587) # 不是使用 GMail 的要修改這裡
    smtpserver.ehlo()
    smtpserver.starttls()
    smtpserver.ehlo
    smtpserver.login(GMAIL_USER, GMAIL_PASS)
    header = 'To:' + TO + '\n' + 'From: ' + GMAIL_USER
    header = header + '\n' + 'Subject:' + SUBJECT + '\n'
    print header
    msg = header + '\n' + TEXT + ' \n\n'
    smtpserver.sendmail(GMAIL_USER, TO, msg)
    smtpserver.close()

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
            send_email()
            time.sleep(3)   # 此時間的設定必須要大於 PIR 的延遲時間
        else:
            print "PIR sensor get ready for movement detection\r",            
    
except KeyboardInterrupt:    
    print "\n\n  Quit!  \n"