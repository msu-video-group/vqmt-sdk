#pragma once

#include "IMetricImage.h"
#include "IMetricValueSink.h"

#include <string>

/*
*	C-style interface for metric plugin. It is easier to use ICustomPlugin in conjunction with
*	CPluginAdapter
*/
class IMetricPlugin
{
public:
	static const int apiLevel = 2000;

	typedef int ID;

	virtual ~IMetricPlugin() {};


	/*
	*	Use this function to initialize metric parameters.
	*	It is always called before Measure(), MeasureAndVisualize(), GetAverage(), ResetMetricStat(),
	*	but other functions may be called earlier than Init().
	*	If metric supports configuration string, then Config is called after Init().
	*	global_id represents current available global_id for metric.
	*	If metric uses N id, it can increment global_it by N, reserving
	*	[global_id ... (global_id + (N-1))] for its' use. This reserved IDs will identify
	*	results of metric in Measure(), MeasureAndVisualize(), MapIDToFrame() and so on.
	*/
	virtual void Init(IMetricImage::ColorComponent cc, int width, int height, int start_id, IMetricValueSink* metricValueSink) = 0;

	/*
	*	Measures metric on images.
	*
	* \param images			[IN] - array of images that metric gets as input
	* \param images_num		[IN  - amount of images in the array
	* \param ids			[IN, OUT]  - pointer to buffer for IDs of results by this metric
	* \param res			[IN, OUT]  - pointer to buffer for results of metric. Each ID in ids correspond to one of results here.
	* \param res_num		[IN, OUT]  - amount of ids/results
	*/
	virtual void Measure(IMetricImage **images, int images_num, ID *ids, float *res, int &res_num) = 0;

	/*
	*	Measures metric on images and saves visualization.
	*
	* \param images			[IN] - array of images that metric gets as input
	* \param images_num		[IN  - amount of images in the array
	* \param ids			[IN, OUT]  - buffer for IDs of results by this metric
	* \param res			[IN, OUT]  - buffer for results of metric. Each ID in ids correspond to one of results here.
	* \param res_num		[IN, OUT]  - amount of ids/results
	* \param visualization	[IN, OUT]  - buffer for visualization. Must be saved the same as DIB, pitch width is word-aligned.
	*/
	virtual void MeasureAndVisualize(IMetricImage **images, int images_num, ID *ids, float *res, int &res_num, unsigned char *visualization, int visualization_pitch) = 0;
	virtual void Stop() = 0;

	/*
	*	Tells which ID correspond to which frame, as given to Measure() and MeasureAndVisualize(),
	*	optionally can give name to an ID (default is empty name, L"")
	*	For instance, if it is metric with one ID without name and two frames, it should return ID, (0,1)
	*	and if it is with two IDs without names for two frames it should return ID1, (0), ID2, (1)
	*
	* \param ids_num		[IN, OUT]  - amount of IDs of this metric. IN value - capacity of arrays
	* \param ids			[IN, OUT]  - buffer for array of all IDs
	* \param frames			[IN, OUT]  - buffer for pointers of information on each id;
	Plugin is ought to fill ids_num records; for each ID record is stored as follows:
	frames[id_number][0] - amount of frames used by the id
	frames[id_number][1..frames[id][0]] - numbers of frames that correspond to id
	* \param names - pointer to array of string that describe IDs.
	names[id_number] - name for id with number id_number. If you do need name for id, put L"" here.
	* \param visualize		[IN, OUT]  - if true, than we are interested in ths info for MeasureandVisualize;
	*/
	virtual void MapIDToFrame(int &ids_num, ID * ids, wchar_t **names, int namesCap, bool visualize) = 0;

	/*
	*	Returns average result.
	*
	* \param ids			[IN, OUT]  - buffer for IDs of results by this metric
	* \param res			[IN, OUT]  - buffer for average results of metric. Each ID in ids correspond to one of results here.
	* \param res_num		[IN, OUT]  - amount of ids/results
	* \param visualize		[IN, OUT]  - if true, than we are interested in average results after processing with visualization;
	*/
	virtual void CalculateAverage(ID *ids, float *res, int &res_num, bool visualize) = 0;

	/*
	*	returns amount of video required for measurement (possibly with visualization)
	* \param visualize		[IN]	- if true, return amount of frames required for visualization.
	*/
	virtual int	GetVideoNum(bool visualize) = 0;

	/*
	*	Must fill the given buffer with supported color components.
	*	\param cc			[IN, OUT]	- buffer for storing supported color components
	*	\param cc_num		[OUT]		- amount of supported color component
	*/
	virtual void GetSupportedColorcomponents(IMetricImage::ColorComponent *cc, int &cc_num) = 0;

	/*
	*	Metric is asked for its' name that will be used as command line parameter for CLI and written in name of resulting CSV file.
	*	Usually in small letters without spaces, like "psnr" of "blurring_measure"
	*/
	virtual void				GetName(wchar_t *name, int buffLen) = 0;

