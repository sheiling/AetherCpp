
#ifndef __ALGO_SHOT_H__
#define __ALGO_SHOT_H__

#include "ci_define.h"
#include "ci_declare.h"
#include "algo_shot_data.h"
#include "algo_shot_errors.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	//初始化颜色为黑色
    C_API AlgoError C_CALL ShotCreateHandle(AlgoHandle* out_shot_handle);

	/************************************
	@Brief		：	对shot中image图像取某块矩形区域内数据求均值
	@Note		：  in_roi值全为0时会计算全局均值
	@Parameter	: 	in_shothandle		--shot句柄
	@Parameter	: 	in_roi		        --矩形区域，(x,y)为左上角坐标，width为矩形宽，height为矩形高
	@Parameter	: 	out_mean		    --输出矩形内数据均值
	@Returns	:   AlgoError
	@Author		：	qianxiaotong@2024/08/26
	************************************/
	C_API AlgoError C_CALL ShotCalAreaMean(AlgoHandle in_shothandle,const AlgoRect in_roi,float* out_mean);


	/************************************
	@Brief		：	shot均匀亮场图像去异常点
	@Note		：
	@Parameter	: 	shot_handle		--shot句柄
	@Returns	:   AlgoError
	@Author		：	qianxiaotong@2024/06/06
	************************************/
	C_API AlgoError C_CALL ShotUniformBrightFiledReplaceAbnormal(AlgoHandle shot_handle);

	/************************************
	@Brief		：	shot暗场图像去异常点
	@Note		：
	@Parameter	: 	shot_handle		--shot句柄
	@Returns	:   AlgoError
	@Author		：	qianxiaotong@2023/12/20
	************************************/
	C_API AlgoError C_CALL ShotUniformFiledReplaceAbnormal(AlgoHandle shot_handle);

   
	/************************************
	@Brief		：	读取jpg格式图像(三通道)
	@Note		：
	@Parameter	: 	shot_handle		--shot句柄
	@Parameter	: 	in_dng_file		--输入的dng文件全名
	@Returns	:   AlgoError
	@Author		：	qianxiaotong@2023/8/11
	************************************/
	C_API AlgoError C_CALL ShotReadJpg(AlgoHandle shot_handle, const char* in_dng_file);
	


	
	/************************************
    @Brief		：	读取dng格式,只支持16U
    @Note		：
    @Parameter	: 	shot_handle		--shot句柄
    @Parameter	: 	in_dng_file		--输入的dng文件全名
    @Parameter	: 	in_cie_type	    --颜色分量类型 dng debayer后,保留选择的通道;in_cie_type=CIE_UNDEFINED 先不debayer; 详见ShotSetImage
    @Returns	:   AlgoError
    @Author		：	chengjiayi@2022/11/10
    ************************************/
    C_API AlgoError C_CALL ShotReadDng(AlgoHandle shot_handle, const char* in_dng_file, CIEType in_cie_type);



	/************************************
	@Brief		：	对shot进行检测
	@Note		：	目前只支持对背景图进行检测
	@Parameter	: 	shot_handle		--shot句柄
	@Parameter	: 	in_detec_type	--检测的类型，目前仅支持ShotDetectBg
	@Parameter	: 	in_rect			--待检测区域的roi
	@Parameter	: 	detec_result	--检测结果，详见_data.h。  尽在AlgoError为ALGO_PASS时有效
	@Returns	:   AlgoError
	@Author		：	tianbo@2024/01/08
	************************************/
	C_API AlgoError C_CALL ShotDetec(AlgoHandle shot_handle, ShotDetectType in_detec_type, AlgoRect in_rect, ShotDetectResult* detec_result);



	/************************************
	@Brief		：	读取dng格式,只支持16U
	@Note		：  dng中图像的大小与rect的wh一致，可节省数据传输以及数据存储
	@Parameter	: 	shot_handle		--shot句柄
	@Parameter	: 	in_dng_file		--输入的dng文件全名
	@Parameter	: 	in_roi_rect		--roi_rect，roi为一个rect
	@Parameter	: 	in_cie_type	    --颜色分量类型 dng debayer后,保留选择的通道;in_cie_type=CIE_UNDEFINED 先不debayer; 详见ShotSetImage
	@Returns	:   AlgoError
	@Author		：	tianbo@2023/11/06
	************************************/
	C_API AlgoError C_CALL ShotReadDngRoiRect(AlgoHandle shot_handle, const char* in_dng_file, const AlgoRect in_roi_rect, CIEType in_cie_type);


	/************************************
	@Brief		：	从dng读取raw格式,只支持16U
	@Note		：
	@Parameter	: 	shot_handle		--shot句柄
	@Parameter	: 	in_raw_file		--输入的raw_dng文件全名
	@Parameter  :	in_cie_type		--颜色分量类型, 详见ShotSetImage
	@Returns	:   AlgoError
	@Author		：	tianbo@2023/08/24
	************************************/
	C_API AlgoError C_CALL ShotReadRawFromDng(AlgoHandle shot_handle, const char* in_raw_file, CIEType in_cie_type);


	/************************************
	@Brief		：	从dng读取raw格式,只支持16U
	@Note		：	in_raw_file的大小为rect的wh，用于沟通仅roi中的shot，可节省数据传输以及数据存储
	@Parameter	: 	shot_handle		--shot句柄
	@Parameter	: 	in_raw_file		--输入的raw_dng文件全名
	@Parameter	: 	in_roi_rect		--roi_rect，roi为一个rect
	@Parameter  :	in_cie_type		--颜色分量类型, 详见ShotSetImage
	@Returns	:   AlgoError
	@Author		：	tianbo@2023/08/24
	************************************/
	C_API AlgoError C_CALL ShotReadRawFromDngRoiRect(AlgoHandle shot_handle, const char* in_raw_file, const AlgoRect in_roi_rect, CIEType in_cie_type);


	/************************************
	@brief  : 对shot中的图像进行滤波
	@note   : 
	@param  : shot_handle   --有效shot句柄
	@param  : in_shot   --shot数据文件
	@param  : in_rad_w   --滤波核宽度
	@param  : in_rad_h  --滤波核高度
	@param  : in_sigma_w --sigma宽
	@param  : in_sigma_h --sigma高
	@return	: AlgoError
	@Author : qianxiaotong@2023/6/25
	************************************/
	C_API AlgoError C_CALL ShotImageFilter(AlgoHandle shot_handle,AlgoHandle in_shot, const int in_rad_w, const int in_rad_h, const double in_sigma_w, const double in_sigma_h, AlgoHandle out_shotfilter_handle);
    
    /************************************
    @brief  : 读取raw数据到shot
    @note   : raw数据必须是
                1 纯像素数据,不包含其他数据
                2 rowmajor
                3 每个像素位深uint16 或者 uint8
    @param  : shot_handle   --有效shot句柄
    @param  : in_raw_file   --raw数据文件
    @param  : in_cie_type   --颜色分量类型, 详见ShotSetImage
    @param  : in_cam_width  --相机图像宽
    @param  : in_cam_height --相机图像高
    @param  : in_bayer_type --bayer类型, 详见ShotSetImage
    @param  : in_cam_bit_depth --图像位深，可选择读取16位图像 和 8位图像，默认为16
    @return	: AlgoError
    @Author : mumeng@2022/8/25
    ************************************/
    C_API AlgoError C_CALL ShotReadRaw(AlgoHandle shot_handle, 
        const char* in_raw_file,
        CIEType in_cie_type,
        int in_cam_width, int in_cam_height,
        BayerType in_bayer_type,
        int in_cam_bit_depth = 16);


	/************************************
	@brief  : 读取raw数据到shot
	@note   : 读取的raw文件中的wh必须和roi_rect的wh一致
				raw数据必须是
				1 纯像素数据,不包含其他数据
				2 rowmajor
				3 每个像素位深uint16 或者 uint8
	@param  : shot_handle   --有效shot句柄
	@param  : in_raw_file   --raw数据文件
	@param  : in_roi_rect   --roi_rect，roi为一个rect. 其中rect的wh与图像的wh一致
	@param  : in_cie_type   --颜色分量类型, 详见ShotSetImage
	@param  : in_bayer_type --bayer类型, 详见ShotSetImage
	@param  : in_cam_bit_depth --图像位深，可选择读取16位图像 和 8位图像，默认为16
	@return	: AlgoError
	@Author : tianbo@2023/11/07
	************************************/
	C_API AlgoError C_CALL ShotReadRawRoiRect(AlgoHandle shot_handle,
		const char* in_raw_file,
		const AlgoRect in_roi_rect,
		CIEType in_cie_type,
		BayerType in_bayer_type,
		int in_cam_bit_depth = 16);

	/************************************
	@brief  : 将shot变为roi shot
	@note   : 直接对shot_handle做改变，处理后的shot仅有roi_rect的尺寸大小
	@param  : shot_handle --
	@param :  in_roi_rect --roi_rect区域
	@return : AlgoError
	@Author : tianbo@2023/11 / 07
	************************************/
	C_API AlgoError C_CALL ShotToRoiRectShot(AlgoHandle shot_handle, AlgoRect in_roi_rect);



	/************************************
	 @brief  : 生成公共shot
	 @note   : 用num组shot数据生成公共shot
	 @param  : in_shot_handles		若干个shot handle
	 @param  : in_num				in_meter_handles个数
	 @param  : out_shot_handle		handle	输出平均的shot handle,信息与第一个shot一致
	 @return : AlgoError
	 @Author : tianbo@2023/08/02
	 ************************************/
	C_API AlgoError C_CALL ShotAverage(AlgoHandle* in_shot_handles, const int in_num, AlgoHandle out_shot_handle);



    /************************************
     @brief  : 将raw数据中的坏点进行检测并修复
     @note   : 如果需要修坏点, ShotReadRaw或者ShotReadDng必须COLOR_BayerNone
               修完坏点后,再ShotDebayer, 详见DemoUsingReadRawWithFixAbnormalPixels
     @param  : shot_handle      --有效shot句柄
     @param  : in_threshold     --坏点检测阈值，默认值为3,in_threshold参数用于控制坏点检出度，该值代表坏点与8邻域均值的比率，大于该比率判定为坏点
     @return	: AlgoError
     @Author : songshaojun@2022/08/11
     ************************************/
    C_API AlgoError C_CALL ShotFixAbnormalPixels(AlgoHandle shot_handle, const float in_threshold = 3.f);



    /************************************
    @brief  :  设置Shot中的图像数据
    @note   :  1) in_bayer_type== COLOR_BayerNone
                    a 当in_cie_type= CIE_XYZ或者in_cie_type= CAM_RGB时候, in_image必须为 ALGO_16UC3 或 ALGO_8UC3

                    b 当 in_cie_type= CIE_X 或者 CIE_Y 或者 CIE_Z 或者 CAM_R 或者 CAM_G 或者 CAM_B,in_image必须为 ALGO_16UC1 或 ALGO_8UC1

                    举例, 单色工业相机, 如C1200,C3200
                        in_cie_type= CIE_X 或者 CIE_Y 或者 CIE_Z

                    举例, 彩色相机可以先不debayer,可以结合ShotDebayer函数后面debayer处理
                        in_cie_type=CIE_UNDEFINED
                
               2) in_bayer_type != COLOR_BayerNone
                    a.in_image代表的是raw数据, 必须为 ALGO_16UC1 或 ALGO_8UC1

                    b.in_cie_type  = CAM_R或CAM_G或CAM_B 或 CAM_RGB
                    会先对in_image按照in_bayer_type进行debayer操作
                    然后取in_cie_type


                    举例,彩色相机,彩色工业相机/数码相机/手机
                    in_bayer_type=如实填写
                    当in_cie_type=CAM_RGB,则内部debayer,且保留三个通道

                    当in_cie_type= CAM_R 或者 CAM_G 或者 CAM_B
                        则内部debayer,只保留in_cie_type分量,数据类型变为 ALGO_16UC1 或 ALGO_8UC1

    @param  : shot_handle   有效shot句柄
    @param  : in_image      需要软件端自己构造AlgoMat
    @param  : in_cie_type   cie分量类型
    @param  : in_bayer_type bayer类型
    @return	: AlgoError
    @Author : chengjiayi@2022/09/19
    ************************************/
    C_API AlgoError C_CALL ShotSetImage(AlgoHandle shot_handle,
        const AlgoMat* in_image,
        CIEType in_cie_type,
        BayerType in_bayer_type= COLOR_BayerNone);

	/************************************
	@brief  :  设置Shot中的图像数据
	@note   :  im_image的大小与roi_rect一致。其余注意事项同上
	@param  : shot_handle   有效shot句柄
	@param  : in_image      需要软件端自己构造AlgoMat
	@param  : in_roi_rect   roi_rect表示当前img在原始图像的位置偏移及大小
	@param  : in_cie_type   cie分量类型
	@param  : in_bayer_type bayer类型
	@return	: AlgoError
	@Author : tianbo@2023/11/07
	************************************/
	C_API AlgoError C_CALL ShotSetImageRoiRect(AlgoHandle shot_handle,
		const AlgoMat* in_image,
		const AlgoRect in_roi_rect,
		CIEType in_cie_type,
		BayerType in_bayer_type = COLOR_BayerNone);


    /************************************
    @brief    : shot单独设置cie_type
    @note     : 
    @param    : shot_handle     有效shot句柄,shot内容不能为空
    @param    : in_cie_type     
    @return   : AlgoError
    @Modified : chengjiayi@2022/09/27
    ************************************/
    C_API AlgoError C_CALL ShotSetType(AlgoHandle shot_handle, CIEType in_cie_type);


    /************************************
    @brief  : 直接从内存构建shot
    @note   : 不包含元信息,详细同ShotSetImage说明
    @param  : shot_handle   有效shot句柄
    @param  : in_pimge      图像指针
    @param  : in_rows       图像行列数
    @param  : in_cols       图像行列数
    @param  : in_mat_type   图像类型
    @param  : in_cie_type   cie分量类型
    @param  : in_bayer_type bayer类型
    @return : AlgoError
    @Author : chengjiayi@2022/09/19
    ************************************/
    C_API AlgoError C_CALL ShotCreateFromMem(AlgoHandle shot_handle,
        const char* in_pimge, int in_rows, int in_cols, AlgoMatType in_mat_type,
        CIEType in_cie_type, BayerType in_bayer_type = COLOR_BayerNone);

	/************************************
	@brief  : 直接从内存构建shot
	@note   : 不包含元信息,详细同ShotSetImage说明
	@param  : shot_handle   有效shot句柄
	@param  : in_pimge      图像指针
	@param  : in_roi_rect   图像当前的位置信息。其中xy表示in_pimge相对于原图的偏移，wh表示in_pimge的wh
	@param  : in_mat_type   图像类型
	@param  : in_cie_type   cie分量类型
	@param  : in_bayer_type bayer类型
	@return : AlgoError
	@Author : chengjiayi@2022/09/19
	************************************/
	C_API AlgoError C_CALL ShotCreateFromMemRoiRect(AlgoHandle shot_handle,
		const char* in_pimge,  AlgoMatType in_mat_type, const AlgoRect in_roi_rect,
		CIEType in_cie_type, BayerType in_bayer_type = COLOR_BayerNone);


    /************************************
    @brief  : 设置彩色图像进行debayer
    @note   : 1) debayer前, shot的cie_type只能为CIE_UNDEFINED
              2) debayer后, shot的图像大小和原始图像一样大, 但是只对in_rect区域debayer, 区域外复制对应raw

    @param  : shot_handle
    @param  : in_cie_type    CAM_R CAM_G CAM_B CAM_RGB
    @param  : in_bayer_type  相机bayer类型
    @param  : in_rect        只对区域debayer, NULL=不指定区域, 全区域debayer
    @return	: AlgoError
    @Author : chengjiayi@2022/03/28
    ************************************/
    C_API AlgoError C_CALL ShotDebayer(AlgoHandle shot_handle, 
        CIEType in_cie_type,
        BayerType in_bayer_type,
        const AlgoRect* in_rect=nullptr);





    //---------1 set meta info

    /************************************
    @Brief		：	set 设备信息
    @Note		：	
    @Parameter	: 	shot_handle		--
    @Parameter	: 	in_device		--设备信息
    @Returns	:   AlgoError
    @Author		：	chengjiayi@2021/04/26 
    ************************************/
    C_API AlgoError C_CALL ShotSetDevice(AlgoHandle shot_handle, const Device* in_device);


    /************************************
    @Brief		：	set 拍摄信息
    @Note		：	
    @Parameter	: 	shot_handle		
    @Parameter	: 	in_capture		拍摄信息
    @Returns	:   AlgoError
    @Author		：	chengjiayi@2021/04/26 
    ************************************/
    C_API AlgoError C_CALL ShotSetCapture(AlgoHandle shot_handle, const Capture* in_capture);


    /************************************
    @Brief		：	设置分区信息
    @Note		：	
    @Parameter	: 	shot_handle		
    @Parameter	: 	in_part		-- 分区信息
    @Returns	:   AlgoError
    @Author		：	chengjiayi@2021/04/26 
    ************************************/
    C_API AlgoError C_CALL ShotSetPartition(AlgoHandle shot_handle, const Partition* in_part);


    /************************************
    @Brief		：	set面板信息
    @Note		：	
    @Parameter	: 	shot_handle		
    @Parameter	: 	in_panel		面板信息
    @Returns	:   AlgoError
    @Author		：	chengjiayi@2021/04/26 
    ************************************/
    C_API AlgoError C_CALL ShotSetLedPanel(AlgoHandle shot_handle, const LedPanel* in_panel);


    /************************************
    @Brief		：	设置Layer信息
    @Note		：	
    @Parameter	: 	shot_handle		--
    @Parameter	: 	in_layer		--Layer信息
    @Returns	:   AlgoError
    @Author		：	chengjiayi@2021/04/26 
    ************************************/
    C_API AlgoError C_CALL ShotSetLayer(AlgoHandle shot_handle, const Layer* in_layer);

    /************************************
    @Brief		：	设置打点信息
    @Note		：	1.先构造Pattern,其中当Pattern.lighted_led_wh={-1,-1} 且 已经SetPartition会,自动计算lighted_led_wh
                    2.背景图设置不设置都行
    @Parameter	: 	shot_handle		--shot句柄
    @Parameter	: 	in_pattern		--打点信息
    @Returns	:   AlgoError
    @Author		：	chengjiayi@2021/04/26
    ************************************/
    C_API AlgoError C_CALL ShotSetPattern(AlgoHandle shot_handle, const Pattern* in_pattern);//deep copy



    /************************************
    @Brief		：	set 设备打屛颜色信息
    @Note		：	背景图设置不设置都行
    @Parameter	: 	shot_handle		--
    @Parameter	: 	in_color		--打屛颜色信息
    @Returns	:   AlgoError
    @Author		：	chengjiayi@2021/04/26 
    ************************************/
    C_API AlgoError C_CALL ShotSetColor(AlgoHandle shot_handle, AlgoColor in_color);

	/************************************
	@Brief		：	往operate_buffer中写入长度为in_info_len的业务操作记录数据
	@Note		：	
	@Parameter	: 	shot_handle		--
	@Parameter	: 	in_op_info		--输入业务操作记录数据
	@Parameter	: 	in_info_len		--输入业务操作记录数据的长度，不能超过SHOT_OPERATE_BUFFER_SIZE
	@Returns	:   AlgoError
	@Author		：	zhongao@2025/02/26
	************************************/
	C_API AlgoError C_CALL ShotSetOperateBuffer(AlgoHandle shot_handle, const char* in_op_info, size_t in_info_len);

	/************************************
	@Brief		：	从operate_buffer中读取全部的业务操作记录数据
	@Note		：  每次固定读取SHOT_OPERATE_BUFFER_SIZE大小的数据
	@Parameter	: 	shot_handle		--
	@Parameter	: 	out_op_info		--业务操作记录数据读到外部内存段
	@Parameter	: 	in_info_len		--外部内存段长度，不能小于SHOT_OPERATE_BUFFER_SIZE，默认为SHOT_OPERATE_BUFFER_SIZE
	@Returns	:   AlgoError
	@Author		：	zhongao@2025/02/26
	************************************/
	C_API AlgoError C_CALL ShotGetOperateBuffer(AlgoHandle shot_handle, char* out_op_info, size_t in_info_len = SHOT_OPERATE_BUFFER_SIZE);


	/************************************
	@Brief		：	set roi_rect信息
	@Note		：	当设置了roi_rect，后续set的img或之前set的img的wh必须和roi_rect的wh一致。
	@Parameter	: 	shot_handle		--
	@Parameter	: 	in_roi_rect		--roi_rect区域
	@Returns	:   AlgoError
	@Author		：	tianbo@2023/11/07
	************************************/
	C_API AlgoError C_CALL ShotSetRoiRect(AlgoHandle shot_handle, AlgoRect in_roi_rect);


    /************************************
    @brief  : 旋转shot图像
    @note   : 测量目标正方向为0°  右手坐标系
                roll = 0,相机视野正方向0°
                roll = 90,//相机视野正方向90°
                roll = 180,//相机视野正方向180°
                roll = 270,//相机视野正方向270°
            roll<0 时候, roll=roll%360+360;
    @param  : shot_handle    --有效的shot句柄
    @param  : in_roll_angle  --旋转角度,可以为负值  
    @return	: AlgoError
    @Author : chengjiayi@2021/07/27
    ************************************/
    C_API AlgoError C_CALL ShotRotate(AlgoHandle shot_handle, int in_roll_angle);

	/************************************
	@brief  : 把shot图像 区域1 内容移动到全零图像 区域2 中,新图像代替原shot图像,区域1 和 区域2 区域大小要一致,都不能超过图像边界
	@note   :
	@param  : shot_handle   --有效的shot句柄
	@param  : in_src        --区域1,源
	@param  : in_dst        --区域2,目的地
	@return	: AlgoError
	@Author : chengjiayi@2024/03/21
	************************************/
	C_API AlgoError C_CALL ShotMoveRegion(AlgoHandle shot_handle, const AlgoRect* in_src, const AlgoRect* in_dst);


    /************************************
    @brief  : 对shot图像翻转
    @note   : 支持roi shot
    @param  : shot_handle   --有效的shot句柄
    @param  : in_flip_code  in_flip_code>0,沿y轴翻转;in_flip_code==0,沿x轴翻转;in_flip_code<0,同时翻转
    @return	: AlgoError
    @Author : chengjiayi@2022/03/28
    ************************************/
    C_API AlgoError C_CALL ShotFlip(AlgoHandle shot_handle, int in_flip_code);

    /************************************
    @brief  : 设置相机位置
    @note   : 
    @param  : shot_handle   --有效的shot句柄
    @param  : in_cam_pos    --相机位置
    @return	: AlgoError
    @Author : chengjiayi@2021/07/26
    ************************************/
    //C_API AlgoError C_CALL  ShotSetCamPos(AlgoHandle shot_handle, const CamPos* in_cam_pos);




    //--------- 0+1 set 
    C_API AlgoError C_CALL ShotSet(AlgoHandle shot_handle,const AlgoShot* in_shot);



    //--------- tools





    /************************************
    @Brief		：	保存载入Shot文件
    @Note		：	后缀只能是.shot
    @Parameter	: 	shot_handle		--
    @Parameter	: 	in_shot_fullname		--
    @Returns	:   AlgoError
    @Author		：	chengjiayi@2021/04/26 
    ************************************/
    C_API AlgoError C_CALL ShotLoad(AlgoHandle shot_handle, const char* in_shot_fullname);
    C_API AlgoError C_CALL ShotSave(AlgoHandle shot_handle, const char* in_shot_fullname);


    /************************************
    @brief  : 获取Shot内容
    @note   : 建议浅copy 不涉及内存申请
    @param  : shot_handle   有效的句柄
    @param  : out_shot      导出的结构体,必须已经初始化完毕
    @param  : is_deep_copy  是否深拷贝, 默认浅拷贝  1=深拷贝;0=浅拷贝
    @return	: AlgoError
    @Author : chengjiayi@2021/05/07
    ************************************/
    C_API AlgoError C_CALL ShotGet(AlgoHandle shot_handle, AlgoShot* out_shot,int in_is_deep_copy=0);


    /************************************
    @brief  : 输出shot信息到console
    @note   : 支持输出roi shot信息
    @param  : shot_handle
    @return	: AlgoError
    @Author : chengjiayi@2021/05/10
    ************************************/
    C_API AlgoError C_CALL ShotInfo(AlgoHandle shot_handle);



    /************************************
    @Brief		：	导出shot为图像
    @Note		：	支持.png jpg tiff 等
    @Parameter	: 	shot_handle		--
    @Parameter	: 	in_img_fullname		--
    @Parameter	: 	in_is_color		--是否为彩色图,当没有ShotSetColor时候,输出是三通道内容相同的图像  1=彩色图;0=灰度图
    @Returns	:   AlgoError
    @Author		：	chengjiayi@2021/04/26 
    ************************************/
    C_API AlgoError C_CALL ShotExportImage(AlgoHandle shot_handle, const char* in_img_fullname,int in_is_color=0);

	/************************************
	@Brief		：	把shot按照in_encode_type指定的方式编码为字节流
	@Note		：	
	@Parameter	: 	shot_handle				--有效的shot句柄
	@Parameter	: 	in_encode_type			--图像编码类型,全小写,如".bmp",".dib",".jpeg",".png",".pbm",".pgm",".ppm",".tiff",".tif"等
	@Parameter	: 	out_stream				--数据流输出: out_stream是外面申请的，以in_img的size大小申请=img_w*img_h*3*1
	@Parameter	: 	out_real_size			--数据流真实的size大小
	@Parameter	: 	in_compression_quality	--压缩质量10-100,值越大,质量越高;值越小,压缩越多, 时间可能越长,默认=95; 目前支持png和jpeg
	@Returns	:   AlgoError
	@Author		：	chengjiayi@2024/07/17
	************************************/
	C_API AlgoError C_CALL ShotEncode(AlgoHandle shot_handle, const char * in_encode_type, char * out_stream, size_t * out_real_size, int in_compression_quality=95);


    /************************************
    @brief  : 将当前句柄拷贝
    @note   : 支持roi
    @param  : shot_handle   当前shot句柄
    @param  : out_shot      必须先通过Createhandle创建
    @return	: AlgoError
    @Author : chengjiayi@2021/06/22
    ************************************/
    C_API AlgoError C_CALL ShotCopyTo(AlgoHandle shot_handle, AlgoHandle out_shot);


    


    /************************************
    @brief  : 转换Shot的颜色空间
    @note   : 1. 注意, 转换的结果为16U或者8U,所以颜色空间转换矩阵有一定要求
              2. in_cie_type 目前只支持CIE_XYZ 或 CIE_X 或CIE_Y 或CIE_Z
              当 in_cie_type=CIE_XYZ, 颜色空间 CAM_RGB-->CIE_XYZ,shot数据 三通道16U或者8U
              当 in_cie_type= CIE_X 或CIE_Y 或CIE_Z, 颜色空间 CAM_RGB-->CIE_XYZ,只保留in_cie_type,shot数据 单通道16U或者8U
    @param  : shot_handle           当前shot句柄
    @param  : in_cie_type           颜色分量类型
    @param  : in_colorspace_trans   颜色空间转换矩阵, xyz(3*n)=in_colorspace_trans(3*3)*rgb(3*n)
    @return	: AlgoError
    @Author : chengjiayi@2021/11/05
    ************************************/
    C_API AlgoError C_CALL ShotConvertColorSpace(AlgoHandle shot_handle, CIEType in_cie_type, const AlgoMat* in_colorspace_trans);
    
    /************************************
    @brief  : 获取buffer大小
    @note   : 
    @param  : shot_handle 当前shot句柄
    @param  : out_len     buffer大小
    @return	: AlgoError
    @Author : chengjiayi@2021/07/06
    ************************************/
    C_API AlgoError C_CALL ShotGetBufferSize(AlgoHandle shot_handle, uint64_t* out_len);


    /************************************
    @brief  : 将shot里面的数据导出到buffer
    @note   : 
    @param  : shot_handle   有效的shot句柄
    @param  : out_buffer     已经申请好申请空间
    @param  : in_out_len    *in_out_len外部申请好的空间大小,会用真实大小修改
    @return	: AlgoError
    @Author : chengjiayi@2021/06/29
    ************************************/
    C_API AlgoError C_CALL ShotToBuffer(AlgoHandle shot_handle, char* out_buffer, uint64_t* in_out_len);


    /************************************
    @brief  : 将buffer数据导入到shot
    @note   : 
    @param  : shot_handle   有效的shot句柄
    @param  : in_buffer     
    @param  : in_len        
    @return	: AlgoError
    @Author : chengjiayi@2021/06/29
    ************************************/
    C_API AlgoError C_CALL ShotFromBuffer(AlgoHandle shot_handle,const char* in_buffer, uint64_t in_len);


    /************************************
    @brief  : 将三通道Shot拆分为单通道
    @note   : 1. 针对数码相机彩色相机情况,shot必须为3通道
              2. out_1 out_2 out_3哪个不想输出,置为nullptr即可, 但不能全为nullptr
			  支持roi shot,roi_rect需一致
    @param  : shot_handle  有效的shot句柄
    @param  : out_1     必须先通过Createhandle创建, nullptr=不输出,split为cam_r或者cie_x
    @param  : out_2     必须先通过Createhandle创建, nullptr=不输出,split为cam_g或者cie_y
    @param  : out_3     必须先通过Createhandle创建, nullptr=不输出,split为cam_z或者cie_z
    @return	: AlgoError
    @Author : chengjiayi@2021/11/05
    ************************************/
    C_API AlgoError C_CALL ShotSplit(AlgoHandle shot_handle, AlgoHandle out_1, AlgoHandle out_2, AlgoHandle out_3);
    
    
    /************************************
    @brief    : 将三通道Shot合并成为多通道
    @note     : 目前只支持,
                1. CIE_X/CIE_Y/CIE_Z合并成 CIE_XYZ
                2. CAM_R/CAM_B/CAM_G合并成 CAM_RGB
                3. 元信息以in_g_y的元信息为主
				支持roi shot,roi_rect需一致
    @param    : shot_handle  有效的shot句柄
    @param    : in_rx        cam_r或者cie_x的shot句柄
    @param    : in_g_y       cam_g或者cie_y的shot句柄
    @param    : in_bz        cam_z或者cie_z的shot句柄
    @return   : AlgoError
    @Modified : chengjiayi@2023/01/10
    ************************************/
    C_API AlgoError C_CALL ShotMerge(AlgoHandle shot_handle, AlgoHandle in_r_x, AlgoHandle in_g_y, AlgoHandle in_b_z);

    
    /************************************
    @brief  : 从当前shot中移除背景
    @note   : 支持roi shot,roi_rect需一致
    @param  : shot_handle       当前句柄
    @param  : in_bg_shot_handle 背景shot句柄
    @return	: AlgoError
    @Author : chengjiayi@2021/04/28
    ************************************/
    C_API AlgoError C_CALL ShotRemove(AlgoHandle shot_handle, const AlgoHandle in_bg_shot_handle);


    /************************************
    @Brief		：	将shot数据释放
    @Note		：	
    @Parameter	: 	shot_handle		--shot句柄
    @Parameter	:   in_type 释放类型, 0=释放所有数据;1=释放image数据;2=释放metainfo
    @Returns	:   AlgoError
    @Author		：	chengjiayi@2021/04/26 
    ************************************/
    C_API AlgoError C_CALL ShotRelease(AlgoHandle shot_handle,int in_type=0);

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
    C_API void C_CALL ShotDestroyHandle(AlgoHandle shot_handle);
    







#ifdef __cplusplus
};
#endif // __cplusplus
#endif
