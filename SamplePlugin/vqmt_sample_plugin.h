/*
********************************************************************
(c) MSU Video Group, http://compression.ru/video/
This source code is property of MSU Graphics and Media Lab

This code may be distributed under LGPL
(see http://www.gnu.org/licenses/lgpl.html for more details).

E-mail: video-measure@compression.ru
Author: Alexey Noskov
********************************************************************
*/  

#pragma once

#include "../PluginBase/ICustomPlugin.h"
#include "../PluginBase/json.h"

/*
*	BIPSNR plugin
*	Counts PSNR independently to brightness
*/
class VQMTsamplePlugin : public ICustomPlugin
{
public:
	void Init(IMetricImage::ColorComponent colorComp, int width, int height, int start_id, IMetricValueSink* sink) override {
		this->width = width;
		this->height = height;
		this->output_id_1 = start_id;
		this->output_id_2 = start_id + 1;
		this->colorComp = colorComp;
		this->sink = sink;
	}

	std::vector< std::pair <IMetricPlugin::ID, float> >	Measure(std::vector<IMetricImage*> &images) override {
		const float* p1 = images[0]->GetComponent(colorComp);
		const float* p2 = images[1]->GetComponent(colorComp);

		int step1 = images[0]->GetWidth();
		int step2 = images[1]->GetWidth();

		int height1 = images[0]->GetHeight();
		int height2 = images[1]->GetHeight();

		//take difference between pixel 100x100 on each image
		int pixelX = std::min(100, width);
		int pixelY = std::min(100, height);
		float diff1 = p2[pixelX + step2 * pixelY] - p1[pixelX + step1 * pixelY];
		float diff2 = diff1*diff1;

		sum1 += diff1;
		sum2 += diff2;

		//we will output 1-st value bu return value
		//and the second value by sink:
		if (sink) {
			int ids[] = { output_id_2 };
			float values[] = { diff2 };
			sink->onValue(currentFrame++, ids, values, sizeof(ids) / sizeof(*ids));
		}

		return { { output_id_1, diff1 } };
	}
	std::vector< std::pair <IMetricPlugin::ID, float> >	MeasureAndVisualize(std::vector<IMetricImage*>&images, unsigned char *vis, int vis_pitch) override {
		auto res = Measure(images);

		// fill visualization channels with solid color dependent on parameters:
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				vis[y*vis_pitch + x * 3 + 0] = param;
				vis[y*vis_pitch + x * 3 + 1] = param3;
				vis[y*vis_pitch + x * 3 + 2] = 0;
			}
		}

		return res;
	}

	std::vector<IDinfo>	MapIDToFrame(bool visualize) override {
		return { 
			{ output_id_1, L"1-st custom val" }, 
			{ output_id_2 , L"2-nd custom val" } 
		};
	}

	std::vector< std::pair <IMetricPlugin::ID, float> > CalculateAverage(bool visualize) override {
		return {
			{ output_id_1, float(sum1/ currentFrame) },
			{ output_id_2 , float(sum2 / currentFrame) }
		};
	}

	int GetVideoNum(bool) override {
		return 2;
	}

	std::vector <IMetricImage::ColorComponent> GetSupportedColorcomponents() override {
		return { IMetricImage::RRGB, IMetricImage::GRGB, IMetricImage::BRGB };
	}

	std::wstring GetName() override {
		return L"sample";
	}
	std::wstring GetInterfaceName() override {
		return L"Sample Plugin";
	}
	std::wstring GetLongName() override {
		return L"VQMT Sample Plugin";
	}

	std::wstring GetMetrInfoURL() override {
		return L"http://example.com";
	}
	std::wstring GetUnit() override {
		return L"pt";
	}

	bool GetMetrIncline() override {
		return true;
	}

	const std::wstring& GetConfigJSON() override {
		static const std::wstring obj =
		{
			L"	{																		"
			L"		\"param\": {														"
			L"			\"description\": \"Integer Param 0-100\",						"
			L"			\"help\": \"Sample param called \\\"param\\\". Range: 0-100\",	"
			L"			\"default_value\": 10,											"
			L"			\"possible_values\": [[0,100]]									"
			L"		},																	"
			L"		\"param2\": {														"
			L"			\"description\": \"Enum Param\",								"
			L"			\"help\": \"Sample param called \\\"param1\\\"\",				"
			L"			\"default_value\": \"val1\",									"
			L"			\"possible_values\": [\"val1\", \"val2\", \"val3\"]				"
			L"		},																	"
			L"		\"param3\": {														"
			L"			\"description\": \"Floating Param\",							"
			L"			\"help\": \"Sample param called \\\"param3\\\"\",				"
			L"			\"default_value\": 100.0										"
			L"		},																	"
			L"		\"param4\": {														"
			L"			\"description\": \"String param\",								"
			L"			\"help\": \"Sample param called \\\"param4\\\"\",				"
			L"			\"default_value\": \"placeholder\"								"
			L"		}																	"
			L"	}																		"
		};
		return obj;
	}

	bool SetConfigParams(const std::wstring& json)  override { 
		try {
			YUVsoft::JSON res = YUVsoft::ParseWrapper::parse(YUVsoft::utf16_to_utf8(json));
			if (res.in("param")) {
				this->param = res["param"].asInteger();
			}
			if (res.in("param2")) {
				this->param2 = res["param2"].asString();
			}
			if (res.in("param3")) {
				this->param3 = res["param3"].asFloat();
			}
			if (res.in("param4")) {
				this->param4 = res["param4"].asString();
			}

			return true;
		}
		catch (...) {}

		return false;
	}

	std::wstring GetConfigSummary() override { 
		std::wstringstream out;
		out << "Configured to: " << param << " " << param2.c_str() << " " << param3 << " " << param4.c_str();
		return out.str();
	}

private:
	int param = 10;
	std::string param2;
	float param3 = 100.;
	std::string param4;

	int currentFrame = 0;

	int width;
	int height;

	int output_id_1;
	int output_id_2;

	double sum1 = 0;
	double sum2 = 0;

	IMetricValueSink* sink;

	IMetricImage::ColorComponent colorComp;
};