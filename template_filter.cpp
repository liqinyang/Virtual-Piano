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

struct pixel_data {
	pixel_type blue;
	pixel_type green;
	pixel_type red;
};
void colourred(int a,int b,	unsigned char red[],unsigned char blue[],unsigned char green[]){
		colourred_label0:for(int i=a;i<=b;i++){
			//red[i]=red[i]+50;
			blue[i]=blue[i]/2;
			green[i]=green[i]/2;
		}

}

void gray_filter(volatile uint32_t* in_data, volatile uint32_t* out_data, int w, int h, int parameter_1){
#pragma HLS INTERFACE s_axilite port=return
#pragma HLS INTERFACE s_axilite port=parameter_1
#pragma HLS INTERFACE s_axilite port=w
#pragma HLS INTERFACE s_axilite port=h

#pragma HLS INTERFACE m_axi depth=1555200 port=in_data offset=slave
#pragma HLS INTERFACE m_axi depth=1555200 port=out_data offset=slave
	volatile uint32_t* start = in_data;
	unsigned char red[1920],blue[1920],green[1920];
	short finger_edges[20],key_edges[20];
	unsigned char finger_count=0,key_count=0;
	short finger=0;
	float Y=0;
	short key=0;
	in_data+=w*200;
	unsigned int last_edge = 0;
	gray_filter_label0:for(int j=0;j<w;++j){
		unsigned int current = *in_data++;
		unsigned char last_Y = int(Y);
		unsigned char in_r = current & 0xFF;
		unsigned char in_b = (current >> 8) & 0xFF;
		unsigned char in_g = (current >> 16) & 0xFF;
		Y = 0.2126f*in_r  + 0.7152f*in_g  + 0.0722f*in_b ;
		int d=int(Y)-last_Y;
		if (d<0){
			d=-d;
		}
		if(d > 20 ){
			if(j-last_edge>90){
				key_edges[key_count]=(last_edge);
				key_count++;
				key_edges[key_count]=j;
				key_count++;
			}
			last_edge=j;
		}
	}
	in_data+=w*350;;
	Y=0;
	last_edge = 0;
	gray_filter_label1:for(int j=0;j<w;++j){
		unsigned int current = *in_data++;
		unsigned char last_Y = int(Y);
		unsigned char in_r = current & 0xFF;
		unsigned char in_b = (current >> 8) & 0xFF;
		unsigned char in_g = (current >> 16) & 0xFF;
		red[j]=in_r;
		blue[j]=in_b;
		green[j]=in_g;
		Y = 0.2126f*in_r  + 0.7152f*in_g  + 0.0722f*in_b ;
		int d=int(Y)-last_Y;
		if (d<0){
			d=-d;
		}
		if(d > 20 ){
			if((j-last_edge>10)&&(j-last_edge<90)){
				finger_edges[finger_count]=(last_edge);
				finger_count++;
				finger_edges[finger_count]=j;
				finger_count++;
			}
			last_edge=j;
		}
	}

	for (int i=0;i<finger_count/2;i++){
		gray_filter_label2:for(int j=finger_edges[2*i];j<finger_edges[2*i+1];j++){
			if ((red[j]>100)&&(red[j]<180)){
				if ((blue[j]>80)&&(blue[j]<140)){
					if ((green[j]>50)&&(green[j]<100)){
						if(finger==0){
							finger=j;
						}
					}
				}
			}
		}
	}

	gray_filter_label3:for (int i=0;i<key_count;i++){
//		std::cout<<key_edges[i]<<std::endl;
		if (finger>key_edges[i]){
			key=i/2+1;
		}
	}
//	std::cout<<"finger is "<<finger<<std::endl;
//	std::cout<<"key is "<<key<<std::endl;
//	for (int i=0;i<finger_count;i++){
//			std::cout<<finger_edges[i]<<std::endl;
//		}
	int test = 0;
	in_data=start;
	for (int i = 0; i < h; ++i) {

		//		unsigned char last_r = 0;
		//		unsigned char last_b = 0;
		//		unsigned char last_g = 0;


		for (int j = 0; j < w; ++j) {

#pragma HLS PIPELINE II=1
#pragma HLS LOOP_FLATTEN off

//			std::cout << "(i,j) " << i << "," << j << std::endl;
			unsigned int current = *in_data++;
			unsigned char last_Y = int(Y);
			unsigned char in_r = current & 0xFF;
			unsigned char in_b = (current >> 8) & 0xFF;
			unsigned char in_g = (current >> 16) & 0xFF;


			//			unsigned char out_r = (in_r + last_r) >> 1;
			//			unsigned char out_b = 0;//(in_b + last_b) >> 1;
			//			unsigned char out_g = (in_g + last_g) >> 1;
			//
			//			last_r = in_r;
			//			last_b = in_b;
			//			last_g = in_g;

			unsigned char out_r = 0;
			unsigned char out_b = 0;
			unsigned char out_g = 0;


			red[j]=in_r;
			blue[j]=in_b;
			green[j]=in_g;
		}
		if(key!=0){
			colourred(key_edges[2*key-2],key_edges[2*key-1],red,blue,green);
		}

		gray_filter_label4:for (int j = 0; j < w; ++j) {
			if((i==200)||(i==551)){
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
				unsigned int output = red[j] | (blue[j] << 8) | (green[j] << 16);
				*out_data++ = output;
			}
		}

	}


}


