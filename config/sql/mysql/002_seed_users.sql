-- Permission seed: roles + users (step 2)

INSERT INTO perm_role (id, code, name, description, is_system, is_hidden, tag_color, sort_order,
                       enabled, created_at, updated_at)
VALUES (1, 'super_admin', '超级管理员', '系统内置，不可删除', 1, 1, '#ef4444', 0, 1,
        '2026-06-03 00:00:00.000', '2026-06-03 00:00:00.000'),
       (2, 'minimal', '最小权限', '未登录缺省身份；可在角色管理中调整 grants', 1, 0, '#64748b', 1, 1,
        '2026-06-03 00:00:00.000', '2026-06-03 00:00:00.000');

INSERT INTO perm_user (id, username, password_hash, display_name, department, status, last_login_at,
                     created_at, updated_at, remark, is_hidden)
VALUES (1, 'admin', '21232f297a57a5a743894a0e4a801fc3', '系统管理员', 'IT', 1, NULL,
        '2026-06-03 00:00:00.000', '2026-06-03 00:00:00.000', NULL, 1),
       (2, 'minimal', 'e10adc3949ba59abbe56e057f20f883e', '最小权限用户', '系统', 1, NULL,
        '2026-06-03 00:00:00.000', '2026-06-03 00:00:00.000', NULL, 0);

INSERT INTO perm_user_role (user_id, role_id, is_primary, assigned_at)
VALUES (1, 1, 1, '2026-06-03 00:00:00.000'),
       (2, 2, 1, '2026-06-03 00:00:00.000');
