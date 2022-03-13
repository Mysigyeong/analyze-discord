#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <cstring>

using namespace std;

// 0000000000b29790 T _ZN3std3sys4unix17decode_error_kind17h9f951928aa06a8aaE
// 00000000002eea40 t opus_decode
// 00000000002ec4e0 t opus_decoder_create
// 00000000002eec10 t opus_decoder_ctl
// 2ec310 opus_decoder_get_size
void* decode_error_kind_ptr = NULL;
void* opus_decode_ptr = NULL;
void* opus_decoder_create_ptr = NULL;
void* opus_decoder_ctl_ptr = NULL;
void* opus_decoder_get_size_ptr = NULL;

const int ERROR_KIND_OFFSET = 0xb29790;
const int OPUS_DECODE_OFFSET = 0x2eea40;
const int OPUS_DECODER_CREATE_OFFSET = 0x2ec4e0;
const int OPUS_DECODER_CTL_OFFSET = 0x2eec10;
const int OPUS_DECODER_GET_SIZE_OFFSET = 0x2ec310;

//#define OPUS_RESET_STATE 4028
const int OPUS_RESET_STATE = 4028;

void init_func_ptr(void* lib);
void read_data(char* filename, char** data, int* len);
void fuzzme(char* filename);


int main(int argc, char** argv)
{
	void* lib = NULL;

	if (argc != 2) {
		cerr << "Usage: ./harness filename" << endl;
		exit(1);
	}
	
	lib = dlopen("/root/discord_voice.node", RTLD_LAZY);
	if (lib == NULL) {
		cerr << "lib is null\n" << dlerror() << endl;
		exit(1);
	}


	init_func_ptr(lib);

	fuzzme(argv[1]);
	
	dlclose(lib);
	return 0;
}

void init_func_ptr(void* lib)
{
	int difference = ERROR_KIND_OFFSET - OPUS_DECODE_OFFSET;
	
	decode_error_kind_ptr = dlsym(lib, "_ZN3std3sys4unix17decode_error_kind17h9f951928aa06a8aaE");
	opus_decode_ptr = decode_error_kind_ptr - difference;
	
	difference = ERROR_KIND_OFFSET - OPUS_DECODER_CREATE_OFFSET;
	opus_decoder_create_ptr = decode_error_kind_ptr - difference;

	difference = ERROR_KIND_OFFSET - OPUS_DECODER_CTL_OFFSET;
	opus_decoder_ctl_ptr = decode_error_kind_ptr - difference;

	difference = ERROR_KIND_OFFSET - OPUS_DECODER_GET_SIZE_OFFSET;
	opus_decoder_get_size_ptr = decode_error_kind_ptr - difference;
}


void read_data(char* filename, char** data, int* len)
{
	int length;
	char* buffer;
	ifstream is;
	is.open (filename, ios::binary );
	// get length of file
	is.seekg(0, ios::end);
	length = is.tellg();
	is.seekg(0, ios::beg);

	// allocate memory
	buffer = (char*)malloc(length*sizeof(char));
	// read data as a block
	is.read (buffer,length);
	is.close();

	*len = length;
	*data = buffer;
}


void fuzzme(char* filename)
{
	void* decoder = NULL;
	short* pcm = NULL;
	int error = -1;
	int frame_size = 0x1680;
	int decoded = 0;
	char* data = NULL;
	int len = 0;

	int decoder_size = ((int(*)(int))opus_decoder_get_size_ptr)(2);
	cout << "decoder size = " << decoder_size << endl;
	
	//decoder = opus_decoder_create(sampling_rate, channels, *error)
	decoder = ((void*(*)(int, int, int*))opus_decoder_create_ptr)(48000, 2, &error);
	int cnt = 0;
	unsigned char* ptr = (unsigned char*)decoder;
/*
	for (int i = 0; i < decoder_size; i++) {
		printf("%02x ", *(ptr + i));
		cnt++;
		if (cnt == 16) {
			puts("");
			cnt = 0;
		}
	}
	puts("");
*/
	((int(*)(void*, int))opus_decoder_ctl_ptr)(decoder, OPUS_RESET_STATE);
/*
	cnt = 0;
	ptr = (unsigned char*)decoder;
	for (int i = 0; i < decoder_size; i++) {
		printf("%02x ", *(ptr + i));
		cnt++;
		if (cnt == 16) {
			puts("");
			cnt = 0;
		}
	}
*/

	read_data(filename, &data, &len);
	cout << "data len = " << len << endl;
//	[out] pcm
	pcm = (short*)malloc(frame_size*2*sizeof(short));
//	for (int i = 0; i < frame_size; i++) {
//		pcm[i] = 10;
//	}
	decoded = ((int(*)(void*, void*, int, void*, int, int))opus_decode_ptr)(decoder, data, len, pcm, frame_size, 0);
	
	cout << "decoded = " << decoded << endl;

/*
	ptr = (unsigned char*)pcm;
	cnt = 0;
	for (int i = 0; i < decoded; i++) {
		printf("%02x ", *(ptr + i));
		cnt++;
		if (cnt == 16) {
			puts("");
			cnt = 0;
		}
	}

	puts("");
	cnt = 0;
	ptr = (unsigned char*)decoder;
	for (int i = 0; i < decoder_size; i++) {
		printf("%02x ", *(ptr + i));
		cnt++;
		if (cnt == 16) {
			puts("");
			cnt = 0;
		}
	}
*/
	free(decoder);
	free(pcm);
	free(data);
}
