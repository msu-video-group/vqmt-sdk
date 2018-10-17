# MSU Video Quality Measurement Tool SDK

---

- [About SDK](#about-sdk)
- [Building Sample Plugin](#building-sample-plugin)
- [Usage plugins](#usage-plugins)
- [Implementing own plugin](#implementing-own-plugin)

### About SDK
This is free SDK for [MSU VQMT](http://compression.ru/video/quality_measure/video_measurement_tool.html). Using this toolset you can create own plugin that
* can have 1 or 2 input videos (RGB or YUV float planes)
* produce arbitrary number of outputs (float number for each frame of input video(s))
* produce visualization (image for every frame)
* can be configurable

Each plugin creates own metric, that can be used in VQMT.

SDK can be used with FREE as well as in PRO VQMT edition.

### Building Sample Plugin
You can build Sample Plugin with CMake utility.

#### Visual Studio
1. Download & install [CMake](https://cmake.org/download/).
2. Open CMake GUI tool
3. Select `<VQMT SDK install path>/sample_plugin/build` as Source code directory, fill directory for the binaries
4. Press "Configure" and select Visual Studio with x64 platform
5. Press "Generate" and "Open Project"
6. Build generated solution

#### Linux
1. Install CMake
2. Goto folder, where you want to place binaries and type
	cmake <VQMT SDK install path>/sample_plugin/build
	make

### Usage plugins
#### Windows
Goto VQMT installation and place output `.vmp` file into folder `plugins`

#### Linux
Place output `.vmp` file into folder `~/.msu_vqmt/plugins`

### Implementing own plugin
#### Creating project
Create CMake project using Sample Plugin cmake as a template. If you don\'t want to use CMake, you should create static library with exports, described in section [Understanging SDK structure and exports](#understanging-sdk-structure-and-exports). That library should be saved with ``.vmp`` extension.

#### Implementation of basic plugin structure
As soon project is created, you should create a class for your plugin that inherits ``ICustomPlugin`` from ``ICustomPlugin.h``.

This class must implement the following member-functions:
```C++
	void Init(IMetricImage::ColorComponent cc, int width, int height, IMetricPlugin::ID start_id, IMetricValueSink* valueSink);
```
```C++
	std::vector< std::pair <IMetricPlugin::ID, float> >	Measure(std::vector<IMetricImage*> &images);
```
```C++
	std::vector< std::pair <IMetricPlugin::ID, float> >	MeasureAndVisualize(std::vector<IMetricImage*>&images, unsigned char *vis, int vis_pitch);
```
```C++
	std::vector<IDinfo>	MapIDToFrame(bool visualize);
```
```C++
	std::vector< std::pair <IMetricPlugin::ID, float> > CalculateAverage(bool visualize);
```
```C++
	int GetVideoNum(bool visualize);
```
```C++
	std::vector <IMetricImage::ColorComponent> GetSupportedColorcomponents();
```
```C++
	std::wstring GetName();
```
```C++
	std::wstring GetInterfaceName();
```
```C++
	std::wstring GetLongName();
```
```C++
	std::wstring GetUnit();
```
```C++
	std::wstring GetMetrInfoURL();
```
```C++
	bool GetMetrIncline();
```

And can implement the following functions:
```C++
	void Stop();
```
```C++
	const std::wstring& GetConfigJSON();
```
```C++
	bool SetConfigParams(const std::wstring& json);
```
```C++
	std::wstring GetConfigSummary();
```

The mean of each overriten member described in the following sections.

#### Initialization and info
```C++
	void Init(IMetricImage::ColorComponent cc, int width, int height, IMetricPlugin::ID start_id, IMetricValueSink* valueSink);
```
Color component, that is written to ``cc`` is one of values returned by prior ``GetSupportedColorcomponents()`` call.

This function will be called before measurement started. ``width`` and ``height`` are constant during all measurement.

Param ``start_id`` is using to index output values of plugin. If plugin should provide N float results for each frame, they should have following ids:
``start_id``, ``start_id``+1, ..., ``start_id``+N-1.

You can save ``valueSink`` if you need to provide results at any time. Not all plugins should use ``valueSink``.

```C++
	std::vector<IDinfo>	MapIDToFrame(bool visualize);
```

This function can give a name each result type. If you plugin provides N float results for each frame, the output should be an std::vector of N elements. You can distinguish the case of saving visualization and working without visualization save by ``visualize`` param.

```C++
	int GetVideoNum(bool visualize);
```

This function should return value 2 for reference metric and 1 for non-reference. Other values are not supported. You can distinguish the case of saving visualization and working without visualization save by ``visualize`` param.

```C++
	std::vector <IMetricImage::ColorComponent> GetSupportedColorcomponents();
```
You can allow user to run metric with only specified color components. Color components supported at this moment are: Y, U, V, R, G, B, L (for LUV). The exact color component from returned vector will be provided to plugin during initialization in ``Init`` function.

```C++
	std::wstring GetName();
```	
	
This function should return name that will be used in command line to select metric. It shouldn\'t start with ``-``, shouldn\'t match VQMT commandline keywords, also we recommend to make it whitespace-free and start from lowercase letter.

```C++
	std::wstring GetInterfaceName();
```
	
This function should return name that will be used in VQMT GUI. It can contain spaces and we recommend to start with uppercase letter.

```C++
	std::wstring GetLongName();
```
	
This value will be used in the description of metric. Provide full expanded name of the metric. 

```C++
	std::wstring GetUnit();
```
	
This function should return short name of units for result floats. Unit name will be used in plot of metric values and in VQMT output files.

```C++
	std::wstring GetMetrInfoURL();
```
	
This function should return URL of page with description of metric. It will be displayed in GUI after selecting metric and in commandline after getting metric list.

```C++
	bool GetMetrIncline();
```
	
Should return true is "bigger means better" for this metric.

#### Measurement and providing results

```C++
    std::vector< std::pair <IMetricPlugin::ID, float> Measure(std::vector<IMetricImage*> &images);
```

You should implement this function that will be called for each frame of VQMT input consequently in case of no visualization needed. Param ``images`` will contain same number of images as value, returned by ``GetVideoNum``. Read more about input images in section [Imput image format](#imput-image-format).

You should choose: to provide values by return values of this function or by using ``IMetricValueSink* valueSink`` transmited to ``Init`` function. Choose second way, if you want don\'t know metric value for frame N after processing frame N and preceeding frames. Using ``valueSink`` you can provide value for a frame after processing any amount of subsequent frames. In that case, ``Measure`` should return empty vector.

If you choose to tell results value by return value of ``Measure``, it should return same number of values as in ``MapIDToFrame`` return value.
``IMetricPlugin::ID`` should be consequent, the first one should be ``start_id`` that transmited to ``Init`` function.
```C++
	void Stop();
```
If you are using ``valueSink``, this is the last chance to use it for providing values. 
``Measure`` should not be called after stop, also, you shouldn\'t use ``valueSink``.
```C++
	std::vector< std::pair <IMetricPlugin::ID, float> > CalculateAverage(bool visualize);
```
Return average results in the same way as in ``Measure`` return value. You can\'t use ``valueSink`` for this task.

#### Visualization
```C++
	std::vector< std::pair <IMetricPlugin::ID, float> >	MeasureAndVisualize(std::vector<IMetricImage*>&images, unsigned char *vis, int vis_pitch);
```
This function should do same things as ``Measure``. Additionally, it should save visualization into ``vis`` argument. 
``vis`` param points to memory that contains RGB24 image of width and height provided to ``Init``. Use ``vis_pitch`` while filling this image.

#### Configuration
```C++
	const std::wstring& GetConfigJSON();
```
Metric can be configured. The configuration described by return value of this function. It should be JSON object, that contains paires of type ``"param-key": <param-description>``. The ``<param-description>`` is object with following fields:
* ``description``
* ``help``
* ``default_value`` -- this value will determine param type. It can be integer, string of floating point.
* ``possible_values`` (optional) - this value should be an array. If ``default_value`` is string, you can create enum param specifying this value.

If ``default_value`` is numeric, you can provide 2-element array as first argument, specifying possible value range.
```C++
	bool SetConfigParams(const std::wstring& json);
```
VQMT will send configuration provided by user via this call. It will contain JSON object with values of all parameters. You can parse JSON using library ``json.h`` by [YUVsoft](http://yuvsoft.com) included in SDK-pack.
```C++
	std::wstring GetConfigSummary();
```
This function, called after ``SetConfigParams`` can return short description of applied configuration.

#### Input image format
``IMetricImage`` declared in ``IMetricImage.h`` is interface that describes an image transmited to ``Measure`` and ``MeasureAndVisualize`` functions. It provides several image planes, only component specified in ``cc`` param of ``Init`` is guaranteed to be filled.

You can get width and height by calling ``int GetWidth() const`` and ``int GetHeight() const``. Note, image in ``IMetricImage`` can have width and height greater than values, provided to ``Init`` call. You should be guided by ``Init`` values and discard parts of the image that go beyond these boundaries.

You can get plane of image calling ``const float* GetX() const`` where ``X`` is one of ``R``, ``G``, ``B``, ``Y``, ``U``, ``V``, ``L``. This call can return ``nullptr`` if plane is not present in image. Each row of plane consists of ``GetWidth()`` floats, rows are not aligned are located one after another.

```C++
	const RangeSpecification* GetRanges() const
```
This function will return array of ranges for each component in order as in IMetricImage::ColorComponent enum.

#### Implementation of exports
See ``vqmt_sample_plugin.cpp`` to know, what functions you should export. You can use this file unchanged, only replaced ``VQMTsamplePlugin`` with name of your own ``ICustomPlugin`` implementation.

#### Understanging SDK structure and exports
Each plugin is shared library that should export following functions:
```C++
	void CreateMetric ( IMetricPlugin** metric );
```
```C++
	void ReleaseMetric ( IMetricPlugin* metric );
```
```C++
	int GetVQMTVersion();
```
```C++
	int CompatibleWithVQMT(int vqmtVer);
```

``CreateMetric`` should create new instance of abstract class IMetricPlugin. IMetricPlugin has implementation-independent methods that are inconvenient to use, so we recommend to use ``ICustomPlugin``, that implements exactly the same functions as ``CreateMetric`` but in more usable way. SDK implements wrapper ``CPluginAdapter`` that can convert ``ICustomPlugin`` to ``IMetricPlugin``.

``Release metric`` destroys created instance.

``GetVQMTVersion`` and ``CompatibleWithVQMT`` can be used to determine whether a particular version of VQMT is compatible with the plugin. The general rule is: VQMT should know plugin SDK, i. e. newer VQMT can use older plugin, but not vice versa.