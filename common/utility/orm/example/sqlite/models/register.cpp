/**
 * @file    register.cpp
 * @brief   灏?User / Department 妯″瀷娉ㄥ唽鍒?QxOrm 涓婁笅鏂? *
 * QxOrm 瑕佹眰姣忎釜妯″瀷鍦?.cpp 涓畬鎴愶細
 *   1. QX_REGISTER_CPP(ClassName)      鈥?宸ュ巶/搴忓垪鍖栧畯灞曞紑
 *   2. namespace qx { template <> void register_class(QxClass<ClassName> &t) { ... } }
 *      鍦ㄥ嚱鏁颁綋鍐呯敤 t.id / t.data / t.relationXxx 瀹屾垚鏄犲皠
 */
#include "Department.h"
#include "User.h"

// 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
// Department
// 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
QX_REGISTER_CPP(Department)
QX_REGISTER_SERIALIZE_QDATASTREAM_CPP(Department)

namespace qx {
template <> void register_class(QxClass<Department> &t)
{
    // 涓婚敭锛坅utoincrement锛?
    t.id(&Department::id, "id");

    // 鏅€氬瓧娈?
    t.data(&Department::name, "name");
    t.data(&Department::code, "code");

    // 鍏宠仈锛氫竴涓儴闂ㄥ寘鍚涓?User锛坰Key="users", 澶栭敭鍦?User 琛ㄤ笂鐨?department_id 鍒楋級
    t.relationOneToMany(&Department::users, "users", "department_id");
}
} // namespace qx

// 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
// User
// 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
QX_REGISTER_CPP(User)
QX_REGISTER_SERIALIZE_QDATASTREAM_CPP(User)

namespace qx {
template <> void register_class(QxClass<User> &t)
{
    // 涓婚敭
    t.id(&User::id, "id");

    // 鏅€氬瓧娈?
    t.data(&User::name,      "name");
    t.data(&User::email,     "email");
    t.data(&User::age,       "age");
    t.data(&User::active,    "active");
    t.data(&User::createdAt, "created_at");

    // 鍏宠仈锛氬涓?User 灞炰簬鍚屼竴涓?Department锛坰Key="department"锛屽閿垪 department_id锛?
    t.relationManyToOne(&User::department, "department");
}
} // namespace qx
