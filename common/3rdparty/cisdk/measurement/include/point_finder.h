#ifndef __POINT_FINDER_H__
#define __POINT_FINDER_H__


#include "ci_define.h"
#include "ci_declare.h"
#include "point_finder_data.h"
#include "point_finder_errors.h"
#include "roi_detector.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    ///************************************
    //@brief  : 初始化点定位句柄
    //@note   : 
    //@param  : 
    //@return	: AlgoHandle
    //@Author : zhongao@2024/09/19
    //************************************/
    //C_API AlgoHandle C_CALL PointFindCreateHandle();

	/************************************
	@brief  : 初始化句柄
	@note   :
	@param  : in_out_point_find_handle
	@return	: AlgoError
	@Author : huangziming@2024/12/12
	************************************/
	C_API AlgoError C_CALL PointFindCreateHandle(AlgoHandle* in_out_point_find_handle);

    /************************************
    @brief  : 设置点定位CPU并行数，仅对TM_NCC有效
    @note   : 不调用此函数, 模块默认 = CPU线程数-1
    @param  : handle			--有效句柄
    @param  : in_omp_num		--并行运算线程个数
    @return	: AlgoError
    @Author : zhongao@2024/09/18
    ************************************/
    C_API AlgoError C_CALL PointFindSetDefaultThreadsNum(AlgoHandle handle, int in_omp_num);

    /************************************
    @brief  : 设置模板匹配的匹配核
    @note   : 不调用此函数, 匹配核默认=TM_CCOEFF
    @param  : handle			--有效句柄
    @param  : in_match_flag     --模板匹配核
    @return	: AlgoError
    @Author : zhongao@2024/09/19
    ************************************/
    C_API AlgoError C_CALL PointFindSetDefaultMatchFlag(AlgoHandle handle, PointFindMatchFlg in_match_flag);

    /************************************
    @brief  : 设置点定位下采样参数
    @note   : 不调用时候=默认无下采样
    @param  : handle			--有效句柄
    @param  : in_downsampling   --下采样倍率,0-1之间, 值越小越快, 精度越差
    @return : AlgoError
    @Author : chengjiayi@2024/07/26
    ************************************/
    C_API AlgoError C_CALL PointFindSetDefaultDownsampling(AlgoHandle handle, float in_downsampling);

    /************************************
    @brief  : 设置亚像素定位
    @note   : 不调用时候=默认1
    @param  : handle		--有效句柄
    @param  : in_flag		--设置亚像素定位flg: 0=关闭,1=开启,使用图像阶距地方法, 默认1
    @return : AlgoError
    @Author : zhongao@2024/09/19
    ************************************/
    C_API AlgoError C_CALL PointFindSetDefaultSubpixelFlag(AlgoHandle handle, int in_flag);

	/************************************
	@brief  : 设置目标定位方法
	@note   : 直接灰度分割速度显著提升，仅适用于背景灰度值小于灯点区域灰度均值10%以下的场景
			  图像腐蚀+圆形均值模板，目前为模糊业务场景提供定位，未进行完备性测试
	@param  : handle		--有效句柄
	@param  : in_method		--设置目标定位方法：0=模板匹配,1=直接灰度分割, 默认0
	@return : AlgoError
	@Author : zhongao@2025/03/24
	************************************/
	C_API AlgoError C_CALL PointFindSetDefaultMethod(AlgoHandle handle, int in_method);

	/************************************
	@brief  : 设置模板生成的参数
	@note   : 目前可调参数有：①模板平滑度 ②模板中心凹陷面积 ③模板中心凹陷程度 ④去均值选择 ⑤方形圆形模板选择
	@param  : handle		--有效句柄
	@param  : in_param		--设置模板设定参数，参数含义及调节范围详见point_finder_data中TemplateParam定义
	@return : AlgoError
	@Author : zhongao@2025/03/24
	************************************/
	C_API AlgoError C_CALL PointFindSetDefaultTemplateParam(AlgoHandle handle, const TemplateParam& in_param);

	/************************************
	@brief  : 设置后处理参数
	@note   : 目前可调参数有：①二值化分割阈值比例 ②灰度图干扰点滤除上界阈值 ③灰度图干扰点滤除下界阈值
	@param  : handle		--有效句柄
	@param  : in_param		--设置后处理参数，参数含义及调节范围详见point_finder_data中PostProcParam定义
	@return : AlgoError
	@Author : zhongao@2025/03/24
	************************************/
	C_API AlgoError C_CALL PointFindSetDefaultPostProcParam(AlgoHandle handle, const PostProcParam& in_param);

	/************************************
	@brief  : 手动设置模板
	@note   : 主要针对非灯点,此处输入的是下采样前的, 如果设置下采样了, 那么算法内部自动把模板下采样
	@param  : handle		--有效句柄
	@param  : in_mat_tmpl	--模版图像
	@return	: AlgoError
	@Author : zhongao@2024/09/19
	************************************/
	C_API AlgoError C_CALL PointFindSetTemplate(AlgoHandle handle, const AlgoMat* in_mat_tmpl);

    /************************************
    @brief  : 手动设置模板宽高
    @note   : 主要针对灯点, 此处输入的是下采样前的, 如果设置下采样了, 那么算法内部自动把模板下采样
    @param  : handle			--有效句柄
    @param  : in_templ_w		--灯点模版的宽
    @param  : in_templ_h		--灯点模版的高
    @return	: AlgoError
    @Author : zhongao@2024/09/19
    ************************************/
    C_API AlgoError C_CALL PointFindSetTemplateWh(AlgoHandle handle, int in_templ_w, int in_templ_h);

	/************************************
	@brief  : 手动设置灯点坐标, 用于补全点定位定位不完整或者加载文件缺失灯点
	@note   : 
	@param  : handle			    --有效句柄
	@param  : in_manual_points		--待补全灯点坐标集合
	@param  : in_num                --待补全灯点坐标集合长度, 必须保持和in_manual_points一致
	@return	: AlgoError
	@Author : huangziming@2024/11/29
	************************************/
	C_API AlgoError C_CALL PointFindSetManualPoint(AlgoHandle handle, const AlgoPoint2f* in_manual_points, const int* in_num);

    /************************************
    @Brief		：	输入为shot与roi的目标定位
    @Note		：	亚像素起始坐标为(0,0)
    @Parameter	: 	handle			--有效句柄
    @Parameter	: 	in_shot         --输入shot句柄
    @Parameter	: 	in_region       --输入roi句柄
    @Returns	:   AlgoError
    @Author		：	zhongao@2024/09/19
    ************************************/
	C_API AlgoError C_CALL PointFindLocate(AlgoHandle handle, const AlgoHandle in_shot, const AlgoHandle in_region);

	/************************************
	@Brief		：	输入为mat与rect的目标定位
	@Note		：  亚像素起始坐标为(0,0)
	@Parameter	: 	handle			--有效句柄
	@Parameter	: 	in_mat			--输入mat图像
	@Parameter	: 	in_rect			--输入rect区域
	@Returns	:   AlgoError
	@Author		：	zhongao@2024/09/19
	************************************/
	C_API AlgoError C_CALL PointFindLocateInRect(AlgoHandle handle, const AlgoMat* in_mat, const AlgoRect in_rect);

	/************************************
	@Brief		：	输入为mat与mask的目标定位
	@Note		：	in_mask与in_mat的宽高必须一致,亚像素起始坐标为(0,0)
	@Parameter	: 	handle			--有效句柄
	@Parameter	: 	in_mat			--输入mat图像
	@Parameter	: 	in_mask			--输入mask区域
	@Returns	:   AlgoError
	@Author		：	zhongao@2024/09/19
	************************************/
	C_API AlgoError C_CALL PointFindLocateInMask(AlgoHandle handle, const AlgoMat* in_mat, const AlgoMat* in_mask);

	/************************************
	@brief  : 逐一添加数据,将不同隔点pattern点定位结果组合完整
	@note   : 仅针对同一采集区域内不同隔点位置的点定位结果进行融合
	@param  : handle                        --有效的组合后的point_find句柄
	@param  : in_point_find_handle          --有效的point_find句柄
	@return	: AlgoError
	@Author : huangziming@2024/11/13
	************************************/
	C_API AlgoError C_CALL PointFindAdd(AlgoHandle handle, AlgoHandle in_point_find_handle);


    /************************************
    @Brief		：	载入和保存.pts定位数据
    @Note		：	文件名后缀必须是.pts
    @Parameter	: 	handle				--有效句柄
    @Parameter	: 	in_pts_fullname		--文件名后缀必须是.pts
    @Returns	:   AlgoError
    @Author		：	chengjiayi@2024/07/26
    ************************************/
    C_API AlgoError C_CALL PointFindLoad(AlgoHandle handle, const char* in_pts_fullname);
    C_API AlgoError C_CALL PointFindSave(AlgoHandle handle, const char* in_pts_fullname); 

    /************************************
    @brief  : 获取定位结果
    @note   :
    @param  : handle	--有效句柄
    @param  : out_pts   --点定位数组, 必须提前初始化, 初始化大小=2*灯点行数*灯点列数
    @param  : out_num   --最终的点定位数组长度
    @return	: AlgoError
    @Author : chengjiayi@2024/07/26
    ************************************/
    C_API AlgoError C_CALL PointFindGet(AlgoHandle handle, AlgoPoint2f* out_pts, uint64_t* out_num);

    /************************************
    @brief  : 将句柄数据release，并载入in_pts数据
    @note   :
    @param  : handle   --有效句柄
    @param  : in_pts   --点定位数组
    @param  : in_num   --最终的点定位数组长度，必须与in_pts长度保持一致
    @return	: AlgoError
    @Author : chengjiayi@2024/07/26
    ************************************/
    C_API AlgoError C_CALL PointFindSet(AlgoHandle handle, const AlgoPoint2f* in_pts, uint64_t in_num);

    /************************************
    @brief  : 画出定位图像
    @note   : 
    @param  : handle		--有效句柄
	@param  : in_fullname	--图像保存路径
    @param  : in_drawon		--将点画在此图像上, 默认为空时, 使用模块内部图像
    @return	: AlgoError
    @Author : chengjiayi@2024/07/26
    ************************************/
    C_API AlgoError C_CALL PointFindExportLocImg(AlgoHandle handle, const char* in_fullname, const AlgoMat* in_drawon=nullptr);

    /************************************
    @brief  : 画出亚像素定位图像
    @note   : 三通道图像中心点为白色，单通道图像中心点为红色
			  此接口绘制的亚像素灯点坐标的起始位置为(0.5, 0.5)
    @param  : handle		--有效句柄
    @param  : in_fullname	--图像保存路径
    @param  : in_multi		--放大倍率
    @param  : in_is_split	--是否拆分, 解决图像过大的情况 1=拆分,0=不拆分
    @param  : in_drawon		--将点画在此图像上, 默认为空时, 使用模块内部图像
    @return	: AlgoError
    @Author : zhongao@2024/09/19
    ************************************/
    C_API AlgoError C_CALL PointFindExportSubpixelLocImg(AlgoHandle handle, const char* in_fullname, int in_multi=3, int in_is_split=1, const AlgoMat* in_drawon = nullptr);

    /************************************
    @brief  : 获取buffer大小
    @note   :
    @param  : handle		--有效句柄
    @param  : out_len		--buffer大小
    @return	: AlgoError
    @Author : chengjiayi@2024/07/26
    ************************************/
    C_API AlgoError C_CALL PointFindGetBufferSize(AlgoHandle handle, uint64_t* out_len);

    /************************************
    @brief  : 将模块数据导出到buffer
    @note   :
    @param  : handle		--有效句柄
    @param  : out_buffer    --已经申请好申请空间
    @param  : in_out_len    --外部申请好的空间大小,会用真实大小修改
    @return	: AlgoError
    @Author : chengjiayi@2024/07/26
    ************************************/
    C_API AlgoError C_CALL PointFindToBuffer(AlgoHandle handle, char* out_buffer, uint64_t* in_out_len);

    /************************************
    @brief  : 将buffer数据导入到模块
    @note   :
    @param  : handle		--有效的句柄
    @param  : in_buffer		--导入的申请空间
    @param  : in_len		--导入的申请空间大小
    @return	: AlgoError
    @Author : chengjiayi@2024/07/26
    ************************************/
    C_API AlgoError C_CALL PointFindFromBuffer(AlgoHandle handle, const char* in_buffer, uint64_t in_len);

	/************************************
	@brief  : 获取模板图像
	@note   :
	@param  : handle		--有效的句柄
	@param  : out_template	--导入的申请空间
	@return	: AlgoError
	@Author : zhongao@2025/06/19
	************************************/
	C_API AlgoError C_CALL PointFindGetTemplate(AlgoHandle handle, AlgoMat& out_template);

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
    C_API AlgoError C_CALL PointFindRelease(AlgoHandle handle);
    C_API void C_CALL PointFindDestroyHandle(AlgoHandle handle);




#ifdef __cplusplus
}
#endif // __cplusplus
#endif