import requests


def TLEreaderW(sats):
	"""Pulls NOAA TLE files from celestrak and pulls out the currently active (as of March 2016) NOAA weather 
	satellites TLE's"""

	r = requests.get('https://www.celestrak.com/NORAD/elements/noaa.txt')

	if(r.status_code != 200):
		print 'TLE network error'

	TLElist = r.text.split('\r\n')
	TLEentry = []


	found = 0

	#the two lines following the satellite are the TLE(Two Line Element) you need
	for x in range(0,len(sats)):

		found = 0

		for y in range(0,len(TLElist)):

			if TLElist[y][0:len(sats[x])] == sats[x]:
				TLEentry.append([str(TLElist[y]),str(TLElist[y+1]),str(TLElist[y+2])])
				found = 1

		if not found:
			print sats[x],' Not Found'
			TLEentry.append(['Not Found',0,0])


	# if((len(noaa15TLE)<3) or (len(noaa18TLE)<3) or (len(noaa19TLE)<3)):

	# 	print 'TLE parse error'

	return TLEentry



# def TLEreader():
# 	"""Pulls NOAA TLE files from celestrak and pulls out the currently active (as of March 2016) NOAA weather 
# 	satellites TLE's"""

# 	r = requests.get('https://www.celestrak.com/NORAD/elements/noaa.txt')

# 	if(r.status_code != 200):
# 		print 'TLE network error'

# 	noaaTLE = r.text.split('\r\n')
# 	noaa15TLE = 0;
# 	noaa18TLE = 0;
# 	noaa19TLE = 0;

# 	#the two lines following the satellite are the TLE(Two Line Element) you need
# 	for x in range(0,len(noaaTLE)):

# 		if noaaTLE[x][0:len('NOAA 15')]=='NOAA 15':
# 			noaa15TLE = [str(noaaTLE[x]),str(noaaTLE[x+1]),str(noaaTLE[x+2])]

# 		if noaaTLE[x][0:len('NOAA 18')]=='NOAA 18':
# 			noaa18TLE = [str(noaaTLE[x]),str(noaaTLE[x+1]),str(noaaTLE[x+2])]

# 		if noaaTLE[x][0:len('NOAA 19')]=='NOAA 19':
# 			noaa19TLE = [str(noaaTLE[x]),str(noaaTLE[x+1]),str(noaaTLE[x+2])]

# 	if((len(noaa15TLE)<3) or (len(noaa18TLE)<3) or (len(noaa19TLE)<3)):

# 		print 'TLE parse error'

# 	return [noaa15TLE, noaa18TLE, noaa19TLE]