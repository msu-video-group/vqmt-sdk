#pragma once

class IMetricValueSink {
public:
	virtual ~IMetricValueSink(){}
	virtual void onValue(int frame, const int* ids, const float* values, int length) = 0;
};