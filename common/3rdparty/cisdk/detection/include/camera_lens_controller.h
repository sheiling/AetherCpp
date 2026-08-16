#ifndef __CAMERA_LENS_CONTROLLER_H__
#define __CAMERA_LENS_CONTROLLER_H__

#include "ci_define.h"
#include "ci_declare.h"
#include "camera_lens_controller_data.h"
#include "camera_lens_controller_errors.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    /************************************
    @brief  : 创建CameraLensController句柄
    @note   :
    @param  : out_cameralenscontroller_handle   -- out_cameralenscontroller句柄
    @return	: AlgoError
    @Author : lichaoping@2024/07/15
    ************************************/
    C_API AlgoError C_CALL CameraLensControllerCreateHandle(
        AlgoHandle *out_cameralenscontroller_handle);


    /************************************
    @brief  : 计算曝光值
    @note   :
    @param  : cameralenscontroller_handle   -- CameraLensController句柄
    @param  : in_shot        -- 输入待检测的图像
    in_type=1时：计算灰度图中多个小区域的亮度作为曝光值，限制灯点全部过曝的模式；箱体效果检测中黑屏、花屏（错位）、余晖检测项采用；
    in_type=2时：普通模式（较多场景推荐使用）；计算灰度图中多个较大区域的亮度为曝光值。箱体效果检测中高对比耦合、第一扫偏暗、花屏色块检测项采用；
    in_type=3时：输入为三通道图，且要求三通道中的值都在线性区（不过曝）；箱体效果检测中均匀性检测项采用。
    @param  : out_exposure_value        -- 计算得到的图像曝光（亮度）值
    @param  : in_type		 --计算曝光值的模式。
    @return	: AlgoError
    @Author : lichaoping@2024/07/15
    ************************************/
    C_API AlgoError C_CALL CameraLensControllerCalExposureValue(
        AlgoHandle cameralenscontroller_handle, AlgoHandle in_shot,
        double *out_exposure_value, int in_type = 2);


    /************************************
    @brief  : 设置自动曝光参数
    @note   :
    @param  : cameralenscontroller_handle   -- CameraLensController句柄
    @param  : in_detection_type        --曝光值范围；
    in_detection_type=1时，限制曝光值在高灰范围；箱体效果检测中黑屏、鬼影、错位检测项采用；
    in_detection_type=2时，限制曝光值在中低灰范围；箱体效果检测中高对比耦合、第一扫偏暗、花屏检测项采用；
    in_detection_type=3时，限制曝光值在相机线性区（普通场景推荐使用），箱体效果检测中均匀性检测项采用）
    @return	: AlgoError
    @Author : lichaoping@2024/07/15
    ************************************/
    C_API AlgoError C_CALL CameraLensControllerSetAEPara(
        AlgoHandle cameralenscontroller_handle, int in_detection_type = 3);

    /************************************
    @brief  : 自定义设置曝光参数
    @note   :
    @param  : cameralenscontroller_handle   -- CameraLensController句柄
    @param  : in_param_struct         -- 参数结构体
    @return	: AlgoError
    @Author : lichaoping@2024/07/15
    ***********************************/
    C_API AlgoError C_CALL CameraLensControllerPersonlizedSetParam(
        AlgoHandle cameralenscontroller_handle,
        CameraLensControllerParam in_param_struct);

    /************************************
    @brief  : 相机内参分配值计算函数(在光圈固定不变场景下适用)
    @note   :
    @param  : cameralenscontroller_handle   -- CameraLensController句柄
    @param  : in_exposure_value        -- 相机实时图像的曝光值
    @param  : in_S        -- 相机实时的增益
    @param  : in_T        -- 相机实时曝光时间
    @param  : out_S        -- 所需调节的增益
    @param  : out_T        --所需调节的曝光时间
    @param  : out_result   --自动曝光结果  （结果为1表示曝光成功）
    @return	: AlgoError
    @Author : lichaoping@2024/07/15
    ************************************/
    C_API AlgoError C_CALL CameraLensControllerAutoExposure(
        AlgoHandle cameralenscontroller_handle, int in_exposure_value, int in_S,
        int in_T, int *out_S, int *out_T, int *out_result);


    /************************************
    @brief  : 获取buffer大小
    @note   :
    @param  : cameralenscontroller_handle  当前有效句柄
    @param  : out_len     buffer大小
    @return	: AlgoError
    @Author : lichaoping@2024/07/15
    ************************************/
    C_API AlgoError C_CALL CameraLensControllerGetBufferSize(
        AlgoHandle cameralenscontroller_handle, uint64_t *out_len);


    /************************************
    @brief  : 将所有设定的阈值导出到buffer
    @note   :
    @param  : cameralenscontroller_handle        有效的CameraLensController句柄
    @param  : out_buffer        已经申请好申请空间
    @param  : in_out_len        *in_out_len外部申请好的空间大小,会用真实大小修改
    @return	: AlgoError
    @Author : lichaoping@2024/07/15
    ************************************/
    C_API AlgoError C_CALL
    CameraLensControllerToBuffer(AlgoHandle cameralenscontroller_handle,
                                 char *out_buffer, uint64_t *in_out_len);


    /************************************
    @brief  : 将buffer数据导入到CameraLensController
    @note   :
    @param  : cameralenscontroller_handle
    有效的CameraLensController句柄
    @param  : in_buffer
    @param  : in_len
    @return	: AlgoError
    @Author : lichaoping@2024/07/15
    ************************************/
    C_API AlgoError C_CALL
    CameraLensControllerFromBuffer(AlgoHandle cameralenscontroller_handle,
                                   const char *in_buffer, uint64_t in_len);


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
    C_API AlgoError C_CALL
    CameraLensControllerRelease(AlgoHandle cameralenscontroller_handle);

    C_API void C_CALL
    CameraLensControllerDestroyHandle(AlgoHandle cameralenscontroller_handle);

#ifdef __cplusplus
};
#endif // __cplusplus
#endif
