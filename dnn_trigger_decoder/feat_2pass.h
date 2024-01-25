#ifndef __TRGGER_FEAT_2PASS_H__
#define __TRGGER_FEAT_2PASS_H__

#include <stdint.h>

class CFeat2pass {
public:
	CFeat2pass(const char root_path[]);
	~CFeat2pass();
	int getFeature(const int in_samples, const int16_t in_pcm[], long* len_feat, float* out_feat);
	int reset();
	int getError() { return err; };
	static bool setChannel(int ch) {
		if (fe_connected == false) {
			fe_channel = ch;
			return true;
		} else {
			return false;
		}
		return false;
	}
private:
	static bool fe_connected;	// singleton FE loaded
	static int fe_channel;
	long chan_id;
	int err;
};

#endif	// __TRGGER_FEAT_2PASS_H__
