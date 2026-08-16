#ifndef __CURVE_FITTING_H__
#define __CURVE_FITTING_H__

#include "ci_define.h"
#include "ci_data.h"
#include "ci_declare.h"
#include "curve_fitting_data.h"
#include "curve_fitting_error.h"
#include "algo_mat_data.h"
#include "meta_info_enum.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


    /************************************
    @brief  : 创建curvefit句柄
    @note   : 
    @param  : out_curvefit_handle   -- curvefit句柄
    @param  : in_method             -- 方法,目前支持5=23阶曲面拟合,
                                                  6=block亮度拉齐＋23阶曲面拟合
                                                  7=高斯滤波曲面
    @return	: AlgoError
    @Author : chengjiayi@2021/06/24
    ************************************/
    C_API AlgoError C_CALL CurveFitCreateHandle(AlgoHandle* out_curvefit_handle, int in_method=5);


	/************************************
	@brief  : 计算白色曲面系数===》利用RGB配比，通过RGB曲面系数合成白色曲面系数
	@note   :
	@param  : curvefit_handle                 有效的curvefit句柄
	@param  : in_rgb_curvef_handle             输入的RGB的曲面系数，必须同时输入"RGB的X"或"RGB的Y"或"RGB的Z"(RZ可以为空，但不支持RZ为nullptr)，则输出的为W对应滤片的曲面系数，
											   输入顺序必须依次为RGB
	@param  : in_rgb_curvef_num                in_rgb_curvef_handle的长度，长度为3
	@param  : in_rgbw_xyz                     对应RGBW的XYZ，用于计算RGB配比，在合成w_curve时需要
	@param  : in_type                         输入的滤片信息，RGB的曲面必须是该滤片的曲面，计算出来的白色曲面也是该滤片的曲面
	@param  : out_w_curvef_handle             输出的w的曲面系数
	@return	: AlgoError
	@Author : zhangxuan@2024/07/04
	************************************/
	C_API AlgoError C_CALL CurveFitComputeWhiteCurveByRGB(AlgoHandle curvefit_handle, AlgoHandle* in_rgb_curvef_handle, int in_rgb_curvef_num, RGBWXYZ in_rgbw_xyz, CIEType in_type, AlgoHandle out_w_curvef_handle);

    /************************************
    @brief  : 设置曲面拟合的方式
    @note   :
    @param  : curvefit_handle     -- curvefit句柄
    @param  : in_method           -- 5=23阶曲面拟合, 6=block亮度拉齐＋23阶曲面拟合，6目前低灰校正使用
    @return	: AlgoError
    @Author : ouxingtao@2021/12/07
    ************************************/
    C_API AlgoError C_CALL CurveFitSetMethod(AlgoHandle curvefit_handle, const int in_method);


    /************************************
    @brief  : 设置灯板数量
    @note   : 拉齐拟合曲面使用
    @param  : curvefit_handle      -- curvefit句柄
    @param  : block_num_row        -- 灯板行数
    @param  : block_num_col        -- 灯板列数
    @return	: AlgoError
    @Author : ouxingtao@2021/12/07
    ************************************/
    C_API AlgoError C_CALL CurveFitSetBlockNum(AlgoHandle curvefit_handle, const int block_num_row, const int block_num_col);

	/************************************
   @brief  : 拼接屏块拉齐高斯滤波去曲面
   @note   : 适用于2k拼接屏
   @param  : in_meter_handle   有效的meter_handle句柄 详见algo_led_meter.h
			 rad_w、rad_h、sigma_w、sigma_h设置高斯滤波核
			 screen_size_w,screen_size_h单个屏宽高, int block_w, int block_h拼接屏横纵数量
   @return	: AlgoError
   @Author : qianxiaotong@2023/06/21
   ************************************/
	C_API AlgoError C_CALL CurveFitGaussianBlurSplic(AlgoHandle in_xyz_meter, const int in_rad_w, const int in_rad_h, const double in_sigma_w, const double in_sigma_h, const int in_screen_size_w, const int in_screen_size_h, const int in_block_w, const int in_block_h, AlgoHandle xyz_meter_out);


    /************************************
    @brief  : 设置灯板归一化位置及宽度
    @note   : 拉齐拟合曲面使用
    @param  : curvefit_handle      -- curvefit句柄
    @param  : refer_loc            -- 归一化位置，0代表第一行、最后一行、第一列、最后一列
    @param  : refer_width          -- 用来归一化的宽度
    @return	: AlgoError
    @Author : ouxingtao@2021/12/07
    ************************************/
    C_API AlgoError C_CALL CurveFitSetReferLocWidth(AlgoHandle curvefit_handle, const int refer_loc, const int refer_width);


	/************************************
	@brief  : 设置滤波曲面中中值滤波参数
	@note   : 滤波曲面使用
	@param  : curvefit_handle       -- curvefit句柄
	@param  : in_medianfilter_para  -- 中值滤波参数
	@return	: AlgoError
	@Author : liying@2023/10/11
	************************************/
	C_API AlgoError C_CALL CurveFitSetMedianFilterPara(AlgoHandle curvefit_handle, const int in_medianfilter_para);


    /************************************
    @brief  : 设置滤波曲面参数
    @note   : 滤波曲面使用
    @param  : curvefit_handle      -- curvefit句柄
    @param  : in_filter_width      -- 高斯滤波器宽度
    @param  : in_filter_height     -- 高斯滤波器高度
    @param  : in_sigma_x           -- 水平方向标准差
    @param  : in_sigma_y           -- 垂直方向标准差
    @return	: AlgoError
    @Author : ouxingtao@2022/7/5
    ************************************/
    C_API AlgoError C_CALL CurveFitSetFilterPara(AlgoHandle curvefit_handle, const int in_filter_width, const int in_filter_height, const float in_sigma_x, const float in_sigma_y);


    /************************************
    @brief  : 拟合曲面
    @note   : 
    @param  : curvefit_handle   有效的拟合曲面修正句柄
    @param  : in_meter_handle   有效的meter_handle句柄 详见algo_led_meter.h
    @return	: AlgoError
    @Author : chengjiayi@2021/06/24
    ************************************/
    C_API AlgoError C_CALL CurveFitCompute(AlgoHandle curvefit_handle, AlgoHandle in_meter_handle);


    /************************************
    @brief  : 箱体迭代校正时对所有箱体生成公共曲面系数修正
    @note   :
    @param  : curvefit_handle           有效的拟合曲面修正句柄
    @param  : curvefit_handle_real      有效的curvefit_handle句柄，表示所有箱体原始数据生成曲面系数
    @param  : curvefit_handle_initial   有效的curvefit_handle句柄，表示预先标定的几个箱体生成的公共曲面系数
    @return	: AlgoError
    @Author : liying@2023/06/17
    ************************************/
    C_API AlgoError C_CALL CurveFitCorrectCurveinIteration(AlgoHandle curvefit_handle, AlgoHandle curvefit_handle_real, AlgoHandle curvefit_handle_initial);


    /************************************
    @brief  : 利用拟合数据, 进行曲面修正
    @note   :
    @param  : curvefit_handle    有效的拟合曲面修正句柄
    @param  : in_chroma_handle   有效的meter_handle句柄 详见algo_led_meter.h
    @return	: AlgoError
    @Author : chengjiayi@2021/06/24
    ************************************/
    C_API AlgoError C_CALL CurveFitRectify(AlgoHandle curvefit_handle, AlgoHandle in_meter_handle);



    /************************************
    @brief  : 读取保存拟合曲面文件
    @note   : 后缀只能是.curvef
    @param  : curvefit_handle        有效的曲面拟合句柄
    @param  : in_curvefit_fullname   标定文件,后缀只能是.curvef
    @return : AlgoError
    @Author : chengjiayi@2021/06/24
    ************************************/
    C_API AlgoError C_CALL CurveFitSave(AlgoHandle curvefit_handle, const char* in_curvefit_fullname);
    C_API AlgoError C_CALL CurveFitLoad(AlgoHandle curvefit_handle, const char* in_curvefit_fullname);



    /************************************
    @brief  : 获取句柄内容
    @note   : 
    @param  : curvefit_handle    有效的曲面拟合句柄
    @param  : out_para           导出的结构体,必须已经初始化完毕
    @param  : in_is_deep_copy    是否深拷贝, 默认浅拷贝  1=深拷贝;0=浅拷贝
    @return	: AlgoError
    @Author : chengjiayi@2021/06/24
    ***********************************/
    C_API AlgoError C_CALL CurveFitGet(AlgoHandle curvefit_handle, AlgoMat* out_para, int in_is_deep_copy=0);


    /************************************
    @brief  : 将系数矩阵导出成图片
    @note   : 
    @param  : curvefit_handle       有效的曲面拟合句柄
    @param  : in_img_fullname
    @return	: AlgoError
    @Author : chengjiayi@2021/06/24
    ************************************/
    C_API AlgoError C_CALL CurveFitExportImage(AlgoHandle curvefit_handle, const char* in_img_fullname);


    /************************************
    @brief  : 获取buffer大小
    @note   :
    @param  : curvefit_handle   当前curvefit句柄
    @param  : out_len           buffer大小
    @return	: AlgoError
    @Author : ouxingtao@2021/07/07
    ************************************/
    C_API AlgoError C_CALL CurveFitGetBufferSize(AlgoHandle curvefit_handle, uint64_t* out_len);


    /************************************
    @brief  : 将curvefit里面的数据导出到buffer
    @note   :
    @param  : curvefit_handle   有效的curvefit句柄
    @param  : out_buffer        已经申请好申请空间
    @param  : in_out_len        外部申请好的空间大小,会用真实大小修改
    @return	: AlgoError
    @Author : ouxingtao@2021/07/02
    ************************************/
    C_API AlgoError C_CALL CurveFitToBuffer(AlgoHandle curvefit_handle, char* out_buffer, uint64_t* in_out_len);


    /************************************
    @brief  : 将buffer数据导入到curvefit
    @note   :
    @param  : curvefit_handle   有效的curvefit句柄
    @param  : in_buffer
    @param  : in_len
    @return	: AlgoError
    @Author : chengjiayi@2021/06/29
    ************************************/
    C_API AlgoError C_CALL CurveFitFromBuffer(AlgoHandle curvefit_handle, const char* in_buffer, uint64_t in_len);


	/**********************************************
	@brief: 检测输入的多个箱体的meter数据曲面是否修好, 返回曲面异常区域边界
	AlgoError：返回值
	@param	:	* meter_handle： 输入in_Num 个箱体数据
	@param	:	in_Num:       --输入箱体个数, 需要大于in_NccNum_thresh（默认为6），否则报错
	@param  :   *out_algoMat_border  --输出，曲面Mura边界点集合， 为1*N的矩阵，3通道， 通道0是点的X轴坐标，通道1是点的Y轴坐标，通道2是boerder序号，N是边界点数量
	@param  :   *out_algoMat_BoxID  --输出，曲面未修好的箱体ID的List
	@param  :   in_NccNum_thresh                  输入--规律化Mura阈值，可缺省，默认为6
	@param  :    in_Nccthreshold                  输入--相似箱体相似度阈值，可缺省，默认为1.3
	@Author		:  Yaof@2024.3.12
	**********************************************/
	C_API AlgoError C_CALL CurveFitCheckAbnormalArea(AlgoHandle* meter_handles, int in_Num, AlgoMat *out_algoMat_border, int *out_algoMat_BoxID, \
		int *out_Num, const int in_NccNum_thresh = 6, const float in_Nccthreshold = 1.3f);


	/**********************************************
	@brief: 手拉曲面边界
	AlgoError：返回值
	@param	:	in_width, int in_height           输入，数据宽高
	@param	:	*in_PosOri   输入参数， 原始点坐标
	@param	:	*in_PosNew： 输入参数， 手拉后点坐标
	@param	:	 *in_out_algoMat_border： 输入&输出， 输入手拉前的边界点集合，输出手拉后的边界点集合
	@Author		:  Yaof@2024.3.12
	**********************************************/
	C_API AlgoError C_CALL CurveFitPullAbnormalAreaByPoint(const int in_width, const int in_height, AlgoPoint *in_PosOri, AlgoPoint *in_PosNew, AlgoMat *in_out_algoMat_border);


	/*************************************************
	@brief: 根据曲面Mura重新生成曲面参数
	AlgoError：返回值
	@param  :   curvecalib_handle：  曲面句柄，不能为空
	@param	:	*meter_handles： 输入in_Num个箱体meter数据,  只能选一个通道数据，例如如果meter为X通道则只能修X通道曲面
	@param	:	in_Num:           输入，箱体个数
	@param  :   in_algoMat_BoxID:  输入，曲面异常箱体编号list ， 通过CurveCalibCheckAbnormalArea 获取
	@param	:	*in_algoMat_border  输入，曲面Mura的border， 通过CurveCalibPullAbnormalAreaByPoint或者CurveCalibCheckAbnormalArea 获取
	@Author		:  Yaof@2024.3.12
	**********************************************/
	C_API AlgoError C_CALL CurveFitModify(AlgoHandle curvecalib_handle, AlgoHandle* meter_handles, const int in_Num, int *in_algoMat_BoxID, const int in_boxNum, AlgoMat *in_algoMat_border);


    /************************************
    Release  句柄指向的对象的内存空间释放, 句柄指向的对象还在
    Destroy  句柄指向的对象销毁, 此句柄已经无效, 请配合handle=nullptr;

    handle=new OBJ;
    OBJ.Set(a_lot_data);

    Release 相当于:
        OBJ.Release();
    Destroy 相当于:
        OBJ.release();
        delete handle;
    ************************************/
    C_API AlgoError C_CALL CurveFitRelease(AlgoHandle curvefit_handle);
    C_API void C_CALL CurveFitDestroyHandle(AlgoHandle curvefit_handle);




#ifdef __cplusplus
}
#endif // __cplusplus
#endif