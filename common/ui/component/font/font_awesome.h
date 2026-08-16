#pragma once

/**
 * @file font_awesome.h
 * @brief Font Awesome 5 Solid 图标（fa-solid-900.ttf，qrc :/resources/fonts/）
 */

#include "component_global.h"

#include <QColor>
#include <QFont>
#include <QIcon>
#include <QSize>
#include <QString>

namespace ui {

/** Font Awesome 5 Solid 常用码点（与 fa-solid-900.ttf 一致） */
// 查询网址 : https://fontawesome.com.cn/v5/
enum class FaIcon : char16_t {
    None = 0,

    // ── 媒体 / 预览 ─────────────────────────────────────────────
    Camera       = 0xf030, ///< 相机 / 拍照
    Play         = 0xf04b, ///< 播放
    Pause        = 0xf04c, ///< 暂停
    Stop         = 0xf04d, ///< 停止
    Image        = 0xf03e, ///< 图片
    Eye          = 0xf06e, ///< 显示 / 可见
    EyeSlash     = 0xf070, ///< 隐藏 / 不可见
    Expand       = 0xf065, ///< 全屏 / 放大视图
    Compress     = 0xf066, ///< 退出全屏 / 缩小视图
    SearchPlus   = 0xf00e, ///< 放大
    SearchMinus  = 0xf010, ///< 缩小

    // ── 编辑 / 文件 ─────────────────────────────────────────────
    Plus         = 0xf067, ///< 新增
    Minus        = 0xf068, ///< 减少
    Edit         = 0xf044, ///< 编辑
    Trash        = 0xf2ed, ///< 删除（trash-alt）
    Save         = 0xf0c7, ///< 保存
    Copy         = 0xf0c5, ///< 复制
    Undo         = 0xf0e2, ///< 撤销
    Redo         = 0xf01e, ///< 重做
    Download     = 0xf019, ///< 下载 / 导出
    Upload       = 0xf093, ///< 上传 / 导入
    Folder       = 0xf07b, ///< 文件夹
    FolderOpen   = 0xf07c, ///< 打开文件夹
    File         = 0xf15b, ///< 文件
    Print        = 0xf02f, ///< 打印

    // ── 确认 / 状态 ─────────────────────────────────────────────
    Check        = 0xf00c, ///< 勾选 / 确认
    Times        = 0xf00d, ///< 关闭 / 取消
    CheckCircle  = 0xf058, ///< 成功
    TimesCircle  = 0xf057, ///< 失败 / 错误
    PlusCircle   = 0xf055, ///< 添加（圆形）
    MinusCircle  = 0xf056, ///< 移除（圆形）
    InfoCircle   = 0xf05a, ///< 信息提示
    QuestionCircle = 0xf059, ///< 帮助 / 疑问
    ExclamationTriangle = 0xf071, ///< 警告
    Ban          = 0xf05e, ///< 禁止 / 停用

    // ── 导航 / 布局 ─────────────────────────────────────────────
    Home         = 0xf015, ///< 主页
    Bars         = 0xf0c9, ///< 菜单
    List         = 0xf03a, ///< 列表
    Th           = 0xf00a, ///< 网格 / 缩略图
    EllipsisH    = 0xf141, ///< 更多（横向）
    ChevronUp    = 0xf077, ///< 上折叠
    ChevronDown  = 0xf078, ///< 下折叠
    ChevronLeft  = 0xf053, ///< 向左
    ChevronRight = 0xf054, ///< 向右
    ArrowUp      = 0xf062, ///< 向上
    ArrowDown    = 0xf063, ///< 向下
    ArrowLeft    = 0xf060, ///< 向左
    ArrowRight   = 0xf061, ///< 向右
    ExpandArrows = 0xf31e, ///< 双向拉伸 / 同步宽高

    // ── 定位 / 地图 ─────────────────────────────────────────────
    Crosshairs   = 0xf05b, ///< 十字准星 / 取点
    LocationArrow = 0xf124, ///< 定位 / 移动到目标
    MapMarker    = 0xf3c5, ///< 地图标记（map-marker-alt）

    // ── 系统 / 工具 ─────────────────────────────────────────────
    Cog          = 0xf013, ///< 设置
    Wrench       = 0xf0ad, ///< 维护 / 调试
    Tools        = 0xf7d9, ///< 工具集
    Sync         = 0xf2f1, ///< 刷新 / 同步（sync-alt）
    Search       = 0xf002, ///< 搜索
    Filter       = 0xf0b0, ///< 筛选
    Bolt         = 0xf0e7, ///< 运行 / 触发
    PowerOff     = 0xf011, ///< 关机 / 停止服务
    Lock         = 0xf023, ///< 锁定
    Unlock       = 0xf09c, ///< 解锁

    // ── 用户 / 通知 / 时间 ───────────────────────────────────────
    User         = 0xf007, ///< 用户
    Users        = 0xf0c0, ///< 用户组
    Bell         = 0xf0f3, ///< 通知
    Calendar     = 0xf073, ///< 日历（calendar-alt）
    Clock        = 0xf017, ///< 时间

    // ── 数据 / 链接 ─────────────────────────────────────────────
    ChartBar     = 0xf080, ///< 图表 / 统计
    Database     = 0xf1c0, ///< 数据库
    Link         = 0xf0c1, ///< 链接
    ExternalLink = 0xf35d, ///< 外部链接
    Share        = 0xf064, ///< 分享

    // ── 形状 / 标记 ─────────────────────────────────────────────
    Circle       = 0xf111, ///< 圆点
    Square       = 0xf0c8, ///< 方块
    CheckSquare  = 0xf14a, ///< 已选方块
    Star         = 0xf005, ///< 收藏 / 标记
};

class UI_COMPONENT_EXPORT FontAwesome
{
public:
    /** 加载 qrc 字体（幂等）；也可依赖 font() 懒加载 */
    static void ensureLoaded();

    static QFont font(int pixelSize);

    static QString glyph(FaIcon icon);

    static QIcon icon(FaIcon glyph, const QSize &size, const QColor &color = {});

    /** checkable 按钮：Off=first，On=second */
    static QIcon toggleIcon(FaIcon offGlyph, FaIcon onGlyph, const QSize &size,
                            const QColor &color = {});

    static QColor defaultIconColor();

private:
    static QString familyName();
};

} // namespace ui