	/*
	*	Returns units of metrics, for instance L"dB" or empty string L""
	*/
	virtual void				GetUnit(wchar_t *unit, int buffLen) = 0;

	/*
	*	Obsolete!
	*/
	virtual int					GetWidthMultiply() = 0;

	/*
	*	Obsolete!
	*/
	virtual int					GetHeightMultiply() = 0;

	/*
	*	Metric is asked for its' name that will be used in interface.
	*	Usually in big letters like "PSNR" of "Blurring Measure"
	*/
	virtual void				GetInterfaceName(wchar_t *interface_name, int buffLen) = 0;
	/*
	*	Metric is asked for its' name that will be used in interface.
	*	Usually in big letters like "PSNR" of "Blurring Measure"
	*/
	virtual void				GetLongName(wchar_t *long_name, int buffLen) = 0;

	/*
	*	Path to web page with info about metric
	*/
	virtual void				GetMetrInfoURL(wchar_t *info_url, int buffLen) = 0;

	/*
	*	If true, then metric is the bigger the better.
	*	If false, then the smaller the better.
	*/
	virtual bool				GetMetrIncline() = 0;

	virtual void GetConfigJSON(const wchar_t** bufPlace, int* bufLen) = 0;
	virtual bool SetConfigParams(const wchar_t* json, int jsonLen) = 0;
	virtual int GetConfigSummary(wchar_t* outBuff, int outBuffCapacity) = 0;

	/**
	*	Metric must return version of interface that it supports
	*/
	virtual int					GetVersion() = 0;
};

/*
*	C-style interface for metric plugin. It is easier to use ICustomPlugin in conjunction with
*	CPluginAdapter
*/
class IMetricPlugin_api1000
{
public:
	enum COLORCOMPONENT	{YYUV = 0, UYUV = 1, VYUV = 2, LLUV = 3, RRGB = 4, GRGB = 5, BRGB = 6};

	typedef int ID;

	virtual ~IMetricPlugin_api1000() {};


	/*
	*	Use this function to initialize metric parameters.
	*	It is always called before Measure(), MeasureAndVisualize(), GetAverage(), ResetMetricStat(),
	*	but other functions may be called earlier than Init().
	*	If metric supports configuration string, then Config is called after Init().
	*	global_id represents current available global_id for metric.
	*	If metric uses N id, it can increment global_it by N, reserving
	*	[global_id ... (global_id + (N-1))] for its' use. This reserved IDs will identify
	*	results of metric in Measure(), MeasureAndVisualize(), MapIDToFrame() and so on.
	*/
	virtual void Init(COLORCOMPONENT cc, int width, int height, int &global_id) = 0;

	/*
	*	Measures metric on images.
	*
	* \param images			[IN] - array of images that metric gets as input
	* \param images_num		[IN  - amount of images in the array
	* \param ids			[IN, OUT]  - pointer to buffer for IDs of results by this metric
	* \param res			[IN, OUT]  - pointer to buffer for results of metric. Each ID in ids correspond to one of results here.
	* \param res_num		[IN, OUT]  - amount of ids/results
	*/
	virtual void Measure(IMetricImage_api1000 **images, int images_num, ID *ids, float *res, int &res_num) = 0;

	/*
	*	Measures metric on images and saves visualization.
	*
	* \param images			[IN] - array of images that metric gets as input
	* \param images_num		[IN  - amount of images in the array
	* \param ids			[IN, OUT]  - buffer for IDs of results by this metric
	* \param res			[IN, OUT]  - buffer for results of metric. Each ID in ids correspond to one of results here.
	* \param res_num		[IN, OUT]  - amount of ids/results
	* \param visualization	[IN, OUT]  - buffer for visualization. Must be saved the same as DIB, pitch width is word-aligned.
	*/
	virtual void MeasureAndVisualize(IMetricImage_api1000 **images, int images_num, ID *ids, float *res, int &res_num, unsigned char *visualization, int visualization_pitch) = 0;

	/*
	*	Tells which ID correspond to which frame, as given to Measure() and MeasureAndVisualize(),
	*	optionally can give name to an ID (default is empty name, L"")
	*	For instance, if it is metric with one ID without name and two frames, it should return ID, (0,1)
	*	and if it is with two IDs without names for two frames it should return ID1, (0), ID2, (1)
	*
	* \param ids_num		[IN, OUT]  - amount of IDs of this metric. IN value - capacity of arrays
	* \param ids			[IN, OUT]  - buffer for array of all IDs
	* \param frames			[IN, OUT]  - buffer for pointers of information on each id;
		Plugin is ought to fill ids_num records; for each ID record is stored as follows:
		frames[id_number][0] - amount of frames used by the id
		frames[id_number][1..frames[id][0]] - numbers of frames that correspond to id
	* \param names - pointer to array of string that describe IDs.
		names[id_number] - name for id with number id_number. If you do need name for id, put L"" here.
	* \param visualize		[IN, OUT]  - if true, than we are interested in ths info for MeasureandVisualize;
	*/
	virtual void MapIDToFrame(int &ids_num, ID * ids, int **frames, int framesCap, wchar_t **names, int namesCap, bool visualize) = 0;

