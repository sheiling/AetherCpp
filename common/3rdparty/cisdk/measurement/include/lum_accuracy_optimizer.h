#ifndef __LUM_ACCURACY_OPTIMIZER_H__
#define __LUM_ACCURACY_OPTIMIZER_H__

#include "ci_define.h"
#include "ci_declare.h"
#include "lum_accuracy_optimizer_data.h"
#include "lum_accuracy_optimizer_error.h"
#include "ci_measurement_enum.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    /************************************
    @brief  : 创建lum_opti句柄
    @note   :
    @param  : out_lum_opti_handle   -- lum_opti句柄
    @return	: AlgoError
    @Author : ouxingtao@2023/3/9
    ************************************/
    C_API AlgoError C_CALL LumOptimizerCreateHandle(AlgoHandle *out_lum_opti_handle);

    /************************************
    @brief  : 生成公共光通量优化系数
    @note   : 用num组生成公共系数
    @param  : in_filename         -- 输入num组文件地址
    @param  : out_filename        -- 公共系数保存地址
    @param  : num                 -- 输入数据的数量
    @return : AlgoError
    @Author : ouxingtao@2023/03/23
    ************************************/
    C_API AlgoError C_CALL LumOptimizerMean(const char *in_filename[], const char *out_filename, const int num);

    /************************************
    @brief  : 输入隔点数据生成的meter
    @note   : 计算优化系数,一个psf只能存储一个颜色一个滤片
    @param  : lum_opti_handle       -- 有效的lum_opti句柄
    @param  : in_meter_handle       -- 输入的光通量meter句柄
    @param  : in_meter_interval     -- 输入光通量的隔点数
    @param  : in_psf_diameter       -- 输入psf的大小--直径
    @param  : in_initial_point      --
    光通量起始位置--隔点光通量，左上角亮灯的第一个位置
    @return	: AlgoError
    @Author : ouxingtao@2023/3/9
    ************************************/
    C_API AlgoError C_CALL LumOptimizerCalib(AlgoHandle lum_opti_handle, AlgoHandle in_meter_handle,
                                             const int in_meter_interval, const int in_psf_diameter,
                                             AlgoPoint in_initial_point);


    /************************************
    @brief  : 输入隔点数据生成的meter
    xyz，生成以color为单位的psf文件，里面可包含XYZ其中的一个或两个或全部数据。
    @note   :
    若输入一个meter长度为1时，与LumOptimizerCalib接口一致。一个psf可以存储一个颜色所有滤片
    @param  : lum_opti_handle       -- 有效的lum_opti句柄
    @param  : in_meter_xyz_handle   -- in_meter_xyz_handle,meter的list指针。
    @param  : in_meter_num          -- 输入的光通量meter的长度
    @param  : in_meter_interval     -- 输入光通量的隔点数
    @param  : in_psf_diameter       -- 输入psf的大小--直径
    @param  : in_initial_point      --
    光通量起始位置--隔点光通量，左上角亮灯的第一个位置
    @return	: AlgoError
    @Author : 田博@2024/06/09
    ************************************/
    C_API AlgoError C_CALL LumOptimizerCalibs(AlgoHandle lum_opti_handle, AlgoHandle *in_meter_xyz_handle,
                                              int in_meter_num, const int in_meter_interval, const int in_psf_diameter,
                                              AlgoPoint in_initial_point);

    /************************************
    @brief  : 对光通量数据进行优化处理
    @note   : 对光通量修正，允许两种情况：
              情况1：设置了“外部光通量”，则使用外部光通量对边缘光通量进行去混光；
              情况2：没有设置“外部光通量”，则最终对上下左右1行光通量默认用原始数据
    @param  : lum_opti_handle       -- 有效的lum_opti句柄
    @param  : in_out_meter_handle   -- 输入的光通量meter句柄
    @param  : in_block_row          -- 光通量分区优化时，分区的行数
    @param  : in_block_col          -- 光通量分区优化时，分区的列数
    @return	: AlgoError
    @Author : jpzheng@2025/01/02
    ************************************/
    C_API AlgoError C_CALL LumOptimizerRectify(AlgoHandle lum_opti_handle, AlgoHandle in_out_meter_handle,
                                               const int in_block_row = 3, const int in_block_col = 4);

    /************************************
    @brief  : 读取保存各颜色各滤片的优化系数
    @note   : 后缀只能是.psf
    @param  : lum_opti_handle       -- 有效的lum_opti句柄
    @param  : in_lum_opti_fullname  -- 标定文件,后缀只能是.psf
    @return : AlgoError
    @Author : ouxingtao@2023/3/9
    ************************************/
    C_API AlgoError C_CALL LumOptimizerSave(AlgoHandle lum_opti_handle, const char *in_lum_opti_fullname);
    C_API AlgoError C_CALL LumOptimizerLoad(AlgoHandle lum_opti_handle, const char *in_lum_opti_fullname);

    /************************************
    @brief  : 将lum_opti_handle里面的数据导出到buffer
    @note   :
    @param  : lum_opti_handle       -- 有效的lum_opti句柄
    @param  : out_buffer            -- 已经申请好申请空间
    @param  : in_out_len            -- 外部申请好的空间大小,会用真实大小修改
    @return	: AlgoError
     @Author : ouxingtao@2023/3/9
    ************************************/
    C_API AlgoError C_CALL LumOptimizerToBuffer(AlgoHandle lum_opti_handle, char *out_buffer, uint64_t *in_out_len);

    /************************************
    @brief  : 获取句柄内容
    @note   :
    @param  : lum_opti_handle       -- 有效的lum_opti句柄
    @param  : out_para           导出的结构体,必须已经初始化完毕
    @param  : in_is_deep_copy    是否深拷贝, 默认浅拷贝  1=深拷贝;0=浅拷贝
    @return	: AlgoError
    @Author : Dulonghui@2024/05/22
    ************************************/
    C_API AlgoError C_CALL LumOptimizerGet(AlgoHandle lum_opti_handle, AlgoLumOptimizerData *out_para,
                                           int in_is_deep_copy = 0);


    /************************************
    @brief  : 获取buffer大小
    @note   :
    @param  : lum_opti_handle       -- 有效的lum_opti句柄
    @param  : out_len               -- buffer大小
    @return	: AlgoError
    @Author : ouxingtao@2023/3/9
    ************************************/
    C_API AlgoError C_CALL LumOptimizerGetBufferSize(AlgoHandle lum_opti_handle, uint64_t *out_len);


    /************************************
    @brief  : 将buffer数据导入到lum_opti_handle
    @note   :
    @param  : lum_opti_handle       -- 有效的lum_opti句柄
    @param  : in_buffer
    @param  : in_len
    @return	: AlgoError
    @Author : ouxingtao@2023/3/9
    ************************************/
    C_API AlgoError C_CALL LumOptimizerFromBuffer(AlgoHandle lum_opti_handle, const char *in_buffer, uint64_t in_len);


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
    C_API AlgoError C_CALL LumOptimizerRelease(AlgoHandle lum_opti_handle);
    C_API void C_CALL LumOptimizerDestroyHandle(AlgoHandle lum_opti_handle);


    /************************************
    @brief  : 计算psf模型，标定动作，多个滤片数据输入
    @note   :
    1、in_psf_size_result该参数算法内部具体实现为“由in_block_size参数经过固定的
    转换关系求解出真实的psf_size_real，并扩展in_psf_size_result-psf_size_real圈数，
    得到最终的尺寸”
    2、使用该标定时，必须提前使用 LumOptimizerSetExternalLum 设置该标定pattern的外部光通量

    @param  : in_lum_opti_handle			-- improver句柄
    @param  : in_block_meter_xyz_handle		-- 标定pattern的meters
    @param  : in_block_shot_xyz_handle		-- 标定pattern的shots(光修&去背景后)
    @param  : in_block_contour_xyz_handle
                                            --标定pattern的contours(该箱体全亮的contour)
    @param  : in_filters_num				--数目（meters、shots、contours数目要一致）
    @param  : in_block_size
	-- 为奇数
    -- 标定亮块的尺寸(需要和DisplayPatternComputeBlockForPSF的尺寸一致)，一般不动
    -- 如果需要改动，该参数取值最小为11，最大为屏幕宽高(取最小)的一半

    @param  : in_psf_size_result    -- 求解的psf尺寸大小
    1、指定尺寸模式：
    -- 必须为奇数
    -- 求解PSF的尺寸，一般不动
    -- 如果需要改动，该参数取值最小为3，最大为屏幕宽高(取最小)的一半
    2、内部自适应计算模式：
    -- 满足1中的所有条件；
    -- 设置数值为负数，比如 -63
    @return	: AlgoError
    @Author : jpzheng@2025/01/21
    ************************************/
    C_API AlgoError C_CALL LumOptimizerCalibsByBlock(AlgoHandle in_lum_opti_handle,
                                                     AlgoHandle *in_block_meter_xyz_handle,
                                                     AlgoHandle *in_block_shot_xyz_handle,
                                                     AlgoHandle *in_block_contour_xyz_handle, int in_filters_num,
                                                     const int in_block_size = 45, const int in_psf_size_result = -63);

    /************************************
    @brief  : 将外扩的光通量数据导入到lum_opti_handle
    @note   :
    @param  : in_lum_opti_handle     -- 有效的lum_opti句柄
    @param  : in_lum_external		 -- 待校正屏幕外部光通量
    @param  : in_cie_type			 -- 该外部光通量的滤片类型
    @param  : in_color				 -- 该外部光通量的颜色类型，需要和有效的lum_opti句柄的颜色一致
    @return	: AlgoError
    @Author : jpzheng@2025/01/06
    ************************************/
    C_API AlgoError C_CALL LumOptimizerSetExternalLum(AlgoHandle in_lum_opti_handle, AlgoMat *in_lum_external,
                                                      const CIEType in_cie_type, const AlgoColor in_color);


    /************************************
    @brief  : Export图像，可视化标定数据
    @note   :
    @param  : in_lum_opti_handle     -- 有效的lum_opti句柄
    @param  : in_full_name			 -- 图像保存完整路径(png、jpeg、bmp等opencv支持文件类型)
    @param  : in_cie_type			 -- 该光通量的滤片类型
    @return	: AlgoError
    @Author : jpzheng@2024/12/27
    ************************************/
    C_API AlgoError C_CALL LumOptimizerExport(AlgoHandle in_lum_opti_handle, const char *in_full_name,
                                              const CIEType cie_type);


	/************************************
	@brief  : 设置校验等级
	@note   :
	@param  : in_lum_opti_handle	-- 有效的lum_opti句柄
	@param  : level					-- 校验等级
	@return	: AlgoError
	@Author : jpzheng@2026/01/20
	************************************/
	C_API AlgoError C_CALL LumOptimizerSetVerificationLevel(AlgoHandle in_lum_opti_handle, const VerificationLevel level);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif