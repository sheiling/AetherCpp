-- Permission schema for SQLite
-- Version: 001
-- 资源唯一：key（page_key / resource_type 为树形与筛选元数据）

PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS perm_user (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    username        TEXT    NOT NULL UNIQUE,
    password_hash   TEXT    NOT NULL,
    display_name    TEXT    NOT NULL,
    department      TEXT    NULL,
    status          INTEGER NOT NULL DEFAULT 1,
    last_login_at   TEXT    NULL,
    created_at      TEXT    NOT NULL,
    updated_at      TEXT    NOT NULL,
    remark          TEXT    NULL,
    is_hidden       INTEGER NOT NULL DEFAULT 0
);

CREATE TABLE IF NOT EXISTS perm_role (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    code         TEXT    NOT NULL UNIQUE,
    name         TEXT    NOT NULL,
    description  TEXT    NULL,
    is_system    INTEGER NOT NULL DEFAULT 0,
    tag_color    TEXT    NULL,
    sort_order   INTEGER NOT NULL DEFAULT 0,
    enabled      INTEGER NOT NULL DEFAULT 1,
    created_at   TEXT    NOT NULL,
    updated_at   TEXT    NOT NULL,
    is_hidden    INTEGER NOT NULL DEFAULT 0
);

CREATE TABLE IF NOT EXISTS perm_permission (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    -- 根节点 parent_id=0（无父级）；不做自引用 FK，避免与业务约定冲突
    parent_id       INTEGER NULL,
    page_key        TEXT    NOT NULL,
    resource_type   TEXT    NOT NULL,
    key             TEXT    NOT NULL,
    display_name    TEXT    NOT NULL,
    description     TEXT    NULL,
    source          TEXT    NOT NULL DEFAULT 'manual',
    sort_order      INTEGER NOT NULL DEFAULT 0,
    enabled         INTEGER NOT NULL DEFAULT 1,
    created_at      TEXT    NOT NULL,
    updated_at      TEXT    NOT NULL,
    UNIQUE (key)
);

CREATE TABLE IF NOT EXISTS perm_user_role (
    user_id      INTEGER NOT NULL REFERENCES perm_user(id) ON DELETE CASCADE,
    role_id      INTEGER NOT NULL REFERENCES perm_role(id) ON DELETE CASCADE,
    is_primary   INTEGER NOT NULL DEFAULT 0,
    assigned_at  TEXT    NOT NULL,
  PRIMARY KEY (user_id, role_id)
);

CREATE TABLE IF NOT EXISTS perm_role_permission (
    role_id        INTEGER NOT NULL REFERENCES perm_role(id) ON DELETE CASCADE,
    permission_id  INTEGER NOT NULL REFERENCES perm_permission(id) ON DELETE CASCADE,
    granted_at     TEXT    NOT NULL,
  PRIMARY KEY (role_id, permission_id)
);

CREATE TABLE IF NOT EXISTS perm_audit_log (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    occurred_at   TEXT    NOT NULL,
    user_id       INTEGER NULL,
    username      TEXT    NOT NULL,
    role_names    TEXT    NULL,
    action_type   TEXT    NOT NULL,
    action_name   TEXT    NULL,
    target        TEXT    NULL,
    result        TEXT    NOT NULL,
    ip            TEXT    NULL,
    module_code   TEXT    NULL,
    detail_json   TEXT    NULL
);

CREATE INDEX IF NOT EXISTS idx_perm_user_status ON perm_user(status);
CREATE INDEX IF NOT EXISTS idx_perm_permission_parent ON perm_permission(parent_id);
CREATE INDEX IF NOT EXISTS idx_perm_permission_resource_type ON perm_permission(resource_type);
CREATE INDEX IF NOT EXISTS idx_perm_permission_page_key ON perm_permission(page_key);
CREATE INDEX IF NOT EXISTS idx_perm_audit_time ON perm_audit_log(occurred_at);
CREATE INDEX IF NOT EXISTS idx_perm_audit_user ON perm_audit_log(user_id);
