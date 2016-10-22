#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <fstream>      // std::ofstream
#include "filt.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/video.hpp"
#include "pngwriter.h"
//#include "opencv2/imgcodecs/imgcodecs.hpp"


bool isRiff(FILE * fp,unsigned char * buf);
int fileSize(FILE * fp, unsigned char * buf);
bool isWave(FILE * fp,unsigned char * buf);
bool isPCM16(FILE * fp, unsigned char * buf);
bool isPCM(FILE * fp, unsigned char * buf);
int numChan(FILE * fp, unsigned char * buf);
int sampRate(FILE * fp, unsigned char * buf);
int blockAlign(FILE * fp, unsigned char * buf);
int bitPerSamp(FILE * fp, unsigned char * buf);
int subChunk2Size(FILE * fp, unsigned char * buf);
double *filterBUT(double * B, double * A, double * X, double * Zi, int input_size, int filter_order);
double *conv(double *A,  int lenA, double *B,int lenB, int *lenC,char *cmethod);
//void downsampler(double *orgimg, int orgimgW, int orgimgH, int scaleimgW, int scaleimgH);
double *outconv;
int lenC=0;
int lenhA =0;
//double butter_11025_13_order[13] = {-0.00132381, 0.0029468, 0.02127082, 0.06446164,  0.12649509, 0.18309832, 0.20610228, 0.18309832, 0.12649509, 0.06446164, 0.02127082, 0.0029468, -0.00132381};
//double Bcoe[14] = {1.0528e-08,1.3686e-07,8.2117e-07,3.011e-06,7.5274e-06,1.3549e-05,1.8066e-05,1.8066e-05,1.3549e-05,7.5274e-06,3.011e-06,8.2117e-07,1.3686e-07,1.0528e-08};
//double Acoe[14] = {1,-8.2743,32.202,-77.874,130.33,-159.19,145.82,-101.33,53.371,-21.028,6.0201,-1.1853,0.14378,-0.0081112};
//double Zi[14] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
double Bcoe[3] = {.0572,.1144,.0572};
double Acoe[3] = {1.0,-1.2189,.477};
double Zi[3] = {0,0,0};
char convmode[] = "full";

//fix where you shift in only 2 bytes into integer


