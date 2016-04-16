#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>




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
double *conv(double *A, double *B, int lenA, int lenB, int *lenC);
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

//fix where you shift in only 2 bytes into integer


int main() {

	FILE * fp;
	FILE * pFile;
	
	fp = fopen("mono_best.wav","rb");

	if(!fp){
		printf("Unable to open file");
		return 1;
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

		int * abuf;
		double * bbuf;
		double * sqbuf;
		double * hA;
		abuf = (int *) malloc(audioBufSize/sizeof(unsigned char)*sizeof(int));
		bbuf = (double *) malloc(audioBufSize/sizeof(unsigned char)*sizeof(double));


		int sqbufsize = floor(32.0*fs/4160.0);  // Ts of sync wave
		double tau = 1.0/4160.0;
		double timeCount = 0;
		printf("Square Pulse Array Size:  ");
		printf("%d\n",sqbufsize);

		sqbuf = (double *) malloc(sqbufsize/sizeof(unsigned char)*sizeof(double));

		for(int i = 0;i<sqbufsize;i++){

			timeCount = timeCount + (1.0/fs);
			sqbuf[i] = 0;

			for(int j =0; j<28;j=j+4){

				// printf("%f\n",(4*j)*tau );
				// printf("%f\n",(6*j)*tau );
				// printf("%f\n\n",timeCount );

				if(timeCount<((6+j)*tau) && timeCount>=((4+j)*tau)){

					sqbuf[i] = 1;

				}


			}
			//printf("%d\n", sqbuf[i]);
			//printf("-------------\n");

		}


		for(int i = 0;i<audioBufSize/2;i++){

			abuf[i] = buf[i*2] | ( (int)buf[i*2+1] << 8 ) | ( 0 << 16 ) | ( 0 << 24 );
			//printf("%d\n",abuf[i] );
			//3839815

			
		}



	//}

	double maxAmp = abs(abuf[0]);
	double sumVal = 0;
	int maxAmpIndex = 0;

	for(int i =0;i<audioBufSize/2;i++){

		
		if(maxAmp<abs(abuf[i])){

			maxAmp = abs(abuf[i]);

		}

		sumVal = sumVal + abuf[i];

	}
	double avgVal = sumVal/audioBufSize/2/maxAmp;
	printf("Max Amplitude:  ");
	printf("%f\n",maxAmp );
	printf("Average Normalized Value:  ");
	printf("%f\n",avgVal );


	for(int i =0;i<audioBufSize/2;i++){

		bbuf[i] = abuf[i]/maxAmp-avgVal;

		if(bbuf[i]<0){
			bbuf[i] = -1*bbuf[i];
		}


	}
	free(abuf);

	outconv = filterBUT(Bcoe,Acoe,bbuf,Zi,audioBufSize/2,3);

	// for(int i =0; i<audioBufSize/2;i++){

	// 	printf("%f\n",outconv[i] );
	// }

	free(bbuf);
	printf("Convolved\n");

	sumVal =0.0;
	maxAmp =0.0;

	for(int i =0;i<audioBufSize/2;i++){

		
		if(maxAmp<fabs(outconv[i])){

			maxAmp = fabs(outconv[i]);



		}

		sumVal = sumVal + outconv[i];

	}

	avgVal = sumVal/(audioBufSize/2)/maxAmp;
	printf("Max Amplitude After Conv:  ");
	printf("%f\n",maxAmp );
	printf("Average Normalized Value After Conv:  ");
	printf("%f\n",avgVal );

	for(int i =0;i<audioBufSize/2;i++){

		outconv[i] = (outconv[i]/-avgVal)/maxAmp;

		if(outconv[i]<0){
			outconv[i] = -1*outconv[i];
		}
		

	}

	hA = conv(outconv,sqbuf,audioBufSize/2,sqbufsize,&lenhA);
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

	int sampsbefore = floor(maxAmpIndex/(fs/2));
	int sampsafter = floor((lenhA-maxAmpIndex)/(fs/2));
	int Q =0;
	int j ;
	double locMax;
	int locMaxIndex;
	Q = floor(maxAmpIndex - (fs/2));
	int search =7; //move to user configurable, change with samp rate?

	int * synclocs;
	synclocs = (int *) malloc((sampsbefore+sampsafter+1)*sizeof(int));

	for(int i = 0; i< sampsbefore;i++){
		//printf("do work\n");

		locMax = 0;
		for(j =floor(Q-(fs/search));j<floor(Q+(fs/search));j++){

			if(hA[j] > locMax){

				locMax = hA[j];
				locMaxIndex = j;

				
			}
			
		}
		synclocs[sampsbefore -i-1] = locMaxIndex;
		//printf("%d\n",locMaxIndex );
		Q = locMaxIndex - floor(fs/2);

	}



	synclocs[sampsbefore] = maxAmpIndex;

	Q = floor(maxAmpIndex + (fs/2));
	for(int i = 0; i< sampsafter;i++){
		//printf("do work\n");

		locMax = 0;
		for(j =floor(Q-(fs/search));j<floor(Q+(fs/search));j++){

			if(hA[j] > locMax){

				locMax = hA[j];
				locMaxIndex = j;
				
			}
			
		}
		synclocs[sampsbefore+i+1] = locMaxIndex;
		//printf("%d\n",locMaxIndex);
		//printf("%d\n",sampsafter+sampsbefore+1 );

		Q = locMaxIndex + floor(fs/2);

	}

	
	double **img =(double**) malloc((sampsbefore+sampsafter+1)*sizeof(double*));
	maxAmp = 0;
	for(int i=0;i<sampsbefore+sampsafter+1;i++){

		img[i] = (double*)malloc(double(fs/2)*sizeof(double));
		for(int j = 0;j < floor(fs/2);j++){

			img[i][j] = outconv[synclocs[i]+j];
			if(img[i][j]>maxAmp){

				
				// if(img[i][j]>30.0){

				// 	img[i][j] =0.0;

				// }
				maxAmp = img[i][j];
				
				//printf("%f\n",img[i][j] );

			}
			
		}
	}
	// printf("%f\n",maxAmp );

	// for(int i=0;i<sampsbefore+sampsafter+1;i++){

		
	// 	for(int j = 0;j < floor(fs/2);j++){

	// 		img[i][j] = img[i][j]/maxAmp;
	// 		//printf("%f\n",img[i][j] );

	// 	}
	// }

	// vector<int> compression_params;
 //    compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
 //    compression_params.push_back(9);
    //float datak[2][5] = {{1,2,3,4,5},{7,8,9,10,11}};
    

	//Mat A = Mat(sampsbefore+sampsafter+1, floor(fs/2), CV_64FC1, img);

	




	// for(int i =0;i< (sampsbefore+sampsafter+1);i++){

	// 	printf("%d\n",synclocs[i] );

	// }
	
	printf("DONE\n");
	fclose(fp);
	return 0;

}

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

double *conv(double *A, double *B, int lenA, int lenB, int *lenC)
{
	int nconv;
	int i, j, i1;
	double tmp;
	double *C;
 
	//allocated convolution array	
	nconv = lenA+lenB-1;
	C = (double*) calloc(nconv, sizeof(double));

	//convolution process
	for (i=0; i<nconv; i++)
	{
		i1 = i;
		tmp = 0.0;
		for (j=0; j<lenB; j++)
		{
			if(i1>=0 && i1<lenA)
				tmp = tmp + ((double)A[i1]*B[j]);
 
			i1 = i1-1;
			C[i] = tmp;
		}

	}
 	
	//get length of convolution array	
	(*lenC) = nconv;

 
	//return convolution array
	return(C);
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