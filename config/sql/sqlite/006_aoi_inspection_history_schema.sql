-- AOI inspection history schema for SQLite
-- Version: 006
-- 字段对齐 DefectInfoList / InspectionHistoryRow（见 plugins/aoi/repository/inspection_history/）
-- 执行时机：InspectionHistoryRepository::ensureSchema() 幂等执行（CREATE TABLE IF NOT EXISTS）

PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS aoi_inspection_history (
    id                          INTEGER PRIMARY KEY AUTOINCREMENT,
    product_name                TEXT    NOT NULL,
    sn                          TEXT    NOT NULL,
    detect_time                 TEXT    NOT NULL,
    inspection_result           INTEGER NOT NULL DEFAULT 0,
    inspect_times               INTEGER NOT NULL DEFAULT 1,
    rw_count                    INTEGER NOT NULL DEFAULT 0,
    defect_partitions           TEXT    NOT NULL,
    defect_info_full_file_name  TEXT    NULL,
    valid_defect_type           INTEGER NOT NULL DEFAULT 0,
    append                      INTEGER NOT NULL DEFAULT 0,
    chip_group_row_count        INTEGER NOT NULL DEFAULT 0,
    chip_group_col_count        INTEGER NOT NULL DEFAULT 0,
    die_row_count               INTEGER NOT NULL DEFAULT 0,
    die_col_count               INTEGER NOT NULL DEFAULT 0,
    defect_count_0              INTEGER NOT NULL DEFAULT 0,
    defect_count_1              INTEGER NOT NULL DEFAULT 0,
    defect_count_2              INTEGER NOT NULL DEFAULT 0,
    defect_count_3              INTEGER NOT NULL DEFAULT 0,
    defect_count_4              INTEGER NOT NULL DEFAULT 0,
    defect_count_5              INTEGER NOT NULL DEFAULT 0,
    defect_count_6              INTEGER NOT NULL DEFAULT 0,
    defect_count_7              INTEGER NOT NULL DEFAULT 0,
    defect_count_8              INTEGER NOT NULL DEFAULT 0,
    defect_count_9              INTEGER NOT NULL DEFAULT 0,
    defect_count_10             INTEGER NOT NULL DEFAULT 0
);

CREATE INDEX IF NOT EXISTS idx_aoi_insp_time
    ON aoi_inspection_history(detect_time DESC);

CREATE INDEX IF NOT EXISTS idx_aoi_insp_sn
    ON aoi_inspection_history(sn);

CREATE INDEX IF NOT EXISTS idx_aoi_insp_model
    ON aoi_inspection_history(product_name);