int main(int argc, char *argv[] ) {


	



	printf("%lu\n", sizeof(int)    );
	printf("%lu\n", sizeof(double) );
	printf("%lu\n", sizeof(char)   );
	printf("%lu\n", sizeof(int16_t));

	//DEBUG FILES
	// std::ofstream procwav;
	// procwav.open("procwav.csv");
	// std::ofstream sqrwav;
	// sqrwav.open("sqrwav.csv");
	// std::ofstream resultwav;
	// resultwav.open("resultwav.csv");
	// std::ofstream synclocsfile;
	// synclocsfile.open("synclocsfile.csv");

	FILE * fp;
	FILE * pFile;
	
	if ( argc != 3 ){

		printf("Error, Wrong Number of Arguments\nUsage: ./Decoder input.wav output.png\n");
		return 1;

	}
	else{

		fp = fopen(argv[1],"rb");

		if (fp==NULL){

			printf("Could not open: ");
			printf("%s\n",argv[1]);
			printf("\n");
			return 1;

		}
	}

	//buffer 4 bytes at a time
	unsigned char * buf;
	buf = (unsigned char *) malloc(4);
	
	int bitSamp = 0;
	int blockSize = 0;
	int audioBufSize = 0;
	double fs = 11025;

 	//buf = (unsigned char *) realloc(buf,(int)buf[4]+(int)buf[5]+(int)buf[6]+(int)buf[7]);

	if(isRiff(fp,buf)&&isWave(fp,buf)&&isPCM16(fp,buf)&&isPCM(fp,buf)){

		printf("File Compatible\n");
		numChan(fp,buf);
		fs = sampRate(fp,buf);
		bitSamp = bitPerSamp(fp,buf);
		blockSize = blockAlign(fp,buf);
		audioBufSize = subChunk2Size(fp,buf);

		buf = (unsigned char *) realloc(buf,audioBufSize);
		fseek(fp ,44,SEEK_SET);
		int getit = fread(buf,1,audioBufSize,fp);
		printf("Bytes Read:  ");
		printf("%d\n",getit);
		//insert stereo to mono code here
		if(getit){

			printf("Loaded\n");

		}
		else{
			printf("File corrupted\n");
			fclose(fp);
			return(1);
		}
		

	}
	else{
		printf("File Incompatible");
		fclose(fp);
		return(1);
	}

	//if(bitSamp == 8){

	//}

	//else if(bitSamp == 16){

	double * abuf;
	double * obuf;
	int * cbuf;
	double * sqbuf;
	//double * sqbuf2;
	double * hA;
	abuf = (double *) malloc(audioBufSize/sizeof(unsigned char)*sizeof(double));
	cbuf = (int *) malloc(audioBufSize/sizeof(unsigned char)*sizeof(int));



	int sqbufsize = floor(39.0*fs/4160.0);  // Ts of sync wave
	double tau = 1.0/4160.0;
	double timeCount = 0;
	printf("Square Pulse Array Size:  ");
	printf("%d\n",sqbufsize);

	sqbuf = (double *) malloc(sqbufsize/sizeof(unsigned char)*sizeof(double));
	//sqbuf2 = (double *) malloc(sqbufsize/sizeof(unsigned char)*sizeof(double));


	double sT = 1/4160.0;
	double freq = (2*3.14)/(4*sT*fs);

	for(int i = 0;i<sqbufsize;i++){

		sqbuf[i] = sin(freq * i)>=0?1:-1;

		if(i<floor(4*fs*sT)){
			sqbuf[i] = 0;
		}
		if(i>floor(32*fs*sT)){

			sqbuf[i] = 0;
		}
		// sqrwav << "," << sqbuf[i];
		
	}

	// for(int i = 0;i<sqbufsize;i++){
	// 	sqbuf2[i] = sin(freq * i)+0.7>=0?1:-1;
	// 	if(i<floor(4*fs*sT)){
	// 		sqbuf2[i] = 0;
	// 	}
	// }

	//Put two bytes into an Int16
	for(int i = 0;i<audioBufSize/2;i++){

		abuf[i] = int16_t(buf[i*2] | ( (int16_t)buf[i*2+1] << 8 ) | ( 0 << 16 ) | ( 0 << 24 ));

	}

	double maxAmp = fabs(abuf[0]);
	double sumVal = 0;
	int maxAmpIndex = 0;

	for(int i =0;i<audioBufSize/2;i++){

		if(maxAmp<fabs(abuf[i])){

			maxAmp = fabs(abuf[i]);

		}
	}
	
	printf("Max Amplitude:  ");
	printf("%f\n",maxAmp );

	//Normalize
	for(int i =0;i<audioBufSize/2;i++){

		abuf[i] = abuf[i]/maxAmp;

	}
	//find mean
	for(int i =0;i<audioBufSize/2;i++){

		sumVal = sumVal + abuf[i];

	}

	double avgVal = sumVal/audioBufSize/2;

	printf("Average Normalized Value:  ");
	printf("%f\n",avgVal );


	//Center
	for(int i =0;i<audioBufSize/2;i++){

		abuf[i] = abuf[i]-avgVal;

	}
	//Rectify
	for(int i =0;i<audioBufSize/2;i++){

		if(abuf[i]<0){

			abuf[i] = -1*abuf[i];
		}
		
	}

	Filter *my_filter;
	double filtered_sample;
	double next_sample;

	my_filter = new Filter(LPF, 51, fs, 1000.0);
  
  	for(int i =0;i<audioBufSize/2;i++){

 		next_sample = abuf[i];
  		filtered_sample = my_filter->do_sample( next_sample );
  		abuf[i] = filtered_sample;

	}
	delete my_filter;
	outconv = abuf;

	printf("Convolved with filter\n");

	sumVal =0.0;
	maxAmp =0.0;

	for(int i =0;i<audioBufSize/2;i++){

		sumVal = sumVal + outconv[i];

	}

	avgVal = sumVal/(audioBufSize/2);
	printf("Average Normalized Value After Conv:  ");
	printf("%f\n",avgVal );

	for(int i =0;i<audioBufSize/2;i++){

		outconv[i] = (outconv[i]-avgVal);


	}
	for(int i =0;i<audioBufSize/2;i++){
		
		if(maxAmp<fabs(outconv[i])){

			maxAmp = fabs(outconv[i]);

		}

	}
	printf("Max Amplitude After Conv:  ");
	printf("%f\n",maxAmp );

	for(int i =0;i<audioBufSize/2;i++){

		outconv[i] = (outconv[i]/maxAmp);
		// procwav << "," << outconv[i];
 		
	}

	hA = conv(outconv,audioBufSize/2,sqbuf,sqbufsize,&lenhA,convmode);

	printf("Square Pulse Convolution Done\n");

	maxAmp =0.0;

	for(int i = 0;i<lenhA;i++){

		if(maxAmp < hA[i]){

			maxAmp = hA[i];
			maxAmpIndex = i;
		}

	}

	printf("Max Amplitude of Coorelated Signal:  ");
	printf("%f\n",maxAmp);
	printf("Index of Max Amplitude:  ");
	printf("%d\n",maxAmpIndex);

	int sampsbefore = floor(maxAmpIndex/(fs/2))-5;
	int sampsafter = floor((lenhA-maxAmpIndex)/(fs/2))-5;
	// sampsafter = 25;
	// sampsbefore = 25;
	double Q =0;
	int j ;
	double locMax;
	int locMaxIndex;
	Q = floor(maxAmpIndex - (fs/2));
	double search =5; //move to user configurable, change with samp rate?
	printf("Searching Back\n");
	int * synclocs;
	synclocs = (int *) malloc((sampsbefore+sampsafter+1)*sizeof(int));

	for(int i = 0; i< sampsbefore;i++){
		//printf("do work\n");

		locMax = 0;
		for(j = floor(Q-(fs/search))-1;j<floor(Q+(fs/search));j++){

			if(hA[j] > locMax){

				locMax = hA[j];
				locMaxIndex = j;

			}
			
		}
		synclocs[sampsbefore - i - 1] = locMaxIndex+1;
		//printf("%d\n",locMaxIndex );
		Q = locMaxIndex - floor(fs/2);

	}

	synclocs[sampsbefore] = maxAmpIndex+1;

	printf("Searching Forward\n");

	Q = floor(maxAmpIndex + (fs/2));
	for(int i = 0; i< sampsafter;i++){
		//printf("do work\n");

		locMax = 0;
		for(j =floor(Q-(fs/search))-1;j<floor(Q+(fs/search));j++){

			if(hA[j] > locMax){

				locMax = hA[j];
				locMaxIndex = j;
				
			}
			
		}
		synclocs[sampsbefore+i+1] = locMaxIndex+1;
		//printf("%d\n",locMaxIndex);
		//printf("%d\n",sampsafter+sampsbefore+1 );

		Q = locMaxIndex + floor(fs/2);

	}
	free(hA);

	printf("Building Image Array\n");


	pngwriter png(2080,sampsbefore+sampsafter+1,0,argv[2]);

	obuf = (double *) malloc(((sampsbefore+sampsafter+1)*floor(fs/2))/sizeof(unsigned char)*sizeof(double));
	maxAmp = 0;

	for(int i=0;i<sampsbefore+sampsafter+1;i++){

		//synclocs[i] = synclocs[i];
		// synclocsfile << "," << synclocs[i];
		for(int j = 0;j < floor(fs/2);j++){

			
			obuf[i*int(floor(fs/2))+j] = outconv[synclocs[i]+j];
			

		}

		
	}

	free(outconv);
	double minAmp = 0;
	maxAmp = 0;

	for(int i=0;i<(sampsbefore+sampsafter+1)*int(floor(fs/2));i++){

		if(obuf[i]<minAmp){

			minAmp = obuf[i];

		}
	}


	maxAmp = 0;
	for(int i=0;i<(sampsbefore+sampsafter+1)*int(floor(fs/2));i++){

		obuf[i] = obuf[i] + fabs(minAmp);

		if(obuf[i]>maxAmp){

			maxAmp = obuf[i];

		}
	}
	for(int i=0;i<(sampsbefore+sampsafter+1)*int(floor(fs/2));i++){

		obuf[i] = obuf[i]/maxAmp*255;

	}


	for(int i=0;i<sampsbefore+sampsafter+1;i++){

		for(int j = 0;j < floor(fs/2);j++){

			png.plot(i*int(floor(fs/2)),j, obuf[i*int(floor(fs/2))+j], obuf[i*int(floor(fs/2))+j], obuf[i*int(floor(fs/2))+j]);

		}

	}

	png.close();

	// std::vector<int> compression_params;
 //    compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
 //    compression_params.push_back(9);
    //float datak[2][5] = {{1,2,3,4,5},{7,8,9,10,11}};
    
	//downsampler(obuf,floor(fs/2),sampsbefore+sampsafter+1,2080,2);
	//cv::Mat A = cv::Mat(sampsbefore+sampsafter+1,2080, CV_64FC1, obuf);

	free(obuf);

	printf("Resizing Image\n");
	
	//cv::resize(A,A,cv::Size(2080, sampsbefore+sampsafter+1),cv::INTER_AREA);
	// cv::transpose(A,A);
	// cv::flip(A,A,1);
	// cv::transpose(A,A);
	// cv::flip(A,A,1);

	printf("Storing Image\n");

	//cv::imwrite(argv[2], A);

	// double * testarr;
	// testarr = (double *) malloc(9/sizeof(unsigned char)*sizeof(double));
	// for(int i =0;i<16;i++){

	// 	testarr[i] = i;
	// }
	// downsampler(testarr,4,4,2,2);

	// for(int i =0;i<8;i++){

	// 	printf("%f\n",testarr[i] );
	// }
	
	printf("DONE\n");
	fclose(fp);

	// sqrwav << std::endl;
	// procwav << std::endl;
	// resultwav << std::endl;
	// synclocsfile << std::endl;
	// sqrwav.close();
	// procwav.close();
	// resultwav.close();
	// synclocsfile.close();

	return 0;

}


