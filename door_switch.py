import time # so we can use "sleep" to wait between actions
import RPi.GPIO as io # import the GPIO library we just installed but call it "io"
import datetime
import os

#from ISStreamer.Streamer import Streamer # import the IS Streamer we just installed but call it "Streamer"
 
## name the bucket and individual access_key
## the bucket_key will send all of our messages to the same place
## the access_key tells Initial State to send the messages to you
#logger=Streamer(bucket_name="Locker Protector",bucket_key="locker_protector",access_key="Your_Access_Key_Here")

## set GPIO mode to BCM
## this takes GPIO number instead of pin number
io.setmode(io.BCM)
 
## enter the number of whatever GPIO pin you're using
door_pin = 23
 
## use the built-in pull-up resistor
io.setup(door_pin, io.IN, pull_up_down=io.PUD_UP)  # activate input with PullUp
 
## initialize door 
door=0

## this loop will execute the if statement that is true
while True:
    time.sleep(1) # wait 1 second before the next action
    ## if the switch is open
    if (io.input(door_pin)==True and door!=0):
        #logger.log("Door","Open") # stream a message saying "Open"
        #logger.flush() # send the message immediately
	now = datetime.datetime.now()
	#print "DOOR_ACTIVATION|OPENED|"+now.strftime("%Y-%m-%d %H:%M:%S")+"|DOOR"
	os.system("echo \"DOOR_ACTIVATION|OPENED|"+now.strftime("%Y-%m-%d %H:%M:%S")+"|DOOR\" > rpiFifo")
        door=0 # set door to its initial value
        #time.sleep(1) # wait 1 second before the next action
    ## if the switch is closed and door does not equal 1
    if (io.input(door_pin)==False and door!=1):
        #logger.log("Door","Close") # stream a message saying "Close"
        #logger.flush() # send the message immediately
	now = datetime.datetime.now()
	#print "DOOR_ACTIVATION|CLOSED|"+now.strftime("%Y-%m-%d %H:%M:%S")+"|DOOR"
	os.system("echo \"DOOR_ACTIVATION|CLOSED|"+now.strftime("%Y-%m-%d %H:%M:%S")+"|DOOR\" > rpiFifo")
        door=1 # set door so that this loop won't act again until the switch has been opened

