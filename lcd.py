#!/usr/bin/python

# Copied mostly from Adafruit's RPi CharLCD code:
# https://github.com/adafruit/Adafruit-Raspberry-Pi-Python-Code/tree/master/Adafruit_CharLCD

import RPi.GPIO as GPIO
from time import sleep
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

class LCD:
        # commands
        LCD_CLEARDISPLAY               = 0x01
        LCD_RETURNHOME                 = 0x02
        LCD_ENTRYMODESET               = 0x04
        LCD_DISPLAYCONTROL             = 0x08
        LCD_CURSORSHIFT                = 0x10
        LCD_FUNCTIONSET                = 0x20
        LCD_SETCGRAMADDR               = 0x40
        LCD_SETDDRAMADDR               = 0x80

        # flags for display entry mode
        LCD_ENTRYRIGHT                 = 0x00
        LCD_ENTRYLEFT                  = 0x02
        LCD_ENTRYSHIFTINCREMENT        = 0x01
        LCD_ENTRYSHIFTDECREMENT        = 0x00

        # flags for display on/off control
        LCD_DISPLAYON                  = 0x04
        LCD_DISPLAYOFF                 = 0x00
        LCD_CURSORON                   = 0x02
        LCD_CURSOROFF                  = 0x00
        LCD_BLINKON                    = 0x01
        LCD_BLINKOFF                   = 0x00

        # flags for display/cursor shift
        LCD_DISPLAYMOVE                = 0x08
        LCD_CURSORMOVE                 = 0x00

        # flags for display/cursor shift
        LCD_DISPLAYMOVE                = 0x08
        LCD_CURSORMOVE                 = 0x00
        LCD_MOVERIGHT                  = 0x04
        LCD_MOVELEFT                   = 0x00

        # flags for function set
        LCD_8BITMODE                   = 0x10
        LCD_4BITMODE                   = 0x00
        LCD_2LINE                      = 0x08
        LCD_1LINE                      = 0x00
        LCD_5x10DOTS                   = 0x04
        LCD_5x8DOTS                    = 0x00

        def __init__(self, RS=2, EN=3, DB=[22,27,17,4], RGB=[9,10,11]):
                self.RS = RS
                self.EN = EN
                self.DB = DB
                self.RGB = []

                GPIO.setmode(GPIO.BCM)
                GPIO.setwarnings(False) #disable warnings
                GPIO.setup(self.RS, GPIO.OUT)
                GPIO.setup(self.EN, GPIO.OUT)
                for pin in self.DB:
                        GPIO.setup(pin, GPIO.OUT)
                for x in xrange(0, len(RGB)):
                        GPIO.setup(RGB[x], GPIO.OUT)
                        self.RGB.append(GPIO.PWM(RGB[x], 50))
                        self.RGB[x].start(100) #start at 100%

                self.write4bits(0x33) # initialization
                self.write4bits(0x32) # initialization
                self.write4bits(0x28) # 2 line 5x7 matrix
                self.write4bits(0x0C) # turn cursor off 0x0E to enable cursor
                self.write4bits(0x06) # shift cursor right

                """ Initialize to default text direction (for romance languages) """
                self.displaymode =  self.LCD_ENTRYLEFT | self.LCD_ENTRYSHIFTDECREMENT
                self.write4bits(self.LCD_ENTRYMODESET | self.displaymode) #  set the entry mode

                self.clear()
        def home(self):
                self.write4bits(self.LCD_RETURNHOME) # set cursor position to zero
                self.delayMicroseconds(3000) # this command takes a long time!
        def clear(self):
                self.write4bits(self.LCD_CLEARDISPLAY) # command to clear display
                self.delayMicroseconds(3000) # 3000 microsecond sleep, clearing the display takes a long time
        def delayMicroseconds(self, microseconds):
                seconds = microseconds / float(1000000) # divide microseconds by 1 million for seconds
                sleep(seconds)
        def write4bits(self, bits, char_mode=False):
                """ Send Command to LCD """

                self.delayMicroseconds(1000)
                bits = bin(bits)[2:].zfill(8)

                GPIO.output(self.RS, char_mode)
                #first set of bits
                for pin in self.DB:
                        GPIO.output(pin, False)
                for i in range(4):
                        if bits[i] == "1":
                                GPIO.output(self.DB[::-1][i], True)
                self.pulseEnable()
                #second set
                for pin in self.DB:
                        GPIO.output(pin, False)
                for i in range(4,8):
                        if bits[i] == "1":
                                GPIO.output(self.DB[::-1][i-4], True)
                self.pulseEnable()
        def pulseEnable(self):
                GPIO.output(self.EN, False)
                self.delayMicroseconds(1)                # 1 microsecond pause - enable pulse must be > 450ns
                GPIO.output(self.EN, True)
                self.delayMicroseconds(1)                # 1 microsecond pause - enable pulse must be > 450ns
                GPIO.output(self.EN, False)
                self.delayMicroseconds(1)                # commands need > 37us to settle
        def message(self, text):
                """ Send string to LCD. Newline wraps to second line"""

                for char in text:
                        if char == '\n':
                                self.write4bits(0xC0) # next line
                        else:
                                self.write4bits(ord(char),True)
        def color(self, r, g, b):
                pass
                self.RGB[0].ChangeDutyCycle(100*(1-r)) #red
                self.RGB[1].ChangeDutyCycle(100*(1-g)) #green
                self.RGB[2].ChangeDutyCycle(100*(1-b)) #blue

class LCDFileWatcher(FileSystemEventHandler):
    def __init__(self, file):
        self.filename = file
        self.screen = LCD()
        self.screen.clear()
    def on_modified(self, event):
        if(event.src_path.find(self.filename) >= 0):
            with open(event.src_path, 'r') as myfile:
                lines = myfile.readlines()
                if len(lines) < 3:
                    return
                #first is color
                color_str = lines[0].replace('#', '')
                r = float(int(color_str[0:2], 16)) / 255
                g = float(int(color_str[2:4], 16)) / 255
                b = float(int(color_str[4:6], 16)) / 255
                #next is each message
                line1 = lines[1].replace('\n', '')
                line2 = lines[2].replace('\n', '')
                #send to screen
                self.screen.clear()
                self.screen.message(line1+"\n"+line2)
                self.screen.color(r, g, b)

if __name__ == "__main__":
    event_handler = LCDFileWatcher('lcd.txt')
    observer = Observer()
    observer.schedule(event_handler, path='.', recursive=False)
    observer.start()

    try:
        while True:
            sleep(1)
    except KeyboardInterrupt:
        observer.stop()
    observer.join()
    GPIO.cleanup()