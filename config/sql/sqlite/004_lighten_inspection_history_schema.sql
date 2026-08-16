-- Lighten inspection history schema for SQLite
-- Version: 004
-- 字段对齐「检测结果 -> 历史数据」宽表（27 列语义，见 InspectionHistoryRecord / InspectionResultsView）
-- 执行时机：由业务层在 open() 或首次写入前幂等执行（CREATE TABLE IF NOT EXISTS）

PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS lighten_inspection_history (
    -- 行主键：自增整型，便于按 id 精确定位与关联
    id                  INTEGER PRIMARY KEY AUTOINCREMENT,

    -- 基本信息（列 1～5）
    product_model       TEXT    NOT NULL,                   -- 产品型号
    board_sn            TEXT    NOT NULL,                   -- 灯板编号
    inspection_result   INTEGER NOT NULL DEFAULT 0,       -- 检测结果：0=OK, 1=NG, 2=RW
    inspect_times       INTEGER NOT NULL DEFAULT 1,       -- 检测次数
    rw_count            INTEGER NOT NULL DEFAULT 0,       -- 返工次数
    inspect_duration_ms INTEGER NOT NULL DEFAULT 0,       -- 单片检测耗时（毫秒）

    -- 缺陷统计（列 6～24：6 类 × R/G/B 共 18 列 + 黑屏亮灯 1 列）
    dead_led_r          INTEGER NOT NULL DEFAULT 0,       -- 死灯 R
    dead_led_g          INTEGER NOT NULL DEFAULT 0,       -- 死灯 G
    dead_led_b          INTEGER NOT NULL DEFAULT 0,       -- 死灯 B
    dim_led_r           INTEGER NOT NULL DEFAULT 0,       -- 暗灯 R
    dim_led_g           INTEGER NOT NULL DEFAULT 0,       -- 暗灯 G
    dim_led_b           INTEGER NOT NULL DEFAULT 0,       -- 暗灯 B
    bright_led_r        INTEGER NOT NULL DEFAULT 0,       -- 亮灯 R
    bright_led_g        INTEGER NOT NULL DEFAULT 0,       -- 亮灯 G
    bright_led_b        INTEGER NOT NULL DEFAULT 0,       -- 亮灯 B
    color_var_r         INTEGER NOT NULL DEFAULT 0,       -- 串色 R
    color_var_g         INTEGER NOT NULL DEFAULT 0,       -- 串色 G
    color_var_b         INTEGER NOT NULL DEFAULT 0,       -- 串色 B
    series_led_r        INTEGER NOT NULL DEFAULT 0,       -- 串灯 R
    series_led_g        INTEGER NOT NULL DEFAULT 0,       -- 串灯 G
    series_led_b        INTEGER NOT NULL DEFAULT 0,       -- 串灯 B
    mixed_color_r       INTEGER NOT NULL DEFAULT 0,       -- 混色 R
    mixed_color_g       INTEGER NOT NULL DEFAULT 0,       -- 混色 G
    mixed_color_b       INTEGER NOT NULL DEFAULT 0,       -- 混色 B
    black_screen_light_count INTEGER NOT NULL DEFAULT 0, -- 黑屏亮灯
    defect_total_count  INTEGER NOT NULL DEFAULT 0,       -- 缺陷总数（18 项分色统计 + 黑屏亮灯之和）
    defect_type         INTEGER NOT NULL DEFAULT 0,       -- 缺陷类型位掩码：bit0=死灯, bit1=暗灯, bit2=亮灯, bit3=串色, bit4=串灯, bit5=混色, bit6=黑屏亮灯, bit7=特殊检测项

    -- 尾部列（列 25～27）
    flash_result        INTEGER NOT NULL DEFAULT 0,       -- Flash 结果：0=OK, 1=NG
    special_detect      TEXT    NULL,                     -- 特殊检测项
    inspected_at        TEXT    NOT NULL                  -- 检测时间（yyyy-MM-dd HH:mm:ss）
);

-- 按产品型号筛选
CREATE INDEX IF NOT EXISTS idx_lighten_insp_product_model
    ON lighten_inspection_history(product_model);

-- 按灯板编号精确/前缀查询
CREATE INDEX IF NOT EXISTS idx_lighten_insp_board_sn
    ON lighten_inspection_history(board_sn);

-- 按检测结果（OK/NG/RW）筛选
CREATE INDEX IF NOT EXISTS idx_lighten_insp_inspection_result
    ON lighten_inspection_history(inspection_result);

-- 按缺陷类型位掩码筛选
CREATE INDEX IF NOT EXISTS idx_lighten_insp_defect_type
    ON lighten_inspection_history(defect_type);

-- 按 Flash 结果筛选
CREATE INDEX IF NOT EXISTS idx_lighten_insp_flash_result
    ON lighten_inspection_history(flash_result);

-- 按检测时间排序/范围筛选
CREATE INDEX IF NOT EXISTS idx_lighten_insp_inspected_at
    ON lighten_inspection_history(inspected_at);
