#include "swell.h"
#include <math.h>

using namespace daisysp;

void Swell::Init(float attack, float sensitivity) {

}

float Swell::GetRampValue() {
	if (send_flag_ && send_value_ < 1.0f) {
		ramp = ; // TODO: figure out how to set rate increment of volume
		send_value_ = fclamp(send_value_ + ramp, 0.0f, 1.0f);
	}
	return send_value_;
}

float Swell::Process(float in) {
	out = in;
	if (in < threshold) {
		send_flag_ = false;
		send_value_ = 0;
		return 0.0f
	}
	send_flag_ = true;
	return GetRampValue() * in;
}

void Swell::UpdateParams(float attack, float sensitivity) {

}