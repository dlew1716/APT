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
double *conv(double *A, double *B, int lenA, int lenB, int *lenC);
double *outconv;
int lenC=0;
double butter_11025_13_order[13] = {-0.00132381, 0.0029468, 0.02127082, 0.06446164,  0.12649509, 0.18309832, 0.20610228, 0.18309832, 0.12649509, 0.06446164, 0.02127082, 0.0029468, -0.00132381};

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
	int fs = 0;

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
		int * sqbuf;
		abuf = (int *) malloc(audioBufSize/sizeof(unsigned char)*sizeof(int));
		bbuf = (double *) malloc(audioBufSize/sizeof(unsigned char)*sizeof(double));

		int sqbufsize = floor(39.0*fs/4160.0);  // Ts of sync wave
		printf("Square Pulse Array Size:  ");
		printf("%d\n",sqbufsize);

		sqbuf = (int *) malloc(sqbufsize/sizeof(unsigned char)*sizeof(int));

		for(int i = 0;i<sqbufsize;i++){

			//if()



		}


		for(int i = 0;i<audioBufSize/2;i++){

			abuf[i] = buf[i*2] | ( (int)buf[i*2+1] << 8 ) | ( 0 << 16 ) | ( 0 << 24 );
			//printf("%d\n",abuf[i] );
			//3839815

			
		}



	//}

	double maxAmp = abs(abuf[0]);
	double sumVal = 0;

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

	outconv = conv(bbuf,butter_11025_13_order,audioBufSize/2,13,&lenC);
	free(bbuf);
	printf("Convolved\n");

	sumVal =0.0;
	maxAmp =0.0;

	for(int i =0;i<lenC;i++){

		
		if(maxAmp<fabs(outconv[i])){

			maxAmp = fabs(outconv[i]);


		}

		sumVal = sumVal + outconv[i];

	}

	avgVal = sumVal/lenC/maxAmp;
	printf("Max Amplitude After Conv:  ");
	printf("%f\n",maxAmp );
	printf("Average Normalized Value After Conv:  ");
	printf("%f\n",avgVal );

	for(int i =0;i<lenC;i++){

		outconv[i] = (outconv[i]/-avgVal)/maxAmp;

		if(outconv[i]<0){
			outconv[i] = -1*outconv[i];
		}


	}

	
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