// 

// void downsampler(double *orgimg, int orgimgW, int orgimgH,int scaleimgW, int scaleimgH){

// 	// if(scaleimgW > orgimgW ){

// 	// 	printf("Error\n");

// 	// }
// 	int scaleDist = orgimgW - scaleimgW;

// 	printf("%d\n",scaleDist );

// 	for (int k =0; k<scaleDist;k++){

// 		for (int i = 0; i<orgimgH; i++){

// 			for(int j = 0; j<orgimgW-1;j++){

// 				orgimg[i*orgimgW+j-i] = (orgimg[i*orgimgW+j] + orgimg[i*orgimgW+j+1])/2;

// 			}

// 		}

// 		orgimgW = orgimgW - 1;
// 	}

// 	orgimg = (double *) realloc(orgimg,(orgimgW*orgimgH)/sizeof(unsigned char)*sizeof(double));

// }


bool isRiff(FILE * fp,unsigned char * buf){
	//Checks if the file follows riff format by checking first 4 bytes

	fseek(fp ,0,SEEK_SET);
	fread(buf,4,1,fp);

	if(buf[0]==0x52 && buf[1]==0x49 && buf[2]==0x46 && buf[3]==0x46){

	 	printf("RIFF: Yes\n");
	 	return(1);
	}
 	else{

		printf("File is not RIFF\n");
		return(0);

	}


}

