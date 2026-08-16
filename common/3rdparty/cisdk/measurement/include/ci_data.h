#ifndef __CI_DATA_H__
#define __CI_DATA_H__
#include "ci_define.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef struct {
		int x;
		int y;
		int width;
		int height;
	} AlgoRect;//矩形


	typedef struct {
		int x;
		int y;
	} AlgoPoint;//点int

	typedef struct {
		int width;
		int height;
	} AlgoSize;//大小
	
	typedef struct {
		float width;
		float height;
	} AlgoSize2f;//大小
	
	typedef struct {
		float x;
		float y;
	} AlgoPoint2f;//点float

	typedef struct {
		float x;
		float y;
		float z;
	} AlgoPoint3f;//点float

	typedef struct {
		float x;
		float y;
		float width;
		float height;
	} AlgoRect2f;//矩形float

	typedef struct
	{
		float X;
		float Y;
		float Z;
	}TriXYZ;//色度分量XYZ

	typedef struct
	{
		float L;
		float a;
		float b;
	}TriLab;

	typedef struct
	{
		float L;
		float c;
		float h;
	}TriLch;

	typedef struct
	{
		float R;
		float G;
		float B;
	}TriRGB;//色度分量RGB

	typedef struct
	{
		float L;
		float x;
		float y;
	}TriLxy;//色度分量Lxy

	typedef struct
	{
		TriLxy R;
		TriLxy G;
		TriLxy B;
	}RGBLxy;//色度分量Lxy

	typedef struct
	{
		TriLxy R;
		TriLxy G;
		TriLxy B;
		TriLxy W;
	}RGBWLxy;

	typedef struct
	{
		TriXYZ R;
		TriXYZ G;
		TriXYZ B;
		TriXYZ W;
	}RGBWXYZ;

	typedef struct
	{
		TriXYZ R;
		TriXYZ G;
		TriXYZ B;
	}RGBXYZ;

	typedef struct
	{
		float R;
		float G;
		float B;
		float W;
	}RGBW;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // 
