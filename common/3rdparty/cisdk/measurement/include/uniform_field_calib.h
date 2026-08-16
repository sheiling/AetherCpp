#ifndef __UNIFORM_FIELD_CALIB_H__
#define __UNIFORM_FIELD_CALIB_H__

#include "ci_define.h"
#include "ci_declare.h"
#include "uniform_field_calib_data.h"
#include "uniform_field_calib_error.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    /************************************
    @brief  : 创建均匀场标定句柄
    @note   : 
    @param  : in_out_uniform_handle    均匀场标定句柄
    @return	: AlgoError
    @Author : qianxiaotong@2024/06/26
    ************************************/
	C_API AlgoError C_CALL UniformFieldCreateHandle(AlgoHandle* in_out_uniform_handle);

	/************************************
	@brief  : 暗噪声采图异常数据上下边界设置
	@note   : 暗噪声系数计算前会对输入的shot图像计算全图均值并进行异常值判断，该接口用于设定异常值判断的范围
			  需要保证输入的shot全图均值， in_bottom_val < shot_mean < in_top_val 
	@param  : uniform_handle    均匀场标定句柄
	@param  : in_bottom_val     异常判断的下边界值，默认为245.f
	@param  : in_top_val		异常判断的上边界值，默认为275.f
	@return	: AlgoError
	@Author : zhongao@2024/12/27
	************************************/
	C_API AlgoError C_CALL UniformFieldSetDefaultDarkAbnormalThresh(AlgoHandle uniform_handle, float in_bottom_val = 245.f, float in_top_val = 275.f);

	/************************************
	@brief  : 暗噪声计算
	@note   : 暗噪声标定，每次标定一个滤片(需调用3次分别标定X/Y/Z滤片的暗噪声)
	@param  : uniform_handle    均匀场标定句柄
	@param  : in_dark_shot      X/Y/Z分量若干个shot handle
	                           （不同曝光时间下的shot：曝光时间由50-5050ms，每隔250ms连续采集四幅图）共21组
	@param  : in_shot_num       in_dark_shot个数
	@return	: AlgoError
	@Author : qianxiaotong@2024/06/26
	************************************/
	C_API AlgoError C_CALL UniformFieldCalibDarkNoising(AlgoHandle uniform_handle, AlgoHandle* in_dark_shot, int in_shot_num);

	/************************************
	@brief  : 相机均匀场kb标定
	@note   : 
	@param  : uniform_handle    均匀场标定句柄
	@param  : in_avg_shot		某滤片下饱和度从低到高排列的shot数组（该shot为n个shot求平均之后的shot）
	@param  : in_shot_num       in_shot的个数
	@return	: AlgoError
	@Author : zhongao@2024/12/27
	************************************/
	C_API AlgoError C_CALL UniformFieldCalib(AlgoHandle uniform_handle, AlgoHandle* in_avg_shot, int in_shot_num);
	
	/************************************
	@brief  : 标定完当前滤片后导出KB系数并落盘
	@note   :
	@param  : uniform_handle    均匀场标定句柄
	@param  : in_save_path		KB系数的文件保存路径，后缀必须是.amat
	@return	: AlgoError
	@Author : zhongao@2024/12/27
	************************************/
	C_API AlgoError C_CALL UniformFieldExportKB(AlgoHandle uniform_handle, const char* in_save_path);

	/************************************
	@brief  : kb系数压缩
	@note   : 
	@param  : uniform_handle    均匀场标定句柄
	@param  : in_root_path      存放kb系数的文件夹目录
	@param  : in_aperture       光圈文件夹名称列表，从"22"->"2"从大到小排列
	@param  : in_aperture_num   光圈个数
	@param  : in_color          输入压缩时对应的颜色
	@param  : in_wh             分区大小，默认为{4，4}
	@return	: AlgoError
	@Author : zhongao@2024/12/27
	************************************/
	C_API AlgoError C_CALL UniformFieldCompress(AlgoHandle uniform_handle, const char* in_root_path, char* in_aperture[], int in_aperture_num, AlgoColor in_color, AlgoSize in_wh = { 4,4 });

	/************************************
	@brief  : 均匀场修正
	@note   : 支持压缩系数文件修正、解压缩后系数文件修正
	@param  : uniform_handle    均匀场标定句柄
	@param  : in_out_shot       需修正的shot
	@return	: AlgoError
	@Author : qianxiaotong@2024/06/26
	************************************/
	C_API AlgoError C_CALL UniformFieldRectify(AlgoHandle uniform_handle, AlgoHandle in_out_shot);

	/************************************
	@brief  : 根据光圈解压XYZ滤片的kb系数
	@note   : 支持转化为只包含特定滤片的decompress_uniform
	@param  : uniform_handle			均匀场标定句柄
	@param  : in_aperture				光圈信息
	@param  : in_decompress_filter		需要解压的滤片信息，默认为XYZ都解压，解压X为CIE_X，解压XY为CIE_X|CIE_Y，解压XYZ为CIE_X|CIE_Y|CIE_Z以此类推
	@return	: AlgoError
	@Author : zhongao@2025/04/07
	************************************/
	C_API AlgoError C_CALL UniformFieldDeCompress(AlgoHandle uniform_handle, int in_aperture, int in_decompress_filter = CIE_X | CIE_Y | CIE_Z);

	/************************************
	@brief  : 根据RGB三色vig进行白色vig压缩
	@note   : 1）接口原理：在应用阶段将rgb三色vig解压后再压缩成白色vig，该压缩方式是有损压缩，属于老相机获取白色vig的补丁接口。
			  新标定相机获取白色vig建议在标定阶段调用UniformFieldCompress接口直接获取白色vig
			  2）白色vig是根据主分量的方式进行压缩的，暂不支持根据RGB配比进行压缩
	@param  : uniform_handle      均匀场标定句柄
	@param  : in_r_uniform		  红色R对应光圈下载入vig后的均匀场句柄
	@param  : in_g_uniform		  绿色G对应光圈下载入vig后的均匀场句柄
	@param  : in_b_uniform		  蓝色B对应光圈下载入vig后的均匀场句柄
	@return	: AlgoError
	@Author : zhongao@2025/01/08
	************************************/
	C_API AlgoError C_CALL UniformFieldRgbCompressW(AlgoHandle uniform_handle, AlgoHandle in_r_uniform, AlgoHandle in_g_uniform, AlgoHandle in_b_uniform);

	/************************************
	@brief  : 合成白色vig
	@note   : 将rgb的decompress_uniform合成w色。支持转化为只包含特定滤片的
	支持两种合成白色方式：1.in_rgbw_xyz为0使用RX,GY,BZ方式合成W(主分量合成)，2.不为0计算比例合成W（配比白色系数生成）
	@param  : uniform_handle      均匀场标定句柄
	@param  : in_r_uniform		  红色R对应光圈下解压后的均匀场句柄
	@param  : in_g_uniform		  绿色G对应光圈下解压后的均匀场句柄
	@param  : in_b_uniform		  蓝色B对应光圈下解压后的均匀场句柄
	@param  : in_rgbw_xyz         相机采集的RGBW_xyz图像经过曝光时间归一化后，计算得到的图像中心区域的均值封装后的结果(不输入或输入值全为0时为方式1合成W)
									即in_rgbw_xyz.R.X=RX_exp_mean, in_rgbw_xyz.R.Y=RY_exp_mean，依此类推
	@return	: AlgoError
	@Author : zhongao@2025/07/17
	************************************/
	C_API AlgoError C_CALL UniformFieldRgbMergeW(AlgoHandle uniform_handle, AlgoHandle in_r_uniform, AlgoHandle in_g_uniform, AlgoHandle in_b_uniform, RGBWXYZ in_rgbw_xyz = {});

	/************************************
	@brief  : 将同颜色多个解压后的单滤片均匀场句柄拼接为多滤片均匀场句柄
	@note   : 
	@param  : in_xyz_uniform      输入的xyz均匀场句柄数组
								  1、要求每个句柄为解压后的句柄
								  2、要求每个句柄中仅存在单个滤片数据，如x滤片的均匀场句柄中只能存在x滤片的相关数据，不能存在yz滤片的相关数据
								  3、对XYZ的先后顺序无要求
								  4、可以支持传入1/2/3个单滤片句柄拼接
	@param  : in_xyz_num		  输入的xyz均匀场句柄数组的长度
	@param  : out_uniform		  拼接后的的多滤片均匀场句柄
								  1、该句柄只要不为空即可，不要求载入数据
	@return	: AlgoError
	@Author : zhongao@2026/02/04
	************************************/
	C_API AlgoError C_CALL UniformFieldConcatNonCompress(AlgoHandle* in_xyz_uniform, int in_xyz_num, AlgoHandle out_uniform);

	/************************************
	@brief  : 将全尺寸的decompress_uniform变更为roi的decompress_uniform
	@note   : 仅对decompress_uniform生效，支持对非全尺寸的decompress_uniform再做ROI
	@param  : uniform_handle    均匀场标定句柄
	@param  : in_rect           ROI位置尺寸
	@return	: AlgoError
	@Author : qianxiaotong@2024/06/26
	************************************/
	C_API AlgoError C_CALL UniformFieldCropRoi(AlgoHandle uniform_handle, AlgoRect in_rect);

	/************************************
	@brief  : 获取均匀场的device信息
	@note   :
	@param  : uniform_handle			均匀场标定句柄
	@param  : out_device				导出的device结构体
	@return	: AlgoError
	@Author : zhongao@2025/11/14
	************************************/
	C_API AlgoError C_CALL UniformFieldGetDevice(AlgoHandle uniform_handle, Device* out_device);

	/************************************
	@brief  : 将uniform_handle导出为图像
	@note   : aperture仅在handle为compress时生效
	@param  : uniform_handle			均匀场标定句柄
	@param  : in_img_fullname			导出图像地址
	@param  : in_export_filter			滤片信息
	@param  : in_aperture				光圈信息
	@param  : in_type					0为热力图，1为灰度图
	@param  : in_abnormalpoint_ratio	收缩比例默认为0
	@return	: AlgoError
	@Author : qianxiaotong@2024/06/26
	************************************/
	C_API AlgoError C_CALL UniformFieldExport(AlgoHandle uniform_handle, const char* in_img_fullname, CIEType in_export_filter, int in_aperture, int in_type=0, float in_abnormalpoint_ratio=0.0f);

    /************************************
    @brief  : 读取保存CC60均匀场标定文件
    @note   : Save要求文件后缀名与句柄类型保持一致：①.vig——解压前的全光圈数据（UniformFieldType_COMPRESS）
													②.vigtdc——解压后的单光圈数据（UniformFieldType_NON_COMPRESS）
													③.darkvigx .darkvigy .darkvigz——标定时的临时数据（UniformFieldType_TEMP）
				Load支持CC60相机所有的.vig .vigtdc .darkvigx .darkvigy .darkvigz文件载入
    @param  : uniform_handle			有效的均匀场标定句柄
    @param  : in_uniform_fullname       标定文件路径
    @return	: AlgoError
    @Author : zhongao@2025/07/22
    ************************************/
    C_API AlgoError C_CALL UniformFieldSave(AlgoHandle uniform_handle, const char* in_uniform_fullname);
    C_API AlgoError C_CALL UniformFieldLoad(AlgoHandle uniform_handle, const char* in_uniform_fullname);

    /************************************
    @brief  : 获取buffer大小
    @note   :
    @param  : uniform_handle	当前有效句柄
    @param  : out_len			buffer大小
    @return	: AlgoError
    @Author : qianxiaotong@2024/06/26
    ************************************/
    C_API AlgoError C_CALL UniformFieldGetBufferSize(AlgoHandle uniform_handle, uint64_t* out_len);

    /************************************
    @brief  : 将uniform里面的数据导出到buffer
    @note   :
    @param  : uniform_handle        有效的uniform句柄
    @param  : out_buffer            已经申请好申请空间
    @param  : in_out_len            *in_out_len外部申请好的空间大小,会用真实大小修改
    @return	: AlgoError
    @Author : qianxiaotong@2024/06/26
    ************************************/
    C_API AlgoError C_CALL UniformFieldToBuffer(AlgoHandle uniform_handle, char* out_buffer, uint64_t* in_out_len);

    /************************************
    @brief  : 将buffer数据导入到uniform
    @note   :
    @param  : uniform_handle   有效的uniform句柄
    @param  : in_buffer
    @param  : in_len
    @return	: AlgoError
    @Author : qianxiaotong@2024/06/26
    ************************************/
    C_API AlgoError C_CALL UniformFieldFromBuffer(AlgoHandle uniform_handle, const char* in_buffer, uint64_t in_len);

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
    C_API AlgoError C_CALL UniformFieldRelease(AlgoHandle uniform_handle);
    C_API void C_CALL UniformFieldDestroyHandle(AlgoHandle uniform_handle);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif