-- Permission seed: 权限插件资源 + 角色授权（step 3）
-- 33 条 perm_permission；id 从 1 连续递增（0 仅作 parent_id 表根，不作资源 id）；sort_order 6000 段起
-- display_name 与 key 相同（i18n key）；展示时由 TR(key) 翻译

INSERT INTO perm_permission (id, parent_id, page_key, resource_type, key, display_name, description,
                             source, owner_plugin, sort_order, enabled, created_at, updated_at)
VALUES
    (1, 0, 'root', 'nav', 'nav.permission', 'nav.permission', '权限资源与角色授权配置', 'seed', 'Permission', 6000, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (2, 1, 'nav.permission', 'sub_page', 'sub.perm.users', 'sub.perm.users', NULL, 'seed', 'Permission', 6010, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (3, 1, 'nav.permission', 'sub_page', 'sub.perm.roles', 'sub.perm.roles', NULL, 'seed', 'Permission', 6020, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (4, 1, 'nav.permission', 'sub_page', 'sub.perm.resources', 'sub.perm.resources', NULL, 'seed', 'Permission', 6030, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (5, 1, 'nav.permission', 'sub_page', 'sub.perm.audit', 'sub.perm.audit', NULL, 'seed', 'Permission', 6040, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (6, 2, 'sub.perm.users', 'action', 'perm.users.query', 'perm.users.query', NULL, 'seed', 'Permission', 6090, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (7, 2, 'sub.perm.users', 'action', 'perm.users.create', 'perm.users.create', NULL, 'seed', 'Permission', 6100, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (8, 2, 'sub.perm.users', 'action', 'perm.users.resetPassword', 'perm.users.resetPassword', NULL, 'seed', 'Permission', 6110, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (9, 2, 'sub.perm.users', 'action', 'perm.users.resetFilter', 'perm.users.resetFilter', NULL, 'seed', 'Permission', 6120, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (10, 2, 'sub.perm.users', 'action', 'perm.users.edit', 'perm.users.edit', NULL, 'seed', 'Permission', 6130, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (11, 2, 'sub.perm.users', 'action', 'perm.users.toggleStatus', 'perm.users.toggleStatus', NULL, 'seed', 'Permission', 6140, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (12, 2, 'sub.perm.users', 'action', 'perm.users.bulkEnable', 'perm.users.bulkEnable', NULL, 'seed', 'Permission', 6150, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (13, 2, 'sub.perm.users', 'action', 'perm.users.bulkDisable', 'perm.users.bulkDisable', NULL, 'seed', 'Permission', 6160, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (14, 2, 'sub.perm.users', 'action', 'perm.users.bulkAssignRoles', 'perm.users.bulkAssignRoles', NULL, 'seed', 'Permission', 6170, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (15, 3, 'sub.perm.roles', 'action', 'perm.roles.save', 'perm.roles.save', NULL, 'seed', 'Permission', 6180, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (16, 3, 'sub.perm.roles', 'action', 'perm.roles.create', 'perm.roles.create', NULL, 'seed', 'Permission', 6190, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (17, 3, 'sub.perm.roles', 'action', 'perm.roles.edit', 'perm.roles.edit', NULL, 'seed', 'Permission', 6200, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (18, 3, 'sub.perm.roles', 'action', 'perm.roles.selectAll', 'perm.roles.selectAll', NULL, 'seed', 'Permission', 6210, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (19, 3, 'sub.perm.roles', 'action', 'perm.roles.clear', 'perm.roles.clear', NULL, 'seed', 'Permission', 6220, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (20, 3, 'sub.perm.roles', 'action', 'perm.roles.reset', 'perm.roles.reset', NULL, 'seed', 'Permission', 6230, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (21, 4, 'sub.perm.resources', 'action', 'perm.resources.createNav', 'perm.resources.createNav', NULL, 'seed', 'Permission', 6250, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (22, 4, 'sub.perm.resources', 'action', 'perm.resources.createSubPage', 'perm.resources.createSubPage', NULL, 'seed', 'Permission', 6260, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (23, 4, 'sub.perm.resources', 'action', 'perm.resources.createAction', 'perm.resources.createAction', NULL, 'seed', 'Permission', 6270, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (24, 4, 'sub.perm.resources', 'action', 'perm.resources.toggleEnabled', 'perm.resources.toggleEnabled', NULL, 'seed', 'Permission', 6280, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (25, 4, 'sub.perm.resources', 'action', 'perm.resources.reset', 'perm.resources.reset', NULL, 'seed', 'Permission', 6290, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (26, 4, 'sub.perm.resources', 'action', 'perm.resources.save', 'perm.resources.save', NULL, 'seed', 'Permission', 6300, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (27, 4, 'sub.perm.resources', 'action', 'perm.resources.delete', 'perm.resources.delete', NULL, 'seed', 'Permission', 6310, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (28, 5, 'sub.perm.audit', 'action', 'perm.audit.query', 'perm.audit.query', NULL, 'seed', 'Permission', 6320, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (29, 5, 'sub.perm.audit', 'action', 'perm.audit.resetFilter', 'perm.audit.resetFilter', NULL, 'seed', 'Permission', 6330, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (30, 5, 'sub.perm.audit', 'action', 'perm.audit.export', 'perm.audit.export', NULL, 'seed', 'Permission', 6340, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (31, 5, 'sub.perm.audit', 'action', 'perm.audit.prevPage', 'perm.audit.prevPage', NULL, 'seed', 'Permission', 6350, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (32, 5, 'sub.perm.audit', 'action', 'perm.audit.nextPage', 'perm.audit.nextPage', NULL, 'seed', 'Permission', 6360, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000'),
    (33, 5, 'sub.perm.audit', 'action', 'perm.audit.view', 'perm.audit.view', NULL, 'seed', 'Permission', 6370, 1,
     '2026-06-03T00:00:00.000', '2026-06-03T00:00:00.000');

-- super_admin：本脚本内全部 perm_permission
INSERT INTO perm_role_permission (role_id, permission_id, granted_at)
SELECT 1, id, '2026-06-03T00:00:00.000'
FROM perm_permission;

-- minimal：顶栏 + 四子页 + 用户查询/重置筛选 + 审计查询/重置/翻页（不含导出、查看详情）
INSERT INTO perm_role_permission (role_id, permission_id, granted_at)
VALUES
    (2, 1, '2026-06-03T00:00:00.000'),
    (2, 2, '2026-06-03T00:00:00.000'),
    (2, 3, '2026-06-03T00:00:00.000'),
    (2, 4, '2026-06-03T00:00:00.000'),
    (2, 5, '2026-06-03T00:00:00.000'),
    (2, 6, '2026-06-03T00:00:00.000'),
    (2, 9, '2026-06-03T00:00:00.000'),
    (2, 28, '2026-06-03T00:00:00.000'),
    (2, 29, '2026-06-03T00:00:00.000'),
    (2, 31, '2026-06-03T00:00:00.000'),
    (2, 32, '2026-06-03T00:00:00.000');

UPDATE sqlite_sequence SET seq = (SELECT MAX(id) FROM perm_permission) WHERE name = 'perm_permission';
