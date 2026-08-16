#ifndef __VIGNETTING_CALIB_HPP__
#define __VIGNETTING_CALIB_HPP__

#include "ci_define.h"
#include "ci_declare.h"
#include "vignetting_calib_data.h"
#include "vignetting_calib_error.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    /************************************
    @brief  : 创建渐晕句柄
    @note   : 
    @param  : out_vig_handle    渐晕句柄
    @param  : in_ratio          下采样倍率,默认16
    @return	: AlgoError
    @Author : chengjiayi@2021/05/10
    ************************************/
    C_API AlgoError C_CALL VignettingCreateHandle(AlgoHandle* out_vig_handle, int in_ratio=16);


	/************************************
   @brief  : 设置分区宽高数量C3200与CC60均采用16个分区
   @note   : 设置w:4,h:4
   @param  : vig_handle 有效的渐晕vig句柄
   @param  : partition_w     区域宽数量
   @param  : partition_h     区域高数量
   @return	: AlgoError
   @Author : qianxiaotong@2024/04/02
   ************************************/
	C_API AlgoError C_CALL VignettingSetPartitionWH(AlgoHandle vig_handle, const int partition_w, const int partition_h);

	/************************************
	@brief  : 均匀场kb标定系数压缩
	@note   :
	@param  : vig_handle        有效的渐晕vig句柄
	@param  : Vecmat_k_in       相同颜色不同光圈不同滤片下k矩阵(顺序按X滤片光圈22、16、11、8、5.6、4、2.8、2，YZ滤片顺序一致进行排列)
	@param  : Vecmat_b_in       相同颜色不同光圈不同滤片下b矩阵
	@param  : num_in            需要压缩的矩阵数量(红16/绿蓝24)
	@return	: AlgoError
	@Author : qianxiaotong@2024/1/21
	************************************/
	C_API AlgoError C_CALL VignettingComprass(AlgoHandle vig_handle, AlgoMat* Vecmat_k_in, AlgoMat* Vecmat_b_in, const int num_in);

	/************************************
	@brief  : 均匀场kb标定
	@note   : k_in_out、b_in_out使用时需要创建
	@param  : vig_handle        有效的渐晕vig句柄
	@param  : shot_x_in         X滤片下饱和度从低到高排列的shot（n个shot做过平均后）
	@param  : shot_y_in         Y滤片下饱和度从低到高排列的shot（n个shot做过平均后）
	@param  : shot_z_in         Z滤片下饱和度从低到高排列的shot（n个shot做过平均后)计算红色支持给nullptr
	@param  : shot_num_in       shot_image_in个数
	@param  : dim_num_in        维度个数红色16、绿蓝24
	@param  : k_in_out          逐点k系数数组，排列顺序(滤片光圈-X22、X16、X11、X8、X5.6、X4、X2.8、X2、
															   Y22、Y16、Y11、Y8、Y5.6、Y4、Y2.8、Y2、
															   Z22、Z16、Z11、Z8、Z5.6、Z4、Z2.8、Z2)
	@param  : b_in_out           逐点b系数数组，排列顺序同k
	@return	: AlgoError
	@Author : qianxiaotong@2023/12/21
	************************************/
	C_API AlgoError C_CALL VignettingCalBrightKB(AlgoHandle vig_handle, AlgoHandle* shot_x_in, AlgoHandle* shot_y_in, AlgoHandle* shot_z_in, const int shot_num_in, AlgoMat* k_in_out, AlgoMat* b_in_out, const int dim_num_in);

	/************************************
	@brief  : 暗噪声计算
	@note   :
	@param  : vig_handle        有效的渐晕vig句柄
	@param  : shot_image_in     X/Y/Z分量若干个shot handle（不同曝光时间下的shot）
	@param  : shot_num_in       shot_image_in个数
	@return	: AlgoError
	@Author : qianxiaotong@2023/12/21
	************************************/
	C_API AlgoError C_CALL VignettingCalDarkNoising(AlgoHandle vig_handle, AlgoHandle* shot_image_in, const int shot_num_in);

	/************************************
	@brief  : 均匀场标定后均匀性计算
	@note   :
	@param  : vig_handle        有效的渐晕vig句柄
	@param  : shot_image_in     输入shot handle
	@param  : value_out         输出均匀性值
	@return	: AlgoError
	@Author : qianxiaotong@2024/10/24
	************************************/
	C_API AlgoError C_CALL VignettingCalibUniformValue(AlgoHandle vig_handle, AlgoHandle in_shot_image, float* out_value);

    /************************************
    @brief  : 进行渐晕标定,使用积分球采集的数据进行
    @note   : shot数据XYZ必须至少有一个不为空, 为空的直接使shot_handle=nullptr即可
    @param  : vig_handle        有效的渐晕vig句柄
    @param  : in_X_shot_handle  有效的shot句柄,X分量
    @param  : in_Y_shot_handle  有效的shot句柄,Y分量
    @param  : in_Z_shot_handle  有效的shot句柄,Z分量
    @return	: AlgoError
    @Author : chengjiayi@2021/05/10
    ************************************/
    C_API AlgoError C_CALL VignettingCalib(AlgoHandle vig_handle, const AlgoHandle in_X_shot_handle, const AlgoHandle in_Y_shot_handle, const AlgoHandle in_Z_shot_handle);

	/************************************
    @brief  : 进行渐晕标定,使用积分球采集的数据进行---每个shot是多张shot求均值之后的结果(调用shot模块中的ShotAverage)
    @note   : shot数据XYZ必须至少有一个不为空，生成的vig系数大小与shot.image的图像分辨率一致
    @param  : vig_handle          有效的渐晕vig句柄
    @param  : in_X_shot_handle  有效的shot句柄,X分量
    @param  : in_Y_shot_handle  有效的shot句柄,Y分量
    @param  : in_Z_shot_handle  有效的shot句柄,Z分量
    @return	: AlgoError
    @Author : zhangxuan@2023/08/09
    ************************************/
	C_API AlgoError C_CALL  VignettingCalibUniform(AlgoHandle vig_handle, const AlgoHandle in_X_shot_handle, const AlgoHandle in_Y_shot_handle, const AlgoHandle in_Z_shot_handle);

	/************************************
    @brief  : 进行渐晕标定,使用数学模型
    @note   : 目前仅支持c1200 c3200
    @param  : vig_handle    有效的渐晕vig句柄
    @param  : in_device     设备信息
    @param  : in_color      颜色信息
    @return	: AlgoError
    @Author : chengjiayi@2021/05/10
    ************************************/
    C_API AlgoError C_CALL VignettingCalibCos(AlgoHandle vig_handle, const Device* in_device, AlgoColor in_color);

    /************************************
    @brief  : 读取保存渐晕标定文件
    @note   : 后缀只能是.vig
    @param  : vig_handle        有效的渐晕vig句柄
    @param  : in_vig_fullname   标定文件,后缀只能是.vig
    @return	: AlgoError
    @Author : chengjiayi@2021/05/10
    ************************************/
    C_API AlgoError C_CALL VignettingSave(AlgoHandle vig_handle, const char* in_vig_fullname);
    C_API AlgoError C_CALL VignettingLoad(AlgoHandle vig_handle, const char* in_vig_fullname);


	/************************************
	@brief  : 将原始的vig转化为roi的vig。后可直接用于修正roi shot。roi vig仅支持Rectify。
	@note   : 如果vig是下采样的，那么该步骤会进行上采样，从而节省Rectify的时间。
			  支持对下采样的vig以及非下采样的vig进行处理。
			  2024.0628：支持对均匀场标定的vig进行roivig生成
	@param  : vig_handle        有效的渐晕vig句柄
	@param  : in_vig_fullname   标定文件,后缀只能是.vig
	@return	: AlgoError
	@Author : tianbo@2023/11/08
	************************************/
	C_API AlgoError C_CALL VignettingToRoiRectVig(AlgoHandle vig_handle, const AlgoRect in_roi_rect);

	/************************************
	@brief  : 保存C3200标定的XYZ滤片txt文件
	@note   : in_x_filename in_y_filename in_z_filename均不能为空
	@param  : vig_handle        有效的渐晕vig句柄
	@param  : in_x_filename     x分量文件存储路径，后缀必须为.txt
	@param  : in_y_filename     y分量文件存储路径，后缀必须为.txt
	@param  : in_z_filename     z分量文件存储路径，后缀必须为.txt
	@return	: AlgoError
	@Author : zhongao@2025/05/24
	************************************/
	C_API AlgoError C_CALL VignettingSaveTxt(AlgoHandle vig_handle, const char* in_x_filename, const char* in_y_filename, const char* in_z_filename);


    /************************************
    @brief  : 读取C3200标定的XYZ滤片txt文件
    @note   : in_x_filename in_y_filename in_z_filename不能同时为空
    @param  : vig_handle        有效的渐晕vig句柄
    @param  : in_device         设备信息
    @param  : in_color          颜色信息
    @param  : in_x_filename     x分量文件, 可以为空="",详见demo
    @param  : in_y_filename     y分量文件, 可以为空="",详见demo
    @param  : in_z_filename     z分量文件, 可以为空="",详见demo
    @return	: AlgoError
    @Author : chengjiayi@2021/09/13
    ************************************/
    C_API AlgoError C_CALL VignettingLoadV1Txt(AlgoHandle vig_handle, const Device* in_device, AlgoColor in_color, const char* in_x_filename, const char* in_y_filename, const char* in_z_filename);

	/************************************
	@brief  : 高斯模糊参数设置
	@note   :
	@param  : vig_handle    有效的渐晕vig句柄
	@param  : in_filter_width      -- 高斯滤波器宽度
    @param  : in_filter_height     -- 高斯滤波器高度
    @param  : in_sigma_x           -- 水平方向标准差
    @param  : in_sigma_y           -- 垂直方向标准差
	@return	: AlgoError
	@Author : zhangxuan@2023/06/19
	************************************/
	C_API AlgoError C_CALL  VignettingSetGauBlurPara(AlgoHandle vig_handle, const int in_filter_width, const int in_filter_height, const float in_sigma_x, const float in_sigma_y);

    /************************************
    @brief  : 渐晕修正
    @note   : 支持对roi vig的修正，当输入shot为黑色时，根据输入的vig文件进行修正，不进行颜色判别
			: 请勿输入in_roi参数。
    @param  : vig_handle    有效的渐晕vig句柄
    @param  : in_out_shot   待修正的shot
    @param  : in_bg_shot    输入背景shot, 无背景时候置为nullptr
    @param  : in_roi        输入roi区域，必须与in_out_shot同尺寸
    @return	: AlgoError
    @Author : chengjiayi@2021/05/10
    ************************************/
    C_API AlgoError C_CALL VignettingRectify(AlgoHandle vig_handle, AlgoHandle in_out_shot, const AlgoHandle in_bg_shot = nullptr, AlgoRect in_roi = { -1, -1, -1, -1 });

	/************************************
	@brief  : 针对均匀场标定的vig文件，支持将压缩的文件进行解压。支持只存储单个滤片的，默认XYZ全部存储。配合VignettingSetSingleKbVigParam接口使用。
	@note   : 
	@param  : vig_handle    有效的渐晕vig句柄
	@return	: AlgoError
	@Author : tianbo@2024/06/28
	************************************/
	C_API AlgoError C_CALL VignettingToNonCompress(AlgoHandle vig_handle);

	/************************************
   @brief  : 渐晕修正(手动输入sensor拍摄图像尺寸)
   @note   : 支持对roi vig的修正，当输入shot为黑色时，根据输入的vig文件进行修正，不进行颜色判别
   @param  : vig_handle    有效的渐晕vig句柄
   @param  : in_out_shot   待修正的shot
   @param  : in_image_size 输入图像尺寸
   @param  : in_bg_shot    输入背景shot, 无背景时候置为nullptr
   @param  : in_roi        输入roi区域，必须与in_out_shot同尺寸
   @return	: AlgoError
   @Author : qianiaotong@2023/11/21
   ************************************/
	C_API AlgoError C_CALL VignettingRectifyOld(AlgoHandle vig_handle, AlgoHandle in_out_shot, const AlgoSize in_image_size,const AlgoHandle in_bg_shot = nullptr, AlgoRect in_roi = { -1, -1, -1, -1 });

	/************************************
	@brief  : 渐晕修正:标定文件是采集了多张图像合并取均值得到的，尺寸与原始图像大小相同，目前只适用于20110039相机
	@note   :，当输入shot为黑色时，根据输入的vig文件进行修正，不进行颜色判别
	@param  : vig_handle    有效的渐晕vig句柄:这里做了均匀场标定。vig的分辨率与图像分辨率必须相同
	@param  : in_out_shot   待修正的shot
	@param  : in_bg_shot    输入背景shot, 无背景时候置为nullptr,或者shot减去背景后不用输入
	@return	: AlgoError
	@Author : zhangxuan@2023/07/18
	************************************/
	C_API AlgoError C_CALL VignettingRectifyMean(AlgoHandle vig_handle, AlgoHandle in_out_shot, const AlgoHandle in_bg_shot = nullptr);

    //tools
    //C_API AlgoError C_CALL VignettingSet(AlgoHandle vig_handle, const AlgoVignetting* in_vig);//deep copy


    /************************************
    @brief  : 获取buffer大小
    @note   :
    @param  : vig_handle  当前有效句柄
    @param  : out_len     buffer大小
    @return	: AlgoError
    @Author : chengjiayi@2021/07/06
    ************************************/
    C_API AlgoError C_CALL VignettingGetBufferSize(AlgoHandle vig_handle, uint64_t* out_len);


    /************************************
    @brief  : 将vig里面的数据导出到buffer
    @note   :
    @param  : vig_handle        有效的vig句柄
    @param  : out_buffer        已经申请好申请空间
    @param  : in_out_len        *in_out_len外部申请好的空间大小,会用真实大小修改
    @return	: AlgoError
    @Author : ouxingtao@2021/06/30
    ************************************/
    C_API AlgoError C_CALL VignettingToBuffer(AlgoHandle vig_handle, char* out_buffer, uint64_t* in_out_len);


    /************************************
    @brief  : 将buffer数据导入到vig
    @note   :
    @param  : vig_handle   有效的vig句柄
    @param  : in_buffer
    @param  : in_len
    @return	: AlgoError
    @Author : ouxingtao@2021/06/30
    ************************************/
    C_API AlgoError C_CALL VignettingFromBuffer(AlgoHandle vig_handle, const char* in_buffer, uint64_t in_len);




    /************************************
    @brief  : 获取渐晕标定数据
    @note   : 注意浅拷贝
    @param  : vig_handle    有效的渐晕vig句柄
    @param  : out_vig       导出的结构体,必须已经初始化完毕
    @param  : is_deep_copy  是否深拷贝, 默认浅拷贝  1=深拷贝;0=浅拷贝
    @return	: AlgoError
    @Author : chengjiayi@2021/05/10
    ************************************/
    C_API AlgoError C_CALL VignettingGet(AlgoHandle vig_handle, AlgoVignetting* out_vig,int in_is_deep_copy = 0);



    /************************************
    @brief  : 设置渐晕模块数据
    @note   : 
    @param  : vig_handle 有效的渐晕vig句柄
    @param  : in_vig     数据结构体
    @return	: AlgoError
    @Author : chengjiayi@2021/09/13
    ************************************/
    C_API AlgoError C_CALL VignettingSet(AlgoHandle vig_handle, const AlgoVignetting* in_vig);



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
    C_API AlgoError C_CALL VignettingRelease(AlgoHandle vig_handle);
    C_API void C_CALL VignettingDestroyHandle(AlgoHandle vig_handle);


	/************************************
	@brief  : 设置KBvig的基本参数信息：光圈、cietype等
	@note   : 这个是用于解压特定滤片特定光圈时，所使用的参数，主要用于提速，配合生成新的单一滤片与光圈的KB-vig使用；
			: 请勿输入in_roi参数。
	@param  : vig_handle				-- 有效的渐晕vig句柄
	@param  : in_type					-- 需要合成的白色vig的CIEType/解压RGBvig时需要保存的滤片，可取CIE_X、CIE_Y、CIE_Z、CIE_XYZ，当为CIE_X、CIE_Y、CIE_Z时，仅解压缩对应滤片的kb数据，节省空间和时间；
	@param  : in_capture				-- 采集信息结构体，其中aperture是光圈，取值为2200、1600、1100、800、560、400、280、200；
	@param  : in_roi					-- kb的roi，需要与shot对应；	
	@return : AlgoError
	@Author : GuoHao@2024/06/11
	************************************/
	C_API AlgoError C_CALL VignettingSetSingleKbVigParam(AlgoHandle vig_handle, CIEType in_type, Capture in_capture, AlgoRect in_roi = { 0, 0, 0, 0 });


	/************************************   
	@brief  : 使用RGB的光路修正文件合成白色光路修正文件    
	@note   : 有两种方案：     
				a:根据光枪测量的 RGB 与 W 值，计算白色中的RGB配比，合成白色光路修正文件(注意，此方案要求输入的单色vig句柄必须是解压前的！算法内部解压):     
				b:不需要光枪测量，分别将R_X、G_Y、B_Z作为W_X、W_Y、W_Z，合成白色光路修正文件(注意，此方案要求输入的单色vig句柄必须是解压后的！需要在使用此接口前手动解压);              
			      在不需要光枪测量的方案中，vig数量必须是3个，vig顺序必须是R、G、B；此时入参rgbw_xyz_in全为0即可；   
	@param  : vig_handle    -- 有效的渐晕vig句柄
	@param  : vig_rgb_in	-- 输入 RGB 光路修正有效句柄
	@param  : n				-- vig文件个数
	@param  : rgbw_xyz_in   -- RGBW原始值，rgbw_xyz_in全为0时默认为不需要光枪测量的方案；
	@return : AlgoError
	@Author : YangShangqi、GuoHao@2024/06/11
	************************************/
	C_API AlgoError C_CALL VignettingRgbMergeW(AlgoHandle vig_handle, AlgoHandle *vig_rgb_in, int n, RGBWXYZ rgbw_xyz_in);

	/************************************
	@brief  : 根据传入的光圈与滤片类型解压对应的kb矩阵
	@note   :
	@param  : vig_handle    -- 有效的渐晕vig句柄
	@param  : in_capture	-- 输入的光圈类型
	@param  : in_type		-- 输入的滤片类型
	@param  : in_roi		-- 输入的roi区域，可以获取对应区域的kb系数矩阵
	@param  : out_kk		-- 输出的k系数矩阵
	@param  : out_bb		-- 输出的b系数矩阵
	@return : AlgoError
	@Author : ZhongAo@2024/12/16
	************************************/
	C_API AlgoError C_CALL VignettingGetKB(AlgoHandle vig_handle, Capture in_capture, CIEType in_type, AlgoMat* out_kk, AlgoMat* out_bb, AlgoRect in_roi = { 0, 0, 0, 0 });


#ifdef __cplusplus
}
#endif // __cplusplus
#endif