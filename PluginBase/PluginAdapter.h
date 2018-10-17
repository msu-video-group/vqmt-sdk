/*
********************************************************************
(c) MSU Video Group, http://compression.ru/video/
This source code is property of MSU Graphics and Media Lab

This code may be distributed under LGPL
(see http://www.gnu.org/licenses/lgpl.html for more details).

E-mail: video-measure@compression.ru
Author: Oleg Petrov
********************************************************************
*/  

#pragma once

#include "ICustomPlugin.h"

#include <cstring>
#include <algorithm>
#include <memory>

#if !defined(VQMT_EXPORT)
#ifdef _MSC_VER
#define VQMT_EXPORT extern "C" __declspec(dllexport)
#else
#define VQMT_EXPORT extern "C" __attribute__ ((visibility ("default")))
#endif
#endif

/*
*	Adapts your plugin that is derived from ICustomPlugin to IMetricPlugin
*/
class CPluginAdapter : public IMetricPlugin {
	static int copyStr(wchar_t* dst, int buffCap, const std::wstring& src) {
		int copyLen = (int)std::min((int)src.size(), buffCap - 1);
		memcpy(dst, src.c_str(), sizeof(wchar_t) * copyLen);
		dst[copyLen] = 0;

		return copyLen;
	}

public:
	CPluginAdapter(std::unique_ptr<ICustomPlugin> plugin) :
		m_plugin(std::move(plugin))
	{
	}

	void Init(IMetricImage::ColorComponent cc, int width, int height, int start_id, IMetricValueSink* metricValueSink) override {
		m_plugin->Init(cc, width, height, start_id, metricValueSink);
	}

	void Measure(IMetricImage **images, int images_num, ID *ids, float *res, int &res_num) override {
		std::vector<IMetricImage*> images_v(images, images + images_num);
		std::vector< std::pair <IMetricPlugin::ID, float> > res_v = m_plugin->Measure(images_v);

		res_num = (int)res_v.size();
		for (int i = 0; i < res_num; i++)
		{
			ids[i] = res_v[i].first;
			res[i] = res_v[i].second;
		}
	}

	void MeasureAndVisualize(IMetricImage **images, int images_num, ID *ids, float *res, int &res_num, unsigned char *visualization, int visualization_pitch) override {
		std::vector<IMetricImage*> images_v(images, images + images_num);
		std::vector< std::pair <IMetricPlugin::ID, float> > res_v = m_plugin->MeasureAndVisualize(images_v, visualization, visualization_pitch);

		res_num = (int)res_v.size();
		for (int i = 0; i < res_num; i++)
		{
			ids[i] = res_v[i].first;
			res[i] = res_v[i].second;
		}
	}

	void Stop() override {
		m_plugin->Stop();
	}

	void MapIDToFrame(int &ids_num, ID * ids, wchar_t **names, int namesCap, bool visualize) override {
		std::vector< IDinfo > res = m_plugin->MapIDToFrame(visualize);
		if (ids_num == 0) {
			ids_num = (int)res.size();
			return;
		}

		ids_num = std::min((int)res.size(), ids_num);
		for (int i = 0; i < ids_num; i++)
		{
			ids[i] = res[i].id;
			copyStr(names[i], namesCap, res[i].name);
		}
	}

	void CalculateAverage(ID *ids, float *res, int &res_num, bool visualize) override {
		std::vector< std::pair <IMetricPlugin::ID, float> > res_v = m_plugin->CalculateAverage(visualize);

		res_num = (int)res_v.size();
		for (int i = 0; i < res_num; i++)
		{
			ids[i] = res_v[i].first;
			res[i] = res_v[i].second;
		}
	}

	int GetVideoNum(bool visualize) override {
		return m_plugin->GetVideoNum(visualize);
	}

	void GetSupportedColorcomponents(IMetricImage::ColorComponent *cc, int &cc_num) override {
		std::vector< IMetricImage::ColorComponent > res = m_plugin->GetSupportedColorcomponents();
		cc_num = (int)res.size();
		for (int i = 0; i < cc_num; i++)
			cc[i] = res[i];
	}

	void GetName(wchar_t *name, int cap) override {
		std::wstring data = m_plugin->GetName();
		copyStr(name, cap, data);
	}

	void GetUnit(wchar_t *unit, int cap) override {
		std::wstring data = m_plugin->GetUnit();
		copyStr(unit, cap, data);
	}

	void GetInterfaceName(wchar_t *interface_name, int cap) override {
		std::wstring data = m_plugin->GetInterfaceName();
		copyStr(interface_name, cap, data);
	}

	void GetLongName(wchar_t *name, int cap) override {
		std::wstring data = m_plugin->GetLongName();
		copyStr(name, cap, data);
	}

	void GetMetrInfoURL(wchar_t *info_url, int cap) override {
		std::wstring data = m_plugin->GetMetrInfoURL();
		copyStr(info_url, cap, data);
	}

	bool GetMetrIncline() override {
		return m_plugin->GetMetrIncline();
	}

	void GetConfigJSON(const wchar_t** bufPlace, int* bufLen) override {
		const std::wstring& data = m_plugin->GetConfigJSON();
		*bufPlace = data.c_str();
		*bufLen = (int)data.size();
	}

	bool SetConfigParams(const wchar_t* json, int jsonLen) override {
		std::wstring str(json, jsonLen);
		return m_plugin->SetConfigParams(str);
	}

	int GetConfigSummary(wchar_t* outBuff, int buffCap) override {
		//std::wstring str(json, jsonLen);
		std::wstring data = m_plugin->GetConfigSummary();
		return copyStr(outBuff, buffCap, data);
	}

	int GetVersion() override {
		return 100;
	}

	int GetWidthMultiply() override {
		return -1;
	}
	int GetHeightMultiply() override {
		return -1;
	}

private:
	std::unique_ptr<ICustomPlugin> m_plugin;
};
