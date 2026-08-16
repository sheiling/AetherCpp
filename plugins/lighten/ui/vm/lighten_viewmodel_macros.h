#pragma once

/**
 * @file lighten_viewmodel_macros.h
 * @brief Lighten 插件 ViewModel 属性宏（信号统一 sig_ 前缀）
 *
 * 与 common/ui/mvvm/viewmodel.h 中 VIEWMODEL_* 宏行为一致，
 * 但具名变更信号使用 sig_<name>Changed() 命名约定。
 */

#include "viewmodel.h"

#define LIGHTEN_VIEWMODEL_PROPERTY(Type, name, Name) \
public: \
    Type name() const { \
        return getProperty(#name).value<Type>(); \
    } \
    void set##Name(const Type &value) { \
        if (name() != value) { \
            setProperty(#name, QVariant::fromValue(value)); \
            emit sig_##name##Changed(); \
            emit propertyChanged(#name, QVariant::fromValue(value)); \
        } \
    } \
private:

#define LIGHTEN_VIEWMODEL_STRUCT_PROPERTY(Type, name, Name) \
public: \
    Type name() const { \
        return getProperty(#name).value<Type>(); \
    } \
    void set##Name(const Type &value) { \
        setProperty(#name, QVariant::fromValue(value)); \
        emit sig_##name##Changed(); \
    } \
    void modify##Name(std::function<void(Type &)> modifier) { \
        Type current = name(); \
        modifier(current); \
        set##Name(current); \
    } \
private:
