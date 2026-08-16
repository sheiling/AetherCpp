#pragma once

#include "param_serde.h"

namespace aoi {

#define PARAM(type, name, default_value, label_tag) type name = default_value

#define PARAM_DECLARE(type, name, default_value, label_tag) PARAM(type, name, default_value, label_tag);

#define REFLECT_FROM_FIELD(type, name, default_value, label_tag) FROM_JSON(j, name);

#define REFLECT_TO_FIELD(type, name, default_value, label_tag) TO_JSON(j, name);

#define EQ_FIELD_SUFFIX(type, name, default_value, label_tag) && ::aoi::serde::eqField(name, o.name)

#define DEFINE_PARAM_STRUCT(StructName, FIELDS_MACRO) \
    void reflectFromJson(const json &j) \
    { \
        FIELDS_MACRO(REFLECT_FROM_FIELD) \
    } \
    void reflectToJson(json &j) const \
    { \
        FIELDS_MACRO(REFLECT_TO_FIELD) \
    } \
    bool operator==(const StructName &o) const \
    { \
        return true FIELDS_MACRO(EQ_FIELD_SUFFIX); \
    } \
    bool operator!=(const StructName &o) const { return !(*this == o); }

} // namespace aoi
