#ifndef __LED_POINT_LOCATOR_H__
#define __LED_POINT_LOCATOR_H__


#include "ci_define.h"
#include "ci_declare.h"
#include "led_point_locator_errors.h"
#include "led_point_locator_data.h"
#include "meta_info_enum.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


    /************************************
    @brief  : 初始化句柄
    @note   : 
    @param  : out_ledpoint_handle
    @return	: AlgoError
    @Author : chengjiayi@2021/06/18
    ************************************/
    C_API AlgoError C_CALL LedPointCreateHandle(AlgoHandle* out_ledpoint_handle);
    C_API AlgoHandle C_CALL LedPointNewHandle();


    /************************************
    @brief  : 初始化句柄
    @note   : 不调用此函数, 模块默认 = CPU线程数-1
    @param  : ledpoint_handle 有效的点定位句柄
    @param  : in_omp_num    并行运算线程个数
    @return	: AlgoError
    @Author : chengjiayi@2021/11/10
    ************************************/
    C_API AlgoError C_CALL LedPointSetThreadsNum(AlgoHandle ledpoint_handle, int in_omp_num);


    /************************************
    @brief  : 设置点定位模块方法
    @note   : 不调用此函数, 模块默认方法0, 不要随意更改方法
              in_method=0, TEMPLATE_MATCH精度最高速度最慢的 亚像素最准确 鲁棒性最好 
                    速度与cpu占用 灯点间隔相关 一般2000ms左右,所有情况都适用,灯点过少时候速度较慢
              in_method=10,GRAY_THRESHOLD, 针对于图像灯点个数比较少,且在图像上间隔很大的情况,灯点分的比较开,速度很快
                    精度与灯点质量有关  对灯点有要求 大概200ms
			  in_method=100,ADAPTIVE_THRESHOLD, 自适应阈值对于灯点数少或者大间隔场景速度很快，同时兼顾鲁棒性
					精度高，速度较快，一般700m左右

    @param  : ledpoint_handle   --有效的点定位句柄
    @param  : in_method         --点定位方法,目前 0=TEMPLATE_MATCH;10=GRAY_THRESHOLD;100=ADAPTIVE_THRESHOLD
    @return	: AlgoError
    @Author : wangyueyao@2024/02/04
    ************************************/
    C_API AlgoError C_CALL LedPointSetMethod(AlgoHandle ledpoint_handle, int in_method);

    /************************************
    @brief  : 设置下采样参数
    @note   :
    @param  : ledpoint_handle   有效的点定位句柄
    @param  : in_downsampling   下采样倍率,0-1之间, 值越小越快, 精度越差
    @return : AlgoError
    @Author : chengjiayi@2022/07/25
    ************************************/
    C_API AlgoError C_CALL LedPointSetDownsampling(AlgoHandle ledpoint_handle, float in_downsampling);


    /************************************
    @Brief		：	进行点定位
    @Note		：	
    @Parameter	: 	ledpoint_handle	点定位句柄
    @Parameter	: 	in_shot         有效的shot句柄    详见algo_shot.h中定义
    @Parameter	: 	in_region       有效的区域定位句柄 详见roi_detector.h中定义
    @Returns	:   AlgoError
    @Author		：	wangyueyao@2024/02/20
    ************************************/
    C_API AlgoError C_CALL LedPointLocate(AlgoHandle ledpoint_handle, const AlgoHandle in_shot, const AlgoHandle in_region);


	/************************************
	@Brief		：	基于任意灯点排布的点定位算法参数设置
	@Note		：	一般情况下，无需设置
	@Parameter	: 	ledpoint_handle	点定位句柄
	@Parameter	: 	thresh					--自适应阈值补偿值，默认44。thresh增大，灯点在图像所占面积减小，可能出现漏检情况；
											  反之，灯点在图像所占面积增大，可能灯点粘连，多颗灯检测为一颗灯
	@Parameter	: 	blockSize				--自适应阈值窗口，默认201，输入必须为奇数。blockSize大小取决于检测灯点所占像素个数，当超过默认201*201时才考虑适当增大
	@Parameter	: 	radius_med_min_ratio    --剔除非灯点最小比例，默认0.2。表示在灯点半径乘以最小和最大比例内的目标都视为灯点。
											  没有暗灯时可适当增大最小比例，精准定位正常发光灯点。
	@Parameter	: 	radius_med_max_ratio	--剔除非灯点最大比例，默认1.5。
	@Returns	:   AlgoError
	@Author		：	wangyueyao@2024/02/04
	************************************/
	C_API AlgoError C_CALL LedPointLocateSetParam(AlgoHandle ledpoint_handle, int thresh = 44, int blockSize = 201, 
		float radius_med_min_ratio = 0.2f, float radius_med_max_ratio = 1.5f);


	/************************************
	@Brief		：	基于任意灯点排布的点定位算法，暂不支持物理位置映射
	@Note		：	用于任意灯点排布，任意形状屏,无需输入ROI区域
	@Parameter	: 	ledpoint_handle	点定位句柄
	@Parameter	: 	in_shot         有效的shot句柄    详见algo_shot.h中定义
	@Returns	:   AlgoError
	@Author		：	wangyueyao@2024/02/04
	************************************/
	C_API AlgoError C_CALL LedPointLocateForAnyArrangement(AlgoHandle ledpoint_handle, const AlgoHandle in_shot);


	/************************************
	@Brief		：	使用标准点定位结果进行进行点定位。 目前仅适用于模组校正的场景
	@Note		：	注意：in_standard_ledpoint_handle必须保证所有的灯点都是live，及需调用LedPointLocateCheckAllStateLive确认是否符合standard的要求
	@Parameter	: 	ledpoint_handle	点定位句柄
	@Parameter	: 	in_shot         有效的shot句柄    详见algo_shot.h中定义
	@Parameter	: 	in_region       有效的区域定位句柄 详见roi_detector.h中定义
	@Parameter	: 	in_stander_ledpoint_handle       有效的点定位句柄。其中不能有一颗死灯
	@Returns	:   AlgoError
	@Author		：	tianbo@2023/12/28
	************************************/
	C_API AlgoError C_CALL LedPointLocateUsingStanderLoc(AlgoHandle ledpoint_handle, const AlgoHandle in_shot, const AlgoHandle in_region, AlgoHandle in_standard_ledpoint_handle);


	/************************************
	@Brief		：	检查ledpoint handle内的state属性是否都是LIVE（正常灯）
	@Note		：
	@Parameter	: 	ledpoint_handle	点定位句柄
	@Parameter	: 	in_shot         有效的shot句柄    详见algo_shot.h中定义
	@Returns	:   true表示全是正常灯， false表示handle输入不正常或内部存在非正常灯（eg：死灯）
	@Author		：	tianbo@2024/01/04
	************************************/
	C_API bool C_CALL LedPointLocateCheckAllStateLive(AlgoHandle ledpoint_handle);

	/************************************
	@Brief		：	Set search led ratio. 目前在模组校正场景下点定位提速使用，表示四角匹配的区域半径，单位是灯点
	@Note		：	in_search_led_ratio越大，则鲁棒性越好，但耗时变大。可支持最大的模组的位置偏移与该参数有关。
	@Parameter	: 	ledpoint_handle	点定位句柄
	@Parameter	: 	in_search_led_radius       灯点搜索半径，搜索范围为正方形，这里的in_search_led_radius表示正方形边长的一半
	@Returns	:   AlgoError
	@Author		：	tianbo@2024/01/04
	************************************/
	C_API AlgoError C_CALL LedPointLocateSetSearchLedRadius(AlgoHandle ledpoint_handle, int in_search_led_radius);

    /************************************
    @brief  : 直接利用rect区域进行点定位
    @note   : 注意, 最好确保rect准确, 常用情景是自动线等情形
    @param  : ledpoint_handle 点定位句柄
    @param  : in_shot        有效的shot句柄    详见algo_shot.h中定义
    @param  : in_rect        rect区域,注意最好确保rect准确, 常用情景是自动线等情形;
    @return	: AlgoError
    @Author : chengjiayi@2022/03/31
    ************************************/
    C_API AlgoError C_CALL LedPointLocateInRect(AlgoHandle ledpoint_handle, const AlgoHandle in_shot, const AlgoRect* in_region);

    /************************************
    @brief  : 对4个角点圈住的区域, 进行点定位
    @note   : 注意, 最好确保rect准确, 常用情景是自动线等情形
    @param  : ledpoint_handle 点定位句柄
    @param  : in_shot         有效的shot句柄    详见algo_shot.h中定义
    @param  : in_corner4      角点区域,注意最好确保in_corner4准确, 常用情景是自动线等情形; in_corner4=NULL时候,内部自动区域定位;
    @param  : in_num          角点个数,只能为4
    @return	: AlgoError
    @Author : chengjiayi@2022/03/31
    ************************************/
    C_API AlgoError C_CALL LedPointLocateInCorner4(AlgoHandle ledpoint_handle,const AlgoHandle in_shot,const AlgoPoint* in_corner4, int in_num = 4);

    /************************************
    @brief  : 针对metainfo不全情况下的点定位
    @note   : 
    @param  : ledpoint_handle       点定位句柄
    @param  : in_shot_no_metainfo   有效的shot句柄, metainfo可以缺失;如果shot包含metainfo,将会被忽略
    @param  : in_lighted_led_wh     shot中点亮灯点的列数行数
    @param  : in_corner4            角点区域,注意最好确保in_corner4准确, 常用情景是自动线等情形; in_corner4=NULL时候,内部自动区域定位;
    @param  : in_num                4
    @param  : in_led_layout         灯点排布类型
    @return : AlgoError
    @Author : chengjiayi@2022/07/25
    ************************************/
    C_API AlgoError C_CALL LedPointLocateNoMetainfo(AlgoHandle ledpoint_handle,
        const AlgoHandle in_shot_no_metainfo,
        const AlgoSize* in_lighted_led_wh,
        const AlgoPoint* in_corner4, int in_num=4,
        const LedLayout in_led_layout= LED_LAYOUT_RECTANGLE);
    

    /************************************
    @brief  : 获取点定位结果
    @note   : 
    @param  : ledpoint_handle  有效点定位句柄
    @param  : out_ledpoint   导出的结构体,必须已经初始化完毕
    @param  : is_deep_copy  是否深拷贝, 默认浅拷贝  1=深拷贝;0=浅拷贝
    @return	: AlgoError
    @Author : chengjiayi@2021/05/17
    ************************************/
    C_API AlgoError C_CALL LedPointGet(AlgoHandle ledpoint_handle, AlgoLedPoint* out_ledpoint, int in_is_deep_copy = 0);


    /************************************
    @brief  : 设置点定位结果数据
    @note   : 
    @param  : ledpoint_handle   有效点定位句柄
    @param  : in_ledpoint       结果数据
    @return	: AlgoError
    @Author : chengjiayi@2021/06/25
    ************************************/
    C_API AlgoError C_CALL LedPointSet(AlgoHandle ledpoint_handle,const AlgoLedPoint* in_ledpoint);

	/************************************
	@brief  : 基于任意灯点排布的点定位算法获取点定位结果
	@note   :
	@param  : ledpoint_handle  有效点定位句柄
	@param  : out_ledpoint   导出的结构体,必须已经初始化完毕
	@param  : out_search_radius		输出灯点搜索半径
	@param  : is_deep_copy  是否深拷贝, 默认浅拷贝  1=深拷贝;0=浅拷贝
	@return	: AlgoError
	@Author : wangyueyao@2024/02/20
	************************************/
	C_API AlgoError C_CALL LedPointGetForAnyArrangement(AlgoHandle ledpoint_handle, AlgoLedPoint* out_ledpoint, float *out_search_radius, int in_is_deep_copy = 0);


	/************************************
	@brief  : 基于任意灯点排布的点定位算法设置点定位结果数据
	@note   :
	@param  : ledpoint_handle   有效点定位句柄
	@param  : in_ledpoint       结果数据
	@param  : in_search_radius	输入灯点搜索半径
	@return	: AlgoError
	@Author : wangyueyao@2024/02/20
	************************************/
	C_API AlgoError C_CALL LedPointSetForAnyArrangement(AlgoHandle ledpoint_handle, const AlgoLedPoint* in_ledpoint, float in_search_radius);


    /************************************
    @Brief		：	载入和保存点定位数据
    @Note		：	文件名后缀必须是.loc
    @Parameter	: 	ledpoint_handle		--点定位句柄
    @Parameter	: 	in_loc_fullname		--文件名后缀必须是.loc
    @Returns	:   AlgoError
    @Author		：	wangyueyao@2024/02/20
    ************************************/
    C_API AlgoError C_CALL LedPointLoad(AlgoHandle ledpoint_handle, const char* in_loc_fullname);
    C_API AlgoError C_CALL LedPointSave(AlgoHandle ledpoint_handle, const char* in_loc_fullname);

   
    /************************************
    @brief  : 导出定位结果图像
    @note   : 黄色灯点是死灯
    @param  : ledpoint_handle   --未释放的点定位句柄
    @param  : in_png_fullname
    @param  : in_scale          --放大倍数建议3或5,默认=1不放大
    @param  : in_is_split       --是否只显示四角及中间,因为放大图特别大,1=分离,0=不分离
    @param  : in_is_show_info   --是否显示信息1=显示,0=不显示
    @param  : in_shot_handle    --将点画在此shot上
    @return	: AlgoError
    @Author : wangyueyao@2024/03/14
    ************************************/
    C_API AlgoError C_CALL LedPointExportImage(AlgoHandle ledpoint_handle,
        const char* in_png_fullname, 
        int in_scale=1, 
        int in_is_split=1,
        int in_is_show_info=1,
        const AlgoHandle in_shot_handle=nullptr);



    /************************************
    @brief    : 导出灯点状态图
    @note     : 红色=死灯,黑色=物理不存在;绿色=活灯;
    @param    : ledpoint_handle     --有效点定位句柄
    @param    : in_png_fullname 
    @param    : in_scale            --放大倍数建议3或5,默认=3放大
    @return   : AlgoError
    @Modified : chengjiayi@2022/10/27
    ************************************/
    C_API AlgoError C_CALL LedPointExportStateImg(AlgoHandle ledpoint_handle, const char* in_png_fullname, int in_scale=3);
    
    
    /************************************
    @brief  : 获取buffer大小
    @note   :
    @param  : ledpoint_handle  当前有效句柄
    @param  : out_len     buffer大小
    @return	: AlgoError
    @Author : wangyueyao@2024/02/20
    ************************************/
    C_API AlgoError C_CALL LedPointGetBufferSize(AlgoHandle ledpoint_handle, uint64_t* out_len);

    /************************************
    @brief  : 将ledpoint里面的数据导出到buffer
    @note   :
    @param  : ledpoint_handle   有效的ledpoint句柄
    @param  : out_buffer        已经申请好申请空间
    @param  : in_out_len        外部申请好的空间大小,会用真实大小修改
    @return	: AlgoError
    @Author : wangyueyao@2024/02/20
    ************************************/
    C_API AlgoError C_CALL LedPointToBuffer(AlgoHandle ledpoint_handle, char* out_buffer, uint64_t* in_out_len);


    /************************************
    @brief  : 将buffer数据导入到ledpoint
    @note   :
    @param  : ledpoint_handle   有效的ledpoint句柄
    @param  : in_buffer
    @param  : in_len
    @return	: AlgoError
    @Author : wangyueyao@2024/02/20
    ************************************/
    C_API AlgoError C_CALL LedPointFromBuffer(AlgoHandle ledpoint_handle, const char* in_buffer, uint64_t in_len);



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
    C_API AlgoError C_CALL LedPointRelease(AlgoHandle ledpoint_handle);
    C_API void C_CALL LedPointDestroyHandle(AlgoHandle ledpoint_handle);









#ifdef __cplusplus
}
#endif // __cplusplus
#endif