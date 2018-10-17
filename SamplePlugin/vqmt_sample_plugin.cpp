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

/*
* bi_psnr.cpp: implementation of VMT plugin for measurement of brigyhness-independent PSNR.
*/

#include "../PluginBase/PluginAdapter.h"
#include "vqmt_sample_plugin.h"

#include <cstring>
#include <algorithm>

/*
* DllMain
*/

VQMT_EXPORT void CreateMetric ( IMetricPlugin**metric )
{
	*metric = new CPluginAdapter ( std::make_unique<VQMTsamplePlugin>() );
}

VQMT_EXPORT void ReleaseMetric ( IMetricPlugin* metric )
{
	delete metric;
}

VQMT_EXPORT int GetVQMTVersion()
{
	return CPluginAdapter::apiLevel;
}

VQMT_EXPORT int CompatibleWithVQMT(int vqmtVer)
{
	return vqmtVer >= CPluginAdapter::apiLevel ? 0 : -1;
}
