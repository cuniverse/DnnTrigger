// dnn_decoder.h
// DNN decoder(PowerAI DeepNet) C++ wrapper for DNN trigger
// 2016-09 by mckeum

#ifndef __DNN_DECODER_HPP__
#define __DNN_DECODER_HPP__


#ifdef DNN_EXPORT
#define POWER_DEEPNET_API __declspec(dllexport)
#else  // if not
#define POWER_DEEPNET_API
#endif 


typedef struct DNN_Resource DNN_Resource;
typedef struct Deepnet Deepnet;
typedef struct DNN_LAYER_UNIT DNN_LAYER_UNIT;


class POWER_DEEPNET_API CDnnDecoder
{
private:
	Deepnet* pDeepnet;
	DNN_LAYER_UNIT* p_dnn_output;

	float* feat_pool;

	int concat_before;	// concatnate before n frames (past frames)
	int concat_after;	// concatnate after n frames (future frames)
	int feat_dim;

	int frame_input;

	int err;	

public:
	CDnnDecoder(const char root_path[], const char config_path[]);
	~CDnnDecoder();
	int decode(float* in, float* out);
	int reset();

	int getNumOutNode();
	int getError() { return err; }
	
};

#endif	// __DNN_DECODER_HPP__
