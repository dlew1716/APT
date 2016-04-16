import matplotlib.pyplot as plt
import numpy as np
import scipy
import time
import wave
import struct
from pylab import *
import scipy.signal as signal
import cv2
import scipy.misc
from scipy.interpolate import RectBivariateSpline

#pleb

def wavLoad (fname):
    wav = wave.open (fname, "r")
    (nchannels, sampwidth, framerate, nframes, comptype, compname) = wav.getparams ()
    frames = wav.readframes (nframes * nchannels)
    out = struct.unpack_from ("%dh" % nframes * nchannels, frames)
    if nchannels == 2:
        sigarr = out[0::2]
    else:
        sigarr = out
    return [sigarr,framerate]

    



location = 'HerroOut2.wav'
WaveFile = wavLoad(location)
sigarr = wavLoad(location)[0]
sampRate = double(wavLoad(location)[1])
sigarr = np.array(sigarr)

# WavSig = wave.open(location, 'r')
# #[x1, fs] = audioread(location)
# # x = x1[0:length(x1),0]
# numOfChannels = WavSig.getnchannels()
# bitwidth = WavSig.getsampwidth()
# sampRate = WavSig.getframerate()
# NumberOfFrames = WavSig.getnframes()
# frame  = WavSig.readframes(1)


# print "Number of Channels:", numOfChannels
# print "Bit Width:", bitwidth
# print "Sample Rate:", sampRate
# print "Number Of Frames", NumberOfFrames




search = 7.
cutoff_hz = double(1000.0)

# print type(cutoff_hz)
# print type(sampRate)

max_amplitude = np.amax(np.absolute(sigarr))



#% find the maximum amplitude
sigarr = np.true_divide(sigarr,max_amplitude)

#% normalize the input signal
sigarr = np.subtract(sigarr,np.mean(sigarr))


#Design Filter
fir_coeff = signal.firwin(13, cutoff_hz/(sampRate/2))

#rectify
sigarr = np.absolute(sigarr)


#Low pass filter
sigarr = scipy.signal.lfilter(fir_coeff,1, sigarr)

sigarr = np.subtract(sigarr,np.mean(sigarr))

sigarr = np.true_divide(sigarr,np.max(sigarr))

y = sigarr


                
#%%Normalizes the message signal
t = np.array(np.hstack((np.arange(0., (1./160.)+(1./sampRate), 1./sampRate))))
#%%Length of the sync wave form in time at the 
#%%sampling freq
cA = signal.square(1040*t*(2*pi),0.5)
#%%Creates the expected sync pulse

hA = np.convolve(sigarr,cA)

#%%Preforms the correlation
syncA = hA


#%%Removes the convolution tails

H = np.amax(syncA)
I = np.argmax(syncA)

sampsbefore = np.floor(np.true_divide(I, sampRate/2.))-5.
sampsafter = np.floor(np.true_divide(np.size(syncA)-I, sampRate/2.))

synclocs = np.ones((sampsbefore+sampsafter))
currIndex = np.round((I-sampRate/2.))


for i in np.arange(1., (sampsbefore)+1):
    locH = np.amax(syncA[int(currIndex-sampRate/search)-1:int(currIndex+sampRate/search)])
    locI = np.argmax(syncA[int(currIndex-sampRate/search)-1:int(currIndex+sampRate/search)])
    globI = currIndex-sampRate/search+locI-1.
    synclocs[int(i)-1] = globI
    currIndex = np.round((globI-sampRate/2.))
    #% hold on
    #% plot(synclocs,syncA(round(synclocs)),'r*')


synclocs = np.insert(synclocs,0,I)

synclocs[0:sampsbefore+1.] = np.flipud(synclocs[0:sampsbefore+1.])
currIndex = np.round((I+sampRate/2.))

for i in np.arange(1., (sampsafter-2.)+1):
    locH = np.amax(syncA[int(currIndex-sampRate/search)-1:int(currIndex+sampRate/search)])
    locI =  np.argmax(syncA[int(currIndex-sampRate/search)-1:int(currIndex+sampRate/search)])
    globI = currIndex-sampRate/search+locI-1.
    synclocs[int((i+sampsbefore+1.))-1] = globI
    currIndex = np.round((globI+sampRate/2.))






#%plot(syncA)
#% axis([3.095e6 3.14e6 14 19.5])


    

    
#%     hold on
#%     plot(synclocs,syncA(round(synclocs)),'r*')
for j in np.arange(1., (np.size(synclocs))+1):
    if j == 1.:
        imagemat = y[int(synclocs[int(j)-1])-1:synclocs[int(j)-1]+np.round((sampRate/2.))]
    
    newmat = y[int(synclocs[int(j)-1])-1:synclocs[int(j)-1]+np.round((sampRate/2.))]
    imagemat = np.vstack((imagemat,newmat))


imagemat = np.flipud(imagemat)
imagemat = np.fliplr(imagemat)

rows = np.arange(0,imagemat.shape[0])
cols = np.arange(0,imagemat.shape[1])


outimg=cv2.resize(imagemat,(2080,imagemat.shape[0]), interpolation = cv2.INTER_CUBIC)

outimg = np.multiply(np.add(outimg,amin(outimg)*-1),(255/amin(outimg)*-1))

scipy.misc.imsave('testpy.png',outimg)

print 'saved image'

# while True:
#     time.sleep(.2)

# imagesc(imagemat)
# colormap(plt.gray)
# [rows, columns, numberOfColorChannels] = matcompat.size(imagemat)
# stretchedImage = imresize(imagemat, np.array(np.hstack((rows, 2080.))), 'lanczos3')
# stretchedImage = stretchedImage-matcompat.max(stretchedImage.flatten(1))
# stretchedImage = matdiv(stretchedImage, matcompat.max(stretchedImage.flatten(1)))
# imwrite(stretchedImage, 'myimage8.png', 'BitDepth', 16.)