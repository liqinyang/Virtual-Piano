#include <ap_fixed.h>
#include <ap_int.h>
#include <stdint.h>
#include <assert.h>
#include <vector>
typedef ap_uint<8> pixel_type;
typedef ap_int<8> pixel_type_s;
typedef ap_uint<96> u96b;
typedef ap_uint<32> word_32;
typedef ap_ufixed<8,0, AP_RND, AP_SAT> comp_type;
typedef ap_fixed<10,2, AP_RND, AP_SAT> coeff_type;
typedef ap_uint<11> myshort;

struct pixel_data {
	pixel_type blue;
	pixel_type green;
	pixel_type red;
};

void gray_filter(volatile uint32_t* in_data, volatile uint32_t* out_data, int w, int h, int parameter_1, int& key_number,int& pitch_number){
#pragma HLS INTERFACE s_axilite port=pitch_number
#pragma HLS INTERFACE s_axilite port=key_number
#pragma HLS INTERFACE s_axilite port=return
#pragma HLS INTERFACE s_axilite port=parameter_1
#pragma HLS INTERFACE s_axilite port=w
#pragma HLS INTERFACE s_axilite port=h

#pragma HLS INTERFACE m_axi depth=1555200 port=in_data offset=slave
#pragma HLS INTERFACE m_axi depth=1555200 port=out_data offset=slave
	volatile uint32_t* start = in_data;
	myshort fingers[200],key_edges[20],pitchred[200];
	unsigned char key_count=0;
	myshort finger=0,finger_count=0,pitchred_count=0,pitchpixel=0;
	unsigned char Y=0;
	unsigned char key=0,pitch=2;
	in_data+=w*99;
	pitchred:for(int j=0;j<1280;++j){
		unsigned int current = *in_data++;
		unsigned char in_r = current & 0xFF;
		unsigned char in_b = (current >> 8) & 0xFF;
		unsigned char in_g = (current >> 16) & 0xFF;
		if (in_r>120){
			if (in_b<80){
				if (in_g<80){
					pitchred[pitchred_count]=j;
					pitchred_count++;
				}
			}
		}
	}


	in_data+=w*250;
	unsigned int last_edge = 0;
	keyedge:for(int j=0;j<1280;++j){
		unsigned int current = *in_data++;
		unsigned char last_Y = Y;
		unsigned char in_r = current & 0xFF;
		unsigned char in_b = (current >> 8) & 0xFF;
		unsigned char in_g = (current >> 16) & 0xFF;
		Y = 0.2126f*in_r  + 0.7152f*in_g  + 0.0722f*in_b ;
		unsigned char d;
		if(Y>last_Y){
			d=Y-last_Y;
		}
		else{
			d=last_Y-Y;
		}
		if(d > 30 ){
			if(last_edge==0){
				last_edge=j;
			}
			if(j-last_edge>90){
				key_edges[key_count]=(last_edge);
				key_count++;
				key_edges[key_count]=j;
				key_count++;
			}
			last_edge=j;
		}
	}
	in_data+=w*300;;
	Y=0;
	last_edge = 0;
	finger:for(int j=0;j<1280;++j){
		unsigned int current = *in_data++;
		unsigned char in_r = current & 0xFF;
		unsigned char in_b = (current >> 8) & 0xFF;
		unsigned char in_g = (current >> 16) & 0xFF;
		if ((in_r>70)&&(in_r<140)){
			if ((in_b>40)&&(in_b<90)){
				if ((in_g>35)&&(in_g<75)){
					fingers[finger_count]=j;
					finger_count++;
				}
			}
		}
	}
	if (finger_count<30){
		finger=0;
	}
	else{
		finger=fingers[finger_count/2];
	}
	if (pitchred_count<30){
		pitchpixel=0;
	}
	else{
		pitchpixel=pitchred[pitchred_count/2];
	}
	gray_filter_label3:for (int i=0;i<key_count;i++){
//		std::cout<<key_edges[i]<<std::endl;
		if (finger>key_edges[i]){
			key=i/2+1;
		}
		if (pitchpixel>key_edges[i]){
			pitch=i/2-1;
		}
	}
//	std::cout<<"finger is "<<finger<<std::endl;
//	std::cout<<"key is "<<key<<std::endl;
//	for (int i=0;i<pitchred_count;i++){
//			std::cout<<pitchred[i]<<std::endl;
//		}
	int test = 0;
	in_data=start;
	int redbegin = -1, redend=-1;
	if(key!=0){
		redbegin = key_edges[2*key-2];
		redend=key_edges[2*key-1];
	}
	for (int i = 0; i < h; ++i) {

		//		unsigned char last_r = 0;
		//		unsigned char last_b = 0;
		//		unsigned char last_g = 0;


		outputing:for (int j = 0; j < 1280; ++j) {

#pragma HLS PIPELINE II=1
#pragma HLS LOOP_FLATTEN off

			unsigned int current = *in_data++;
			if((i==99)||(i==350)||(i==651)){
				if (j==finger){
					unsigned int output = 0 | (0 << 8) | (0xFF << 16);
					*out_data++ = output;
				}
				else{
					unsigned int output = 0 | (0xFF << 8) | (0 << 16);
					*out_data++ = output;
				}
			}
			else{
				if((j>=redbegin)&&(j<=redend)){
					unsigned char in_r = current & 0xFF;
					unsigned char in_b = (current >> 8) & 0xFF;
					unsigned char in_g = (current >> 16) & 0xFF;
					in_b=in_b>>1;
					in_g=in_g>>1;
					unsigned int output = in_r | (in_b << 8) | (in_g << 16);
					*out_data++ = output;
				}
				else{
					*out_data++ = current;
				}
			}
		}


	}
	key_number=key;
	pitch_number=pitch;

}


