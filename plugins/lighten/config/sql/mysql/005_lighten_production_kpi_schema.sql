-- Lighten production KPI schema for MySQL / MariaDB
-- Version: 005
-- 字段：总点亮灯板数、OK/NG/RW 数量、对插次数、通过率（对齐首页 KpiSnapshot 核心统计项）
-- 执行时机：由业务层在 open() 或首次写入前幂等执行（CREATE TABLE IF NOT EXISTS）

SET NAMES utf8mb4;

CREATE TABLE IF NOT EXISTS lighten_production_kpi (
    total_lightened_panels  INT            NOT NULL DEFAULT 0,   -- 总点亮灯板数
    ok_count                INT            NOT NULL DEFAULT 0,   -- OK 数量
    ng_count                INT            NOT NULL DEFAULT 0,   -- NG 数量
    rw_count                INT            NOT NULL DEFAULT 0,   -- RW 数量
    insertion_count         INT            NOT NULL DEFAULT 0,   -- 对插次数
    pass_rate               DOUBLE         NOT NULL DEFAULT 0.0  -- 通过率（百分比 0–100）
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
