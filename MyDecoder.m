% Automatic Picture Transmission (APT) Decoding
% Developed by Bill Dower, Kawaya Swana Eric Mbaka, Mark Roland, Vijo Varghese
% at the University of Kansas Electrical Engineering Department, May 2006

%%WXSat demodulation
clear all;
close all;
location='N18_4827.wav';
[x1, fs]=audioread(location);
x=x1(1:length(x1),1);

search =7;


max_amplitude = max([max(x), abs(min(x))]); % find the maximum amplitude
x = x./max_amplitude; % normalize the input signal
x=x-mean(x);
%%Low pass filter used in demodulation of the AM signal
[B,A] = butter(20,1000/(fs/2),'low');
ensig=abs(x);  %%Rectifies the AM signal                   
out=filter(B,A,ensig);  %%Preforms the smothing function in Envelope detection
%out=ensig;  %%Preforms the smoothing function in Envelope detection

out=out-mean(out);%% Remove any DC                 
y=out/max(out);%%Normalizes the message signal
t=[0:1/fs:1/160];%%Length of the sync wave form in time at the 
%%sampling freq
cA=(square(1040*t*(2*pi)));%%Creates the expected sync pulse
hA=conv(transpose(cA),y(1:length(y)));%%Preforms the correlation
syncA=hA(length(cA):length(hA));%%Removes the convolution tails

[H,I]=max(syncA);

sampsbefore = floor(I/(fs/2))-5;
sampsafter  = floor((length(syncA)-I)/(fs/2));


 %plot(syncA)
% axis([3.095e6 3.14e6 14 19.5])

synclocs = ones(sampsbefore+sampsafter,1);
currIndex = round(I-(fs/2));
for i = 1:sampsbefore
    
    
    [locH,locI] = max(syncA((currIndex - fs/search):(currIndex + fs/search)));
    
    globI = (currIndex - fs/search) + locI -1;
   
    
    synclocs(i) = globI;
    
    currIndex = round(globI-(fs/2));
    % hold on
    % plot(synclocs,syncA(round(synclocs)),'r*')
  
end


synclocs = [I;synclocs];
synclocs(1:sampsbefore+1) = flipud(synclocs(1:sampsbefore+1));



currIndex = round(I+(fs/2));

for i = 1:sampsafter-2
    
    
    [locH,locI] = max(syncA((currIndex - fs/search):(currIndex + fs/search)));
    
    globI = (currIndex - fs/search) + locI -1;
   
    
    synclocs(i+sampsbefore+1) = globI;
    
    currIndex = round(globI+(fs/2));
  
 
end
%     hold on
%     plot(synclocs,syncA(round(synclocs)),'r*')
    
    
    
for j = 1:length(synclocs)
    
    if j==1
        
        imagemat = y(synclocs(j):synclocs(j)+round(fs/2))';
    
    end
    
    imagemat = [imagemat;y(synclocs(j):synclocs(j)+round(fs/2))'];

    
    
    
    
end

    imagemat=flipud(imagemat);
    imagemat=fliplr(imagemat);
    
    imagesc(imagemat);
    colormap(gray);
[rows columns numberOfColorChannels] = size(imagemat);
stretchedImage = imresize(imagemat, [rows 2080],'lanczos3');
stretchedImage=stretchedImage-min(stretchedImage(:));
stretchedImage=stretchedImage/max(stretchedImage(:));
imwrite(stretchedImage,'myimage8.png', 'BitDepth', 16);
