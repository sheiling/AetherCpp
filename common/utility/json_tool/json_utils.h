#ifndef __COMMON_UTILITY_JSON_UTILS_H__
#define __COMMON_UTILITY_JSON_UTILS_H__

#include <fstream>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <filesystem>
#include <system_error>
#include <string>
#include <3rdparty/nlohmann/json.hpp>
#include <Logger.h>

namespace common
{

namespace utility
{
/**
 * @brief json_utils 统一错误输出（写入 Logger 日志文件）
 */
inline void json_logError(const std::string& msg)
{
    LOG_ERROR("[common][utility][json_utils] {}", msg);
}
struct JsonParseFileOptions
{
    bool allow_missing_file   = false;      ///< 文件不存在时是否视为成功（并返回空对象）
    bool normalize_to_object  = false;      ///< 根节点不是 object 时是否归一化为 {}
};

struct JsonSaveFileOptions
{
    bool create_parent_dir = false;         ///< 写入前是否自动创建父目录
};

/**
 * @brief 解析 JSON 文件（扩展版本，支持行为策略）
 *
 * @param data      输出 JSON 对象
 * @param file_path 文件路径
 * @param options   解析策略（缺失文件、根对象归一化）
 */
bool inline json_parseFromFileEx(nlohmann::json& data,
                                 const std::string& file_path,
                                 const JsonParseFileOptions& options)
{
    // 读文件失败时可按策略将“文件缺失”视作空配置。
    std::ifstream file(file_path);
    if (!file.is_open())
    {
        if (options.allow_missing_file)
        {
            data = nlohmann::json::object();
            return true;
        }
        json_logError("Open file failed: " + file_path);
        return false;
    }

    std::ostringstream stringStream;
    stringStream << file.rdbuf();
    const std::string jsonStrStream = stringStream.str();
    file.close();
    try
    {
        data = nlohmann::json::parse(jsonStrStream);
        // 某些业务只接受 object 根节点，此处按策略统一归一化。
        if (options.normalize_to_object && !data.is_object())
            data = nlohmann::json::object();
    }
    catch (std::exception& e)
    {
        json_logError("parse json error, filename=" + file_path + ", errMsg=" + e.what());
        return false;
    }
    return true;
}

 // 解析 JSON 文件
bool inline json_parseFromFile(nlohmann::json& data, const std::string& file_path)
{
    return json_parseFromFileEx(data, file_path, JsonParseFileOptions{});
}

bool inline json_saveToFileEx(const nlohmann::json& data,
                              const std::string& file_path,
                              const JsonSaveFileOptions& options)
{
    // 写文件前可按策略自动创建目录，减少业务重复代码。
    if (options.create_parent_dir)
    {
        std::error_code ec;
        const std::filesystem::path path(file_path);
        const auto parent = path.parent_path();
        if (!parent.empty() && !std::filesystem::exists(parent)
            && !std::filesystem::create_directories(parent, ec))
        {
            json_logError("Create parent dir failed: " + parent.string() + ", errMsg=" + ec.message());
            return false;
        }
    }

    std::ofstream file(file_path);
    if (!file.is_open())
    {
        json_logError("Open file failed: " + file_path);
        return false;
    }
    file << data.dump(4);
    file.close();
    return true;
}

 // 保存 JSON 文件
bool inline json_saveToFile(const nlohmann::json& data, const std::string& file_path)
{
    return json_saveToFileEx(data, file_path, JsonSaveFileOptions{});
}


// 解析 JSON 字符串
bool inline json_parseFromString(nlohmann::json& data, const std::string& json_str)
{
    if (json_str.empty())
    {
        json_logError("json_str is empty.");
        return false;
    }
    try
    {
        data = nlohmann::json::parse(json_str);
    }
    catch (std::exception& e)
    {
        json_logError("parse json string error, errMsg=" + std::string(e.what()));
        return false;
    }
    return true;
}


// 保存 JSON 字符串
bool inline json_saveToString(const nlohmann::json& data, std::string& json_str)
{
    if (data.is_null())
    {
        json_logError("data is null.");
        return false;
    }
    try
    {
        json_str = data.dump(4);
    }
    catch (std::exception& e)
    {
        json_logError("dump json error, errMsg=" + std::string(e.what()));
        return false;
    }
    return true;
}

//检查是否存在指定路径的键  点分隔的键路径（如 "a.b.c"）
bool inline json_hasKey(const nlohmann::json& data, const std::string& key_path)
{
    std::istringstream iss(key_path);
    std::string key;
    nlohmann::json current = data;

    while (std::getline(iss, key, '.')) {
        if (!current.is_object() || !current.contains(key)) {
            return false;
        }
        current = current[key];
    }
    return true;
}


// 获取键路径对应的值（严格类型检查）
template <typename T>
T json_getValue(const nlohmann::json& data, const std::string& key_path, const T& default_val = T())
{
    std::istringstream iss(key_path);
    std::string key;
    nlohmann::json current = data;

    try 
    {
        while (std::getline(iss, key, '.')) 
        {
            if (!current.is_object() || !current.contains(key)) 
            {
                json_logError("Key not found: " + key_path);
                break;
            }
            current = current[key];
        }

        return current.is_null() ? default_val : current.get<T>();
    } 
    catch (const json::exception& e) 
    {
        json_logError("JSON error at key: " + key_path + ", errMsg=" + e.what());
    }

    return default_val;
}


// 设置键路径对应的值（自动创建中间路径）
template <typename T>
void json_setValue(nlohmann::json& data, const std::string& key_path, const T& value)
{
    std::vector<std::string> keys;
    std::istringstream iss(key_path);
    std::string key;

    while (std::getline(iss, key, '.')) 
    {
        keys.push_back(key);
    }

    nlohmann::json* current = &data;

    for (size_t i = 0; i < keys.size(); ++i)
    {
        const std::string& k = keys[i];

        if (!current->is_object())
        {
            json_logError("Path conflict at: " + k);
        }

        if (i == keys.size() - 1) 
        {
            (*current)[k] = value;
        } 
        else 
        {
            if (!current->contains(k)) 
            {
                (*current)[k] = nlohmann::json::object();
            }
            current = &(*current)[k];
        }
    }
}

}

}

#endif