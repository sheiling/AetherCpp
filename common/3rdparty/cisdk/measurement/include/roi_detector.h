#ifndef __ROI_DETECTOR_H__
#define __ROI_DETECTOR_H__

#include <stdint.h>
#include "ci_declare.h"
#include "ci_define.h"
#include "algo_shot.h"
#include "roi_detector_data.h"
#include "roi_detector_errors.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


	/************************************
	@brief  : 计算单灯大小，用于质量评估、点定位等
	@note   : 针对常规屏体、裁切屏体与球形屏，使用不同计算方法；
	@param  : in_region						--ROI句柄
	@param  : in_shot					-	--shot句柄，Pattern信息必须是真实有效的
	@param  : out_point_wh					--计算得到的灯点宽高尺寸
	@param  : in_method						--对应的计算方法
					COMPUTE_POINT_TEMPLATE_WH_HEIGHT_METHOD = 0,			// 高度法，根据左上到左下、右上到右下角点距离以及灯点行数计算模板尺寸，适用于不抽行的屏体，例如球形屏球身
					COMPUTE_POINT_TEMPLATE_WH_RADIUS_METHOD = 1,			// 半径法，将大表行数作为半径灯点个数，roi的宽高均值作为直径计算灯点间距，适用于球形屏球顶
					COMPUTE_POINT_TEMPLATE_WH_WIDTH_HEIGHT_METHOD = 2,		// 宽高法，ROI的宽高除以点亮灯点行列数，适用于常规屏、裁切屏体
					COMPUTE_POINT_TEMPLATE_WH_AREA_METHOD = 3,				// 面积法，ROI的mask的面积除以灯点个数，得到单灯面积，开方得到灯点宽高，适用于抽点屏体、矩形屏体，不适用于裁切屏体(业务上拿不到灯点个数)  
	@param  : in_point_num					--屏体灯点个数
	@return	: AlgoError
	@Author : GuoHao@2025/11/18
	************************************/
	C_API AlgoError C_CALL ROIComputePointTemplateWh(const AlgoHandle in_region, const AlgoHandle in_shot, AlgoSize2f* out_point_wh
		, ComputePointTemplateWhMethod in_method = ComputePointTemplateWhMethod::COMPUTE_POINT_TEMPLATE_WH_WIDTH_HEIGHT_METHOD, int in_point_num = 0);

	// 手机亮度修缝相关C API接口如下：
	/************************************
   @Brief		：	用于网格打屏时的区域定位
   @Note		：	利用打屏的黑色区域进行定位，对于亮暗环境都适用；定位结果为边角处黑色矩形确定的屏体四个角点
   @Note       ：	边界：保证拍摄画面中没有与打屏时灯板的黑色矩形大小形状接近的暗块即可；对RGBW打屏都适用，推荐W;
   @Parameter	: 	roi_handle_in				区域定位句柄,用于调用内部C++函数
   @Parameter	: 	shot_handle_in				待待定的shot的句柄
   @Parameter	: 	band_size_in				打屏图案窄带宽度
   @Parameter	: 	edge_size_in				打屏图案边栏宽度
   @Parameter	: 	hem_size_in					打屏图案黑边宽度
   @Parameter	: 	max_sampling_rate_in		最大采样率，由相机像素分辨率与分区大小计算得到
   @Parameter	: 	grid_corner_out  			角点坐标,入参前需要已经初始化过,初始化空间必须>=4；从0-3分别是左上、右上、右下、左下角点；
   @Returns	:   AlgoError
   @Author		：	GuoHao@2023/11/15
   ************************************/
	C_API AlgoError C_CALL ROIAutoLocateForGrid(AlgoHandle roi_handle_in, AlgoHandle shot_handle_in, const int* band_size_in, const int* edge_size_in, const int* hem_size_in,
		const float* max_sampling_rate_in, AlgoPoint2f * grid_corner_out);

	/************************************
   @Brief		：	网格透视变换拉齐
   @Note		：	仅用于手机亮度修缝网格定位之后；根据黑色矩形做透视变换拉齐后将屏体区域裁切输出mat
   @Parameter	: 	roi_handle_in				区域定位句柄,用于调用内部C++函数
   @Parameter	: 	shot_handle_in				shot的句柄
   @Parameter	: 	band_size_in				打屏图案窄带宽度
   @Parameter	: 	edge_size_in				打屏图案边栏宽度
   @Parameter	: 	hem_size_in					打屏图案黑边宽度
   @Parameter	: 	grid_corner_in  			屏体四个角点坐标,0-3分别是左上、右上、右下、左下角点；由ROIAutoLocateForGrid()得到；
   @Parameter	: 	LED_ing_out  			    透视变换后截取的屏体区域；
   @Returns	:   AlgoError
   @Author		：	GuoHao@2023/11/15
   ************************************/
	C_API AlgoError C_CALL ROIGridTransform(AlgoHandle roi_handle_in, AlgoHandle shot_handle_in, const int* band_size_in, const int* edge_size_in, const int* hem_size_in,
		AlgoPoint2f * grid_corner_in, AlgoMat* LED_ing_out);

	// 手机亮度修缝相关C API接口如上
    
    C_API AlgoError C_CALL ROICreateHandle(AlgoHandle* out_region_handle);

    /************************************
    @Brief		：	设置区域定位场景FLAG
    @Note		：
    @Parameter	: 	region_handle -- 区域定位句柄
    @Parameter	: 	in_flag		  -- 参考roi_detector_data.h
    @Returns	:   AlgoError
    @Author		：	hanpanxia@2022/07/07
    ************************************/
    C_API AlgoError C_CALL ROISetProcFlag(AlgoHandle region_handle, RoiFlag in_flag);


	/************************************
	@Brief		：	设置区域定位掩膜MASKFLAG，仅对ROIAutoLocate接口的RoiFlag::ROI_FLAG_NORMAL模式有效
	@Note		：	MaskFlag解释见下
						MaskFlag::MASKFLAG_CONVEX_HULL = 0,			// 凸包补全的mask，适用于矩形屏体，该模块默认的mask类型
						MaskFlag::MASKFLAG_TRUE_SHAPE = 1,			// 真实形状的mask，适用于异形屏
	@Parameter	: 	region_handle -- 区域定位句柄
	@Parameter	: 	in_flag		  -- 参考roi_detector_data.h
	@Returns	:   AlgoError
	@Author		：	hanpanxia@2022/07/07
	 ************************************/
	C_API AlgoError C_CALL ROISetMaskFlag(AlgoHandle region_handle, MaskFlag in_flag);


    /************************************
    @Brief		：	自动区域定位（1.0版本修改by haopanxia）
    @Note		：	
    @Parameter	: 	region_handle -- 区域定位句柄
    @Parameter	: 	in_shot		  -- shot句柄
    @Returns	:   AlgoError
    @Author		：	haopanxia@2022/07/07 
    ************************************/
    C_API AlgoError C_CALL ROIAutoLocate(AlgoHandle region_handle, AlgoHandle in_shot);


	/************************************
	@Brief		：	点亮机多灯板自动区域定位
	@Note		：
	@Parameter	: 	region_handle                 -- 区域定位句柄
	@Parameter	: 	in_shot		                  -- shot句柄
	@Parameter	: 	in_panel_wh		              -- 输入图像中最小一个灯板区域的宽度和高度
	@Parameter	: 	out_panel_num		          -- 输出当前图像中检测得到的所有灯板数量
	@Returns	:   AlgoError
	@Author		：	haopanxia@2024/01/18
	************************************/
	C_API AlgoError C_CALL ROIAutoLocateForMultiPanels(AlgoHandle region_handle, AlgoHandle in_shot, const AlgoSize in_panel_wh, int* out_panel_num);


	/************************************
	  @Brief		：	获取点亮机多灯板自动区域定位后的4个角点，与ROIAutoLocateForMultiPanels配合使用
	  @Note		    ：
	  @Parameter	: 	region_handle		--区域定位句柄
	  @Parameter	: 	out_all_corner4		--角点坐标,已经初始化过,外部用algomat形式保存，行数代表角点个数，每一行保存的是一个区域所对应的四个角点
	  @Parameter	: 	in_corner_type		--默认0=Rect角点; 1=RRect角点; 2=contour角点; 3=精确角点
	  @Returns	    :   AlgoError
	  @Author		：	haopanxia@2024/01/18
	  ************************************/
	C_API AlgoError C_CALL ROIGetMultiPanels4Corner(AlgoHandle region_handle, AlgoMat* out_all_corner4, int in_corner_type = 0);




	/************************************
	@Brief		：	自动区域定位后获取单块屏的Shot
	@Note		：
	@Parameter	: 	region_handle     -- 区域定位句柄
	@Parameter	: 	in_img		  -- 原始拼接屏Shot
	                camera_wh     --图像宽高
					in_mask       --单块屏ROI计算的mask
					in_num        --拼接屏数量
	@Returns	:   AlgoError
	@Author		：	qianxiaotong@2023/06/16
	************************************/
	C_API AlgoError C_CALL ROIToSingleSpliceShot(const AlgoHandle in_shot, const AlgoSize in_camera_wh, const AlgoSize in_block_wh, AlgoHandle *in_roi, AlgoHandle *single_screen_out, const int in_num);

	/************************************
	@Brief		：	自动区域定位——液晶拼接屏各块区域并排序
	@Note		：
	@Parameter	: 	region_handle     -- 区域定位句柄
	@Parameter	: 	in_shot		  -- SHOT图像
	                block_wh    --拼接屏横纵数量
					nRoi_out_len  --识别到屏体个数为1或拼接屏个数
	@Returns	:   AlgoError
	@Author		：	qianxiaotong@2023/06/15
	************************************/
	C_API AlgoError C_CALL ROIAutoLocateForSplice(const AlgoHandle in_shot, const AlgoSize in_block_wh, AlgoHandle nRoi_out[], int out_len);

	/************************************
	@Brief		：	从AlgoMat中直接自动区域定位
	@Note		：  传入AlgoMat类型的数据，直接对该数据进行自动区域定位
	@Parameter	: 	region_handle -- 区域定位句柄
	@Parameter	: 	in_algomat	  -- 输入待区域定位的AlgoMat数据
	@Returns	:   AlgoError
	@Author		：	wushaoshuang@2023/06/09
	************************************/
	C_API AlgoError C_CALL ROIAutoLocateFromAlgoMat(AlgoHandle region_handle, AlgoMat* in_algomat);


	/************************************
	@Brief		：	自动区域定位——亮环境下的JPG图像——目前屏体颜色只能为绿色（1.0版本修改by zhangxuan）
	@Note		：
	@Parameter	: 	region_handle     -- 区域定位句柄
	@Parameter	: 	in_JPGfile		  -- JPG图像的路径
	@Returns	:   AlgoError
	@Author		：	zhangxaun@2023/04/06
	************************************/
	C_API AlgoError C_CALL ROIAutoLocateForBright(AlgoHandle region_handle, const char* in_JPGfile);

    /************************************
    @Brief		：	自动区域定位
    @Note		：
    @Parameter	: 	region_handle -- 区域定位句柄
    @Parameter	: 	in_shot		  -- 待处理的数据句柄
	@Parameter	: 	in_shot_type  -- in_shot句柄类型，支持shot句柄和thermal_imager句柄
    @Returns	:   AlgoError
    @Author		：	tianbo@2021/09/23
    ************************************/
	C_API AlgoError C_CALL ROIAutoLocateForBlur(AlgoHandle region_handle, AlgoHandle in_shot, int in_shot_type = ALGOMOD_ALGOSHOT);

	


    /************************************
      @Brief		：	获取自动区域定位后的4个角点
      @Note		    ：
      @Parameter	: 	region_handle		--区域定位句柄
      @Parameter	: 	in_corner_type		--默认0=Rect角点;1=RRect角点;2=contour 角点; 3=精确角点;4=小分辨率精确角点 
      @Parameter	: 	out_corner4		    --  角点坐标,已经初始化过,初始化空间必须>=4
      @Returns	    :   AlgoError
      @Author		：	chengjiayi@2021/04/26
      ************************************/
    C_API AlgoError C_CALL ROIGet4Corner(AlgoHandle region_handle, AlgoPoint* out_corner4, int in_corner_type = 0);


    /************************************
      @Brief		：	区域定位四个角点外扩
      @Note		    ：
      @Parameter	: 	region_handle -- 区域定位句柄
      @Parameter	: 	shift		  -- shift.x 和 shift.y 分别为四个角点在x和y方向的偏移量
      @Returns   	:   AlgoError
      @Author		：	hanpanxia@2022/08/10
      ************************************/

    C_API AlgoError C_CALL ROIExpand(AlgoHandle region_handle, const AlgoPoint in_shift);



    /************************************
    @Brief		：	手动区域定位
    @Note		：	
    @Parameter	: 	region_handle		--区域定位句柄
    @Parameter	: 	in_img_wh		    --图像大小
    @Parameter	: 	in_corner4		-- 输入的4个角点
    @Parameter	: 	in_num		--  4
    @Returns	:   AlgoError
    @Author		：	chengjiayi@2021/04/26 
    ************************************/
    C_API AlgoError C_CALL ROISet4Corner(AlgoHandle region_handle, const AlgoSize* in_img_wh, const AlgoPoint* in_corner4,int in_num=4);



    /************************************
    @brief  : 设置形变区域
    @note   : 
    @param  : region_handle --区域定位句柄
    @param  : in_corner4    -- 输入的4个角点, 凸四边形
    @param  : in_num        4
    @return	: AlgoError
    @Author : chengjiayi@2021/07/14
    ************************************/
    C_API AlgoError C_CALL ROIPerspectiveSet4Corner(AlgoHandle region_handle, const AlgoPoint* in_corner4, int in_num = 4);

	/************************************
	@brief  : 设置形变区域
	@note   : 使用超分提取光通量时前置透视变化Set操作
	@param  : region_handle  --区域定位句柄
	@param  : img_resolution --图像的分辨率，如C3200位{6464,4852}.
	@param  : in_corner4    -- 输入的4个角点, 凸四边形，可为roi自动定位得到的精确四点
	@param  : in_num        4
	@return	: AlgoError
	@Author : tianbo@2021/10/10
	************************************/
	C_API AlgoError C_CALL ROIPerspectiveSet4CornerFroBlur(AlgoHandle region_handle, AlgoSize img_resolution,const AlgoPoint* in_corner4, int in_num = 4);

    /************************************
    @brief  : 对输入shot图像进行透视变换,任意四边形区域变换为矩形
    @note   : 
    @param  : region_handle --区域定位句柄
    @param  : in_out_shot   --图像
    @return	: AlgoError
    @Author : chengjiayi@2021/07/14
    ************************************/
    C_API AlgoError C_CALL ROIWarpPerspective(AlgoHandle region_handle, AlgoHandle in_out_shot);


    /************************************
    @brief  : 旋转roi区域
    @note   : 必须和shot同步旋转
    @param  : region_handle     --区域定位句柄
    @param  : in_roll_angle     --旋转角度, 详见 algo_shot.h中  ShotRotate
    @return	: AlgoError
    @Author : chengjiayi@2021/07/27
    ************************************/
    C_API AlgoError C_CALL ROIRotate(AlgoHandle region_handle, int in_roll_angle);


    /************************************
    @brief  : 获取区域数据
    @note   : 
    @param  : region_handle 有效的句柄
    @param  : out_region    导出的结构体,必须已经初始化完毕
    @param  : is_deep_copy  是否深拷贝, 默认浅拷贝 1=深拷贝;0=浅拷贝
    @return	: AlgoError
    @Author : chengjiayi@2021/05/13
    ************************************/
    C_API AlgoError C_CALL ROIGet(AlgoHandle region_handle, AlgoROI* out_region, int in_is_deep_copy = 0);

	/************************************
	@brief  : 根据输入的角点信息输出对应rect及mask
	@note   :
	@param  : in_corner     输入的角点信息，要求角点数量≥3个
	@param  : in_img_size   输入保存为mask的图像尺寸
	@param  : in_num        角点个数
	@param  : out_rect      输出的最小外接正矩形
	@param  : out_mask      输出的角点组成的mask，可选out_mask = nullptr时，不输出mask信息
	@return	: AlgoError
	@Author : haopanxia@2023/01/12
	************************************/
	C_API AlgoError C_CALL ROICornersToRectMask(const AlgoPoint* in_corner, const AlgoSize* in_img_size, int in_num, AlgoRect* out_rect, AlgoMat* out_mask = nullptr);


    //C_API AlgoError C_CALL LedRegionSet(AlgoHandle region_handle, const AlgoLedRegion* in_region);


	/************************************
   @Brief		：	ROI单方向等间隔拆分
   @Note		：	用于多色质量评估打屏时提取每一颜色roi
   @Parameter	: 	in_color_pattern_num	  多色打屏时颜色数目
   @Parameter	: 	in_mode                   分区模式，拆分方向，可选0或1。 0：表示水平方向拆分；1：表示竖直方向拆分；
   @Parameter	: 	out_nRoi                  提取的多个颜色roi(长度 == in_color_pattern_num）
   @Returns	    :   AlgoError
   @Author		：	tianbo@2025/01/20
   ************************************/
	C_API AlgoError C_CALL ROIUnidirectionalUniformDivide(AlgoHandle out_region_handle, const int in_divide_num, const int in_mode, AlgoHandle* out_nRoi);



    /************************************
    @Brief		：	载入和保存区域定位结果
    @Note		：	文件名后缀必须为.roi
    @Parameter	: 	region_handle		---区域定位句柄
    @Parameter	: 	in_roi_fullname		--文件后缀必须为.roi
    @Returns	:   AlgoError
    @Author		：	chengjiayi@2021/04/26 
    ************************************/
    C_API AlgoError C_CALL ROILoad(AlgoHandle region_handle, const char* in_roi_fullname);
    C_API AlgoError C_CALL ROISave(AlgoHandle region_handle, const char* in_roi_fullname);


    /************************************
    @Brief		：	导出区域定位结果图片
    @Note		：	
    @Parameter	: 	region_handle		--
    @Parameter	: 	in_img_fullname		--图像文件 png jpg tiff
    @Parameter	: 	in_shot_handle		--输入的shot句柄, nullptr=绘制框到空白图上, 不为nullptr=把roi框绘制原图上
    @Returns	:   AlgoError
    @Author		：	chengjiayi@2021/04/26 
    ************************************/
    C_API AlgoError C_CALL ROIExportImage(AlgoHandle region_handle, const char* in_img_fullname, AlgoHandle in_shot_handle=nullptr);


    /************************************
	@Brief		：	导出点亮机多面板区域定位结果图片
	@Note		：
	@Parameter	: 	region_handle		--
	@Parameter	: 	in_img_fullname		--图像文件 png jpg tiff
	@Parameter	: 	in_shot_handle		--输入的shot句柄, nullptr=绘制框到空白图上, 不为nullptr=把roi框绘制原图上
	@Returns	:   AlgoError
	@Author		：	haopanxia@2024/01/18
	************************************/
	C_API AlgoError C_CALL ROIExportImageForMultiPanels(AlgoHandle region_handle, const char* in_img_fullname, AlgoHandle in_shot_handle);



    /************************************
    @brief  : 获取buffer大小
    @note   :
    @param  : region_handle  当前有效句柄
    @param  : out_len     buffer大小
    @return	: AlgoError
    @Author : chengjiayi@2021/07/06
    ************************************/
    C_API AlgoError C_CALL ROIGetBufferSize(AlgoHandle region_handle, uint64_t* out_len);

    /************************************
    @brief  : 将region里面的数据导出到buffer
    @note   :
    @param  : region_handle   有效的region句柄
    @param  : out_buffer      已经申请好申请空间
    @param  : in_out_len      外部申请好的空间大小,会用真实大小修改
    @return	: AlgoError
    @Author : ouxingtao@2021/07/01
    ************************************/
    C_API AlgoError C_CALL ROIToBuffer(AlgoHandle region_handle, char* out_buffer, uint64_t* in_out_len);


    /************************************
    @brief  : 将buffer数据导入到region
    @note   :
    @param  : region_handle   有效的shot句柄
    @param  : in_buffer
    @param  : in_len
    @return	: AlgoError
    @Author : ouxingtao@2021/07/01
    ************************************/
    C_API AlgoError C_CALL ROIFromBuffer(AlgoHandle region_handle, const char* in_buffer, uint64_t in_len);


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
    C_API AlgoError C_CALL ROIRelease(AlgoHandle region_handle);
    C_API void C_CALL ROIDestroyHandle(AlgoHandle region_handle);



#ifdef __cplusplus
}
#endif // __cplusplus
#endif