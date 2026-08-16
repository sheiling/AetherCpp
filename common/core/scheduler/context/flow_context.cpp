#include "flow_context.h"

namespace scheduler {

// ── GlobalVariableStore ─────────────────────────────────────
GlobalVariableStore& GlobalVariableStore::instance() {
    static GlobalVariableStore inst;
    return inst;
}

// ── FlowContext ─────────────────────────────────────────────
bool FlowContext::has(const std::string& key) const {
    for (auto& scope : m_stepScopes_)
        if (scope.count(key)) return true;
    {
        std::shared_lock<std::shared_mutex> lk(m_flowMutex_);
        if (m_flowData_.count(key)) return true;
    }
    return GlobalVariableStore::instance().has(key);
}

void FlowContext::remove(const std::string& key, VariableScope scope) {
    switch (scope) {
    case VariableScope::STEP:
        if (!m_stepScopes_.empty()) m_stepScopes_.back().erase(key);
        break;
    case VariableScope::FLOW: {
        std::unique_lock<std::shared_mutex> lk(m_flowMutex_);
        m_flowData_.erase(key);
        break;
    }
    case VariableScope::GLOBAL:
        GlobalVariableStore::instance().remove(key);
        break;
    }
}

json FlowContext::serialize() const {
    json j;
    j["flowName"]   = flowName;
    j["instanceId"] = instanceId;
    j["priority"]   = priority;
    j["initParams"] = initParams;
    // 仅序列化 flow scope 的字符串/数值类型（通用序列化需反射，这里简化为 JSON 字符串型）
    json flowData = json::object();
    {
        std::shared_lock<std::shared_mutex> lk(m_flowMutex_);
        for (auto& [k, v] : m_flowData_) {
            try {
                // 尝试常见类型
                if (v.type() == typeid(std::string))
                    flowData[k] = std::any_cast<std::string>(v);
                else if (v.type() == typeid(int))
                    flowData[k] = std::any_cast<int>(v);
                else if (v.type() == typeid(double))
                    flowData[k] = std::any_cast<double>(v);
                else if (v.type() == typeid(bool))
                    flowData[k] = std::any_cast<bool>(v);
                else if (v.type() == typeid(int64_t))
                    flowData[k] = std::any_cast<int64_t>(v);
            } catch (...) {}
        }
    }
    j["flowData"] = flowData;
    return j;
}

void FlowContext::deserialize(const json& data) {
    flowName   = data.value("flowName",   "");
    instanceId = data.value("instanceId", "");
    priority   = data.value("priority",   0);
    if (data.contains("initParams")) initParams = data["initParams"];
    if (data.contains("flowData")) {
        std::unique_lock<std::shared_mutex> lk(m_flowMutex_);
        for (auto& [k, v] : data["flowData"].items()) {
            if (v.is_string())      m_flowData_[k] = v.get<std::string>();
            else if (v.is_number_integer()) m_flowData_[k] = v.get<int>();
            else if (v.is_number_float())   m_flowData_[k] = v.get<double>();
            else if (v.is_boolean())        m_flowData_[k] = v.get<bool>();
        }
    }
}

} // namespace scheduler
