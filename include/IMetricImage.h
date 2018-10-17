/**
*  \file IMetricImage.h
*   \brief Interface to access metric image
*/

#pragma once

struct RangeSpecification {
	RangeSpecification() {}
	RangeSpecification(float min, float max, float realMin, float realMax)
		: min(min), max(max), realMin(realMin), realMax(realMax) {}

	RangeSpecification(float min, float max)
		: min(min), max(max), realMin(min), realMax(max) {}

	float min = 0;
	float max = 0;
	float realMin = 0;
	float realMax = 0;
};

/*!\brief Interface to image that is given by metric.
*
*    All color planes are stored without alignment.
*    Do not get pointer to color planes that differ from the one your metric was configured for, 
*    this may lead to unexpected results.
*/
class IMetricImage
{
public:
	enum ColorComponent { YYUV = 0, UYUV = 1, VYUV = 2, LLUV = 3, RRGB = 4, GRGB = 5, BRGB = 6, CC_LAST = 7 };

    /**
    **************************************************************************
    *  \brief Get R color plane from image in RGB
    */
	virtual const float* GetR() const  = 0;

    /**
    **************************************************************************
    * \brief Get G color plane from image in RGB
    */
	virtual const float* GetG() const = 0;

    /**
    **************************************************************************
    * \brief Get B color plane from image in RGB
    */
	virtual const float* GetB() const = 0;

    /**
    **************************************************************************
    * \brief Get Y color plane from image in YUV
    */
	virtual const float* GetY() const = 0;

    /**
    **************************************************************************
    * \brief Get U color plane from image in YUV or LUV
    */
	virtual const float* GetU() const = 0;

    /**
    **************************************************************************
    * \brief Get V color plane from image in YUV or LUV
    */
	virtual const float* GetV() const = 0;

    
    /**
    **************************************************************************
    * \brief Get L color plane from image in LUV
    */
	virtual const float* GetL() const = 0;

	const float* GetComponent(IMetricImage::ColorComponent component) const {
		switch (component) {
		case IMetricImage::YYUV: return GetY();
		case IMetricImage::UYUV: return GetU();
		case IMetricImage::VYUV: return GetV();
		case IMetricImage::LLUV: return GetL();
		case IMetricImage::RRGB: return GetR();
		case IMetricImage::GRGB: return GetG();
		case IMetricImage::BRGB: return GetB();
		}
		return nullptr;
	}

    /**
    **************************************************************************
    * \brief Get width of the stored image
    */
    virtual int GetWidth() const = 0;

    /**
    **************************************************************************
    * \brief Get height of the stored image
    */
	virtual int GetHeight() const = 0;

	virtual const RangeSpecification* GetRanges() const = 0;
};


class IMetricImage_api1000
{
public:

	/**
	**************************************************************************
	*  \brief Get R color plane from image in RGB
	*/
	virtual const float*        GetR() const = 0;

	/**
	**************************************************************************
	* \brief Get G color plane from image in RGB
	*/
	virtual const float*        GetG() const = 0;

	/**
	**************************************************************************
	* \brief Get B color plane from image in RGB
	*/
	virtual const float*        GetB() const = 0;

	/**
	**************************************************************************
	* \brief Get Y color plane from image in YUV
	*/
	virtual const float*        GetY() const = 0;

	/**
	**************************************************************************
	* \brief Get U color plane from image in YUV or LUV
	*/
	virtual const float*        GetU() const = 0;

	/**
	**************************************************************************
	* \brief Get V color plane from image in YUV or LUV
	*/
	virtual const float*        GetV() const = 0;


	/**
	**************************************************************************
	* \brief Get L color plane from image in LUV
	*/
	virtual const float*        GetL() const = 0;

	/**
	**************************************************************************
	* \brief Get width of the stored image
	*/
	virtual int            GetWidth() const = 0;

	/**
	**************************************************************************
	* \brief Get height of the stored image
	*/
	virtual int            GetHeight() const = 0;
};