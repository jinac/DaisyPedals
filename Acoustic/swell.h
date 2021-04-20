#pragma once
#ifndef SWELL_H
#define SWELL_H

#include <stdint.h>
#ifdef __cplusplus

#include <math.h>

namespace daisysp {

class Swell {
  public:
	  Swell() {}
	  ~Swell() {}

	  void Init(float attack, float sensitivity);

	  float Process(float in);

    void UpdateParams(float attack, float sensitivity);

  private:
    float attack_, sensitivity_, send_value_;
    bool send_flag_;

}

}