-- Permission schema for MySQL / MariaDB
-- Version: 001

SET NAMES utf8mb4;

-- 权限资源：nav / sub_page / action（key 全局唯一）

CREATE TABLE IF NOT EXISTS perm_user (
    id              BIGINT        NOT NULL AUTO_INCREMENT,
    username        VARCHAR(64)   NOT NULL,
    password_hash   VARCHAR(255)  NOT NULL,
    display_name    VARCHAR(128)  NOT NULL,
    department      VARCHAR(128)  NULL,
    status          TINYINT       NOT NULL DEFAULT 1,
    last_login_at   DATETIME(3)   NULL,
    created_at      DATETIME(3)   NOT NULL,
    updated_at      DATETIME(3)   NOT NULL,
    remark          VARCHAR(512)  NULL,
    is_hidden       TINYINT(1)    NOT NULL DEFAULT 0,
    PRIMARY KEY (id),
    UNIQUE KEY uk_perm_user_username (username),
    KEY idx_perm_user_status (status)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS perm_role (
    id           BIGINT        NOT NULL AUTO_INCREMENT,
    code         VARCHAR(64)   NOT NULL,
    name         VARCHAR(128)  NOT NULL,
    description  VARCHAR(512)  NULL,
    is_system    TINYINT(1)    NOT NULL DEFAULT 0,
    tag_color    VARCHAR(16)   NULL,
    sort_order   INT           NOT NULL DEFAULT 0,
    enabled      TINYINT(1)    NOT NULL DEFAULT 1,
    created_at   DATETIME(3)   NOT NULL,
    updated_at   DATETIME(3)   NOT NULL,
    is_hidden    TINYINT(1)    NOT NULL DEFAULT 0,
    PRIMARY KEY (id),
    UNIQUE KEY uk_perm_role_code (code)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 权限资源（nav / sub_page / action）
CREATE TABLE IF NOT EXISTS perm_permission (
    id              BIGINT        NOT NULL AUTO_INCREMENT,
    parent_id       BIGINT        NULL,
    page_key        VARCHAR(128)  NOT NULL,
    resource_type   VARCHAR(32)   NOT NULL,
    `key`           VARCHAR(128)  NOT NULL,
    display_name    VARCHAR(256)  NOT NULL,
    description     VARCHAR(512)  NULL,
    source          VARCHAR(32)   NOT NULL DEFAULT 'manual',
    sort_order      INT           NOT NULL DEFAULT 0,
    enabled         TINYINT(1)    NOT NULL DEFAULT 1,
    created_at      DATETIME(3)   NOT NULL,
    updated_at      DATETIME(3)   NOT NULL,
    PRIMARY KEY (id),
    UNIQUE KEY uk_perm_permission_key (`key`),
    KEY idx_perm_permission_parent (parent_id),
    KEY idx_perm_permission_resource_type (resource_type)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS perm_user_role (
    user_id      BIGINT      NOT NULL,
    role_id      BIGINT      NOT NULL,
    is_primary   TINYINT(1)  NOT NULL DEFAULT 0,
    assigned_at  DATETIME(3) NOT NULL,
    PRIMARY KEY (user_id, role_id),
    CONSTRAINT fk_perm_user_role_user FOREIGN KEY (user_id)
        REFERENCES perm_user(id) ON DELETE CASCADE,
    CONSTRAINT fk_perm_user_role_role FOREIGN KEY (role_id)
        REFERENCES perm_role(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS perm_role_permission (
    role_id        BIGINT      NOT NULL,
    permission_id  BIGINT      NOT NULL,
    granted_at     DATETIME(3) NOT NULL,
    PRIMARY KEY (role_id, permission_id),
    CONSTRAINT fk_perm_rp_role FOREIGN KEY (role_id)
        REFERENCES perm_role(id) ON DELETE CASCADE,
    CONSTRAINT fk_perm_rp_perm FOREIGN KEY (permission_id)
        REFERENCES perm_permission(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS perm_audit_log (
    id            BIGINT        NOT NULL AUTO_INCREMENT,
    occurred_at   DATETIME(3)   NOT NULL,
    user_id       BIGINT        NULL,
    username      VARCHAR(64)   NOT NULL,
    role_names    VARCHAR(512)  NULL,
    action_type   VARCHAR(64)   NOT NULL,
    action_name   VARCHAR(128)  NULL,
    target        VARCHAR(256)  NULL,
    result        VARCHAR(16)   NOT NULL,
    ip            VARCHAR(64)   NULL,
    module_code   VARCHAR(64)   NULL,
    detail_json   TEXT          NULL,
    PRIMARY KEY (id),
    KEY idx_perm_audit_time (occurred_at),
    KEY idx_perm_audit_user (user_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
