# -*- coding: utf-8 -*-
import itertools, sys, time
import subprocess
import signal
import os
import ephem
import TLE 
from tabulate import tabulate
import copy
import pickle
import thread
import socket
import datetime
import base64
import requests


sats = ['NOAA 15','NOAA 18','NOAA 19'] #Names must be complete


def updateTLE(sats):
	""" Creates list of pyephem tracked objects from list of name sats provided using TLEreaderW() """

	trackedObjs = []

	tles = TLE.TLEreaderW(sats) # pull from network

	for x in xrange(0,len(sats)):

		trackedObjs.append(ephem.readtle(tles[x][0],tles[x][1],tles[x][2]))

	return trackedObjs



def nextTenEvents(trackedObjs,Location):

	""" Returns a lists of lists, first list is the next ten events for use by the rest of the program  TODO: Searching for passes should not be arbitrary number"""

	atl = ephem.city(Location) # Antenna Location
	currentTime = ephem.now() # UTC time object
	atl.date = currentTime 
	minimumElevation = 50 #50 Degrees minimum elevation for pass
	satPass = []
	tabsat = []

	for i in xrange(0,len(trackedObjs)):  # Run through all currently tracked objects

		atl.date = currentTime 

		for j in xrange(0,100):  # Check individual object's next 60 passes(arbitrary) TODO fix to be smarter

			if float(atl.next_pass(trackedObjs[i])[3])*180/3.14159 > minimumElevation: 

				#Create individuals pass list [name,start of pass time object, end of pass time object, elevation deg, tracked object reference]
				satPass.append([trackedObjs[i].name, atl.next_pass(trackedObjs[i])[0], atl.next_pass(trackedObjs[i])[4], float(atl.next_pass(trackedObjs[i])[3])*180/3.14159, 'TRACK' ,trackedObjs[i]])
				#Create pass list for printing to terminal with tabulate [float start time, name, string start time, str end time, elevation deg]
				#tabsat.append([float(atl.next_pass(trackedObjs[i])[0]), trackedObjs[i].name,str(atl.next_pass(trackedObjs[i])[0]),str(atl.next_pass(trackedObjs[i])[4]),float(atl.next_pass(trackedObjs[i])[3])*180/3.14159])
			atl.date = atl.next_pass(trackedObjs[i])[4] + .00000001

	satPass = sorted(satPass, key=lambda pa: pa[1]) # Sort passes by start pass time
	satPass = satPass[0:10] # save only the top ten passes
	#tabsat = sorted(tabsat, key=lambda pa: pa[0])# Sort passes by start pass time

	# for x in tabsat: #delete the float start pass time in tabsat, was only needed for sorting purpose, not really human readable
	# 	del x[0]

	#tabsat = tabsat[0:30]  # save only the top ten passes


	#atl.date = currentTime

	return satPass


def checkOverlap(satPass):

	for i in xrange(0,len(satPass)):

		#skip = 0

		for j in xrange(0,i):

			if j != i:

				if satPass[j][1] <= satPass[i][1] <= satPass[j][2]:

					if satPass[j][3] > satPass[i][3] and satPass[j][4] != 'SKIP':

						satPass[i][4] = 'SKIP'
					else:
						
						satPass[j][4] ='SKIP'
					

		#satPass[i].append(str('1') + ' Overlaps')

	return satPass

def clientTransfer(fileName):
	fileNombre = fileName

	s = socket.socket()
	s.connect(("localhost",9999))
	fileName.rjust(len(fileName) + 12, '+')
	fileName = fileName[len(fileName)-13:len(fileName)]
	s.send(fileName)
	f=open (fileNombre+'.wav', "rb") 
	l = f.read(1024)
	while (l):
	    s.send(l)
	    l = f.read(1024)
	s.close


# def clientTransferText(stringthing):

# 	s = socket.socket()
# 	s.connect(("localhost",9999))
# 	s.send(stringthing)
# 	s.close()

def transferImages():
	count =0
	
	manifest = pickle.load(open("man.dat","rb"))

	for x in manifest:
		if x[1] ==0:
			count = count +1
		if x[1] ==1:
			del x
	print count, ' Files to send'

	for x in manifest:
		if x[1] ==0:
			clientTransfer(str(x[0]))
			x[1] =1
			del x

	pickle.dump(manifest,open("man.dat","wb")) 
	


def signal_handling(signum,frame):
    try:
    	os.killpg(os.getpgid(pro.pid), signal.SIGTERM)
    except:
    	pass
    sys.exit()



signal.signal(signal.SIGINT,signal_handling)

#time.strftime("%Y/%m/%d %H:%M:%S", time.gmtime())


trackedObjs = updateTLE(sats) #update from network
events = nextTenEvents(trackedObjs,'Atlanta') # get list of next ten passes


satPass = checkOverlap(events)

satPass2 = copy.deepcopy(satPass)# make a deep copy that will be formated for terminal display

for x in satPass2: # Delete the tracked object in the copy, no need to print that to terminal
	del x[5]

for x in satPass2: # convert time to strings for printing
	x[1] = str(ephem.localtime(x[1])).split(".")[0]
	x[2] = str(ephem.localtime(x[2])).split(".")[0]
	x[3] = str(round(x[3],2)) + u'°'

satPass2.insert(0,['Name + Status','Start Time','End Time','Elevation','Overlap?']) # Title of printed table
print tabulate(satPass2)


nextEvent = satPass[0]

#rtl_fm -g 20 -f 91.1M -M fm -s 200k -r 11025 -E deemp | sox -t raw -e signed -c 1 -b 16 -r 11025 - subrecord.wav

atl = ephem.city('Atlanta')
spinner = itertools.cycle(['-', '/', '|', '\\'])

print 'Waiting for Pass'

y =1
while True:

	

	if float(nextEvent[2]) > float(ephem.now()) > float(nextEvent[1]) or y:

		timeStamp = datetime.datetime.now().strftime('%Y-%m-%dT%Hz%Mz%S')

		pro = subprocess.Popen('rtl_fm -g 20 -f 91.1M -M fm -s 200k -r 11025 -E deemp | sox -t raw -e signed -c 1 -b 16 -r 11025 - '+ str(timeStamp)+'.wav', shell=True, preexec_fn=os.setsid)
		time.sleep(5)

		if pro.poll() != None:
			sys.exit()

		print 'Recording Pass\n'

		while float(nextEvent[2]) > float(ephem.now()) > float(nextEvent[1]) or y:

			atl.date = ephem.now()
			nextEvent[5].compute(atl)
			sys.stdout.write('\r' + 'Current Alt' + str(nextEvent[5].alt) + ' ' * 20)
			sys.stdout.flush() 
			#time.sleep(0.1)
			time.sleep(2) #TEMP
			y = 0



		os.killpg(os.getpgid(pro.pid), signal.SIGTERM)



		f=open(timeStamp+".wav", "rb") 

		data = f.read()
		b64data = base64.b64encode(data)

		postdata = {'wav':b64data,'date':timeStamp}

		res = requests.post('http://127.0.0.1:8080',postdata)

		f.close

		time.sleep(.1)

		print 'Pass Complete'

		trackedObjs = updateTLE(sats) #update from network
		events = nextTenEvents(trackedObjs,'Atlanta') # get list of next ten passes
		satPass = checkOverlap(events)
		nextEvent = satPass[0]





	sys.stdout.write(spinner.next())  # write the next character
	sys.stdout.flush()                # flush stdout buffer (actual character display)
	time.sleep(0.1)
	sys.stdout.write('\b')            # erase the last written char
	atl.date = ephem.now()
	nextEvent[5].compute(atl)

