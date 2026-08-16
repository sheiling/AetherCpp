-- Lighten inspection history schema for MySQL / MariaDB
-- Version: 004
-- 字段对齐「检测结果 -> 历史数据」宽表（27 列语义，见 InspectionHistoryRecord / InspectionResultsView）
-- 执行时机：由业务层在 open() 或首次写入前幂等执行（CREATE TABLE IF NOT EXISTS）

SET NAMES utf8mb4;

CREATE TABLE IF NOT EXISTS lighten_inspection_history (
    -- 行主键：自增整型，便于按 id 精确定位与关联
    id                  BIGINT        NOT NULL AUTO_INCREMENT,

    -- 基本信息（列 1～5）
    product_model       VARCHAR(128)  NOT NULL,                   -- 产品型号
    board_sn            VARCHAR(128)  NOT NULL,                   -- 灯板编号
    inspection_result   TINYINT       NOT NULL DEFAULT 0,         -- 检测结果：0=OK, 1=NG, 2=RW
    inspect_times       INT           NOT NULL DEFAULT 1,           -- 检测次数
    rw_count            INT           NOT NULL DEFAULT 0,           -- 返工次数
    inspect_duration_ms INT           NOT NULL DEFAULT 0,           -- 单片检测耗时（毫秒）

    -- 缺陷统计（列 6～24：6 类 × R/G/B 共 18 列 + 黑屏亮灯 1 列）
    dead_led_r          INT           NOT NULL DEFAULT 0,           -- 死灯 R
    dead_led_g          INT           NOT NULL DEFAULT 0,           -- 死灯 G
    dead_led_b          INT           NOT NULL DEFAULT 0,           -- 死灯 B
    dim_led_r           INT           NOT NULL DEFAULT 0,           -- 暗灯 R
    dim_led_g           INT           NOT NULL DEFAULT 0,           -- 暗灯 G
    dim_led_b           INT           NOT NULL DEFAULT 0,           -- 暗灯 B
    bright_led_r        INT           NOT NULL DEFAULT 0,           -- 亮灯 R
    bright_led_g        INT           NOT NULL DEFAULT 0,           -- 亮灯 G
    bright_led_b        INT           NOT NULL DEFAULT 0,           -- 亮灯 B
    color_var_r         INT           NOT NULL DEFAULT 0,           -- 串色 R
    color_var_g         INT           NOT NULL DEFAULT 0,           -- 串色 G
    color_var_b         INT           NOT NULL DEFAULT 0,           -- 串色 B
    series_led_r        INT           NOT NULL DEFAULT 0,           -- 串灯 R
    series_led_g        INT           NOT NULL DEFAULT 0,           -- 串灯 G
    series_led_b        INT           NOT NULL DEFAULT 0,           -- 串灯 B
    mixed_color_r       INT           NOT NULL DEFAULT 0,           -- 混色 R
    mixed_color_g       INT           NOT NULL DEFAULT 0,           -- 混色 G
    mixed_color_b       INT           NOT NULL DEFAULT 0,           -- 混色 B
    black_screen_light_count INT      NOT NULL DEFAULT 0,           -- 黑屏亮灯
    defect_total_count  INT           NOT NULL DEFAULT 0,           -- 缺陷总数（18 项分色统计 + 黑屏亮灯之和）
    defect_type         INT           NOT NULL DEFAULT 0,           -- 缺陷类型位掩码：bit0=死灯, bit1=暗灯, bit2=亮灯, bit3=串色, bit4=串灯, bit5=混色, bit6=黑屏亮灯, bit7=特殊检测项

    -- 尾部列（列 25～27）
    flash_result        TINYINT       NOT NULL DEFAULT 0,         -- Flash 结果：0=OK, 1=NG
    special_detect      VARCHAR(256)  NULL,                         -- 特殊检测项
    inspected_at        DATETIME      NOT NULL,                      -- 检测时间（yyyy-MM-dd HH:mm:ss）

    PRIMARY KEY (id),
    KEY idx_lighten_insp_product_model (product_model),
    KEY idx_lighten_insp_board_sn (board_sn),
    KEY idx_lighten_insp_inspection_result (inspection_result),
    KEY idx_lighten_insp_defect_type (defect_type),
    KEY idx_lighten_insp_flash_result (flash_result),
    KEY idx_lighten_insp_inspected_at (inspected_at)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