	/*
	*	Returns average result.
	*
	* \param ids			[IN, OUT]  - buffer for IDs of results by this metric
	* \param res			[IN, OUT]  - buffer for average results of metric. Each ID in ids correspond to one of results here.
	* \param res_num		[IN, OUT]  - amount of ids/results
	* \param visualize		[IN, OUT]  - if true, than we are interested in average results after processing with visualization;
	*/
	virtual void CalculateAverage(ID *ids, float *res, int &res_num, bool visualize) = 0;

	/*
	*	returns amount of video required for measurement (possibly with visualization)
	* \param visualize		[IN]	- if true, return amount of frames required for visualization.
	*/
	virtual int	GetVideoNum(bool visualize) = 0;

	/*
	*	Must fill the given buffer with supported color components.
	*	\param cc			[IN, OUT]	- buffer for storing supported color components
	*	\param cc_num		[OUT]		- amount of supported color component
	*/
	virtual void GetSupportedColorcomponents(COLORCOMPONENT *cc, int &cc_num) = 0;

	/*
	*	Metric is asked for its' name that will be used as command line parameter for CLI and written in name of resulting CSV file.
	*	Usually in small letters without spaces, like "psnr" of "blurring_measure"
	*/
	virtual void				GetName(wchar_t *name, int buffLen) = 0;

	/*
	*	Returns units of metrics, for instance L"dB" or empty string L""
	*/
	virtual void				GetUnit(wchar_t *unit, int buffLen) = 0;

	/*
	*	Returns width multiply
	*/
	virtual int					GetWidthMultiply() = 0;

	/*
	*	Returns height multiply
	*/
	virtual int					GetHeightMultiply() = 0;

	/*
	*	Metric is asked for its' name that will be used in interface.
	*	Usually in big letters like "PSNR" of "Blurring Measure"
	*/
	virtual void				GetInterfaceName(wchar_t *interface_name, int buffLen) = 0;
	/*
	*	Metric is asked for its' name that will be used in interface.
	*	Usually in big letters like "PSNR" of "Blurring Measure"
	*/
	virtual void				GetLongName(wchar_t *long_name, int buffLen) = 0;

	/*
	*	Path to web page with info about metric
	*/
	virtual void				GetMetrInfoURL(wchar_t *info_url, int buffLen) = 0;

	/*
	*	If true, then metric is the bigger the better.
	*	If false, then the smaller the better.
	*/
	virtual bool				GetMetrIncline() = 0;		

	virtual void GetConfigJSON(const wchar_t** bufPlace, int* bufLen) = 0;
	virtual bool SetConfigParams(const wchar_t* json, int jsonLen) = 0;
	virtual int GetConfigSummary(wchar_t* outBuff, int outBuffCapacity) = 0;

	/**
	*	Metric must return version of interface that it supports
	*/
	virtual int					GetVersion() = 0;
};


/*
*	C-style interface for metric plugin. It is easier to use ICustomPlugin in conjunction with
*	CPluginAdapter
*/
class IMetricPlugin_api320
{
public:
	enum COLORCOMPONENT	{ YYUV = 0, UYUV = 1, VYUV = 2, LLUV = 3, RRGB = 4, GRGB = 5, BRGB = 6 };

	typedef int ID;

	virtual ~IMetricPlugin_api320() {};
	virtual void Init(COLORCOMPONENT cc, int width, int height, int &global_id) = 0;
	virtual void Measure(IMetricImage_api1000 **images, int images_num, ID *ids, float *res, int &res_num) = 0;
	virtual void MeasureAndVisualize(IMetricImage_api1000 **images, int images_num, ID *ids, float *res, int &res_num, unsigned char *visualization, int visualization_pitch) = 0;
	virtual void MapIDToFrame(int &ids_num, ID * ids, int **frames, wchar_t **names, bool visualize) = 0;
	virtual void CalculateAverage(ID *ids, float *res, int &res_num, bool visualize) = 0;
	virtual int	GetVideoNum(bool visualize) = 0;
	virtual void GetSupportedColorcomponents(COLORCOMPONENT *cc, int &cc_num) = 0;
	virtual void				GetName(wchar_t *name) = 0;
	virtual void				GetUnit(wchar_t *unit) = 0;
	virtual int					GetWidthMultiply() = 0;
	virtual int					GetHeightMultiply() = 0;
	virtual void				GetInterfaceName(wchar_t *interface_name) = 0;
	virtual void				GetMetrInfoURL(wchar_t *info_url) = 0;
	virtual bool				GetMetrIncline() = 0;
	virtual bool				IsConfigurable() = 0;
	virtual bool				Config(const wchar_t* configuration_line) = 0;
	virtual bool				GetDefaultConfiguration(wchar_t* buf, int buf_size) = 0;

	virtual int					GetVersion() = 0;
};