int fileSize(FILE * fp, unsigned char * buf){
	//reads chunk size, which is file size in bits - 32

	fseek(fp ,4,SEEK_SET);
	fread(buf,4,1,fp);
	int Int = buf[0] | ( (int)buf[1] << 8 ) | ( (int)buf[2] << 16 ) | ( (int)buf[3] << 24 );
 	printf("File Size: ");
 	printf("%d\n",Int+32);
 	return(Int+32);

}

bool isWave(FILE * fp,unsigned char * buf){
	//reads if file is WAVE type

	fseek(fp ,8,SEEK_SET);
	fread(buf,4,1,fp);

	if(buf[0]==0x57 && buf[1]==0x41 && buf[2]==0x56 && buf[3]==0x45){

	 	printf("WAVE: Yes\n");
	 	return(1);
	}
 	else{

		printf("File is not WAVE\n");
		return(0);

	}


}

bool isPCM16(FILE * fp,unsigned char * buf){
	//See if header is 16 bytes, needs uncompressed 

	fseek(fp ,16,SEEK_SET);
	fread(buf,4,1,fp);

	if(buf[0]==0x10 && buf[1]==0x00 && buf[2]==0x00 && buf[3]==0x00){

	 	printf("PCM Size 16: Yes\n");
	 	return(1);
	}
 	else{

		printf("Not PCM\n");
		return(0);

	}


}

bool isPCM(FILE * fp,unsigned char * buf){
	//See if header is 16 bytes, needs uncompressed 

	fseek(fp ,20,SEEK_SET);
	fread(buf,2,1,fp);

	if(buf[0]==0x01 && buf[1]==0x00){

	 	printf("PCM Encoding: Yes\n");
	 	return(1);
	}
 	else{

		printf("Not PCM\n");
		return(0);

	}


}

