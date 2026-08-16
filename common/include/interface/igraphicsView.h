#pragma once

#include <QImage>
#include <QList>
#include <QRect>

class QWidget;

enum E_ItemType
{
    E_TYPE_IMG = 1,
    E_TYPE_RECT,
    E_TYPE_ROT_RECT,
    E_TYPE_CIRCLE,
    E_TYPE_SECTOR,
    E_TYPE_RING,
    E_TYPE_POLYGON,
    E_TYPE_CTRL,
    E_TYPE_CROSS,
    E_TYPE_NONE
};

/**
 * @brief 图像显示视图服务接口（纯虚接口，无 QObject / Qt 插件 IID）
 *
 * GraphicsView 插件通过 GraphicsViewImpl 实现此接口，并注册到 PluginManager 对象池。
 * 消费方通过 PluginManager::getInterface<IGraphicsVew>() 获取实例后调用 CreateWidget()。
 *
 * 包含方式：#include <common/include/interface/igraphicsView.h>
 */
class IGraphicsVew
{
public:
    virtual ~IGraphicsVew() = default;

    /**
     * @brief 创建图像显示控件
     * @param winId 窗口标识（由调用方约定，用于区分多路预览等场景）
     * @return 调用方负责将返回的 QWidget 加入布局；所有权由调用方管理
     */
    virtual QWidget *CreateWidget(int winId) = 0;

    /**
     * @brief 设置指定窗口控件的显示状态
     * @param winId 窗口标识（与 CreateWidget 一致）
     * @param bStatus 状态栏区域是否显示（true 显示）
     * @param bDraw 右侧绘制面板是否显示（true 显示）
     */
    virtual bool SetVisible(int winId, bool bStatus, bool bDraw) = 0;

    virtual void SetImage(int winId, QImage image) = 0;
    virtual bool GetImage(int winId, QImage &out) const = 0;
    virtual void Reset(int winId) = 0;
    virtual void DrawRect(int winId) = 0;
    virtual void DrawRotRect(int winId) = 0;
    virtual void DrawCircle(int winId) = 0;
    virtual void DrawRing(int winId) = 0;
    virtual void DrawPolygon(int winId) = 0;
    virtual void DrawCross(int winId) = 0;
    virtual void DrawClean(int winId) = 0;

    /**
     * @brief 设置指定窗口 ROI（先清屏再按图像坐标绘制）
     * @param winId 窗口标识（与 CreateWidget 一致）
     * @param itemType ROI 图元类型
     * @param roi 参数格式与 GetRoi 一致（图像坐标系，单位：像素）
     */
    virtual void SetRoi(int winId, E_ItemType itemType, const QList<double> &roi) = 0;

    /**
     * @brief 获取指定窗口当前 ROI 参数（图像坐标系，单位：像素）
     * @param winId 窗口标识（与 CreateWidget 一致）
     * @param itemType 当前 ROI 图元类型
     * @param roi 按 itemType 填充：
     *   E_TYPE_RECT / E_TYPE_ROT_RECT — x1,y1,x2,y2,x3,y3,x4,y4（4 顶点）
     *   E_TYPE_CIRCLE — centerX,centerY,radius
     *   E_TYPE_RING — centerX,centerY,innerRadius,outerRadius
     *   E_TYPE_POLYGON — x1,y1,x2,y2,…（各顶点）
     * @return 存在有效 ROI 时返回 true
     */
    virtual bool GetRoi(int winId, E_ItemType &itemType, QList<double> &roi) = 0;

    /** @brief 获取当前 ROI 外接矩形（图像像素坐标）；无有效 ROI 时返回 false */
    virtual bool GetRoiRect(int winId, QRect &out) const = 0;

    /**
     * @brief 设置像素物理尺度（μm/px），用于状态栏 ROI 宽高 mm 换算；≤0 时不显示 mm
     */
    virtual void SetPixelScaleUmPerPx(int winId, double umPerPx)
    {
        (void)winId;
        (void)umPerPx;
    }

    /**
     * @brief 缺陷定位十字线（图像像素坐标，随 ImageItem 缩放/平移）
     * @note 须保持在 vtable **末尾**；与 GraphicsView.dll 同版本部署时生效
     */
    virtual void SetDefectCrosshair(int winId, double imageX, double imageY)
    {
        (void)winId;
        (void)imageX;
        (void)imageY;
    }
    /** @brief 清除缺陷定位十字线 */
    virtual void ClearDefectCrosshair(int winId) { (void)winId; }
};