int numChan(FILE * fp, unsigned char * buf){
	//reads chunk size, which is file size in bits - 32

	fseek(fp ,22,SEEK_SET);
	fread(buf,2,1,fp);
	int Int = buf[0] | ( (int)buf[1] << 8 ) ;
 	printf("Number of Channels: ");
 	printf("%d\n",Int);
 	return(Int);

}

int sampRate(FILE * fp, unsigned char * buf){
	//reads chunk size, which is file size in bits - 32

	fseek(fp ,24,SEEK_SET);
	fread(buf,4,1,fp);
	int Int = buf[0] | ( (int)buf[1] << 8 ) | ( (int)buf[2] << 16 ) | ( (int)buf[3] << 24 );
 	printf("Sample Rate: ");
 	printf("%d\n",Int);
 	return(Int);

}

int bitPerSamp(FILE * fp, unsigned char * buf){
	//reads chunk size, which is file size in bits - 32

	fseek(fp ,34,SEEK_SET);
	fread(buf,4,1,fp);
	int Int = buf[0] | ( (int)buf[1] << 8 ) ;
 	printf("Bits per sample: ");
 	printf("%d\n",Int);
 	return(Int);

}

int subChunk2Size(FILE * fp, unsigned char * buf){
	//reads chunk size, which is file size in bits - 32

	fseek(fp ,40,SEEK_SET);
	fread(buf,4,1,fp);
	int Int = buf[0] | ( (int)buf[1] << 8 ) | ( (int)buf[2] << 16 ) | ( (int)buf[3] << 24 );
 	printf("Size of sound data (kilobytes): ");
 	printf("%0.f\n",(double)Int/1000.0);
 	return(Int);

}

int blockAlign(FILE * fp, unsigned char * buf){
	//reads chunk size, which is file size in bits - 32

	fseek(fp ,32,SEEK_SET);
	fread(buf,4,1,fp);
	int Int = buf[0] | ( (int)buf[1] << 8 ) ;
 	printf("Bytes per sample (all Channels): ");
 	printf("%d\n",Int);
 	return(Int);

}

double *conv(double *A, int lenA, double *B, int lenB,
		int *lenC, char *cmethod)
{
	int nconv;
	int i, j, i1;
	int nb;
	double tmp;
	double *C=NULL;
	double *Result=NULL;
 
	nb = 0;
 
	//allocated convolution array
	nconv = lenA + lenB - 1;
	C = (double*) calloc (nconv, sizeof(double));
 
	//convolution process
	for (i = 0; i < nconv; i++)
	{
		i1 = i;
		tmp = 0.0;
		for (j = 0; j < lenB; j++)
		{
			if (i1 >= 0 && i1 < lenA)
				tmp = tmp + (A[i1] * B[j]);
 
			i1 = i1 - 1;
			C[i] = tmp;
		}
	}
 
	//-----------------------------------------------------
	//create the result
	//-----------------------------------------------------
	if (strcmp(cmethod, "full") == 0)
	{
		//get length of convolution array
		*lenC = nconv;
		return (C);
	}
	else if (strcmp(cmethod, "same") == 0)
	{
		*lenC = lenA;
		nb = (lenB - 1) % 2;
		if (nb != 0)
			nb = (lenB + 1) / 2;
		else
			nb = (lenB - 1) / 2;
 
		Result = (double*) calloc(lenA, sizeof(double));
		for (i = 0; i < lenA; i++)
			Result[i] = C[nb + i];
 
		free(C);
		return (Result);
	}
	else //default=full method
	{
		//get length of convolution array
		*lenC = nconv;
		return (C);
	}
}



double *filterBUT(double * B, double * A, double * X, double * Zi, int input_size, int filter_order)
{

    double * Y = (double *) malloc(input_size*sizeof(double));

    for (int i = 0; i < input_size; ++i)
    {
        int order = filter_order - 1;
        while (order)
        {
            if (i >= order)
                Zi[order - 1] = B[order] * X[i - order] - A[order] * Y[i - order] + Zi[order];
            --order;
        }
        Y[i] = B[0] * X[i] + Zi[0];
        //printf("%f\n",Y[i] );
    }
    //Zi.resize(filter_order - 1);
    return Y;
}




