#include "namedservice.h"

namespace Container {

// ── MethodDispatcher ──────────────────────────────────────────────────────────

void MethodDispatcher::registerMethod(const std::string& name, InvokeFn fn)
{
    methods_[name] = std::move(fn);
}

bool MethodDispatcher::hasMethod(const std::string& name) const
{
    return methods_.count(name) > 0;
}

std::any MethodDispatcher::invoke(const std::string& name,
                                  void*              instance,
                                  std::vector<std::any> args) const
{
    auto it = methods_.find(name);
    if (it == methods_.end()) {
        throw std::runtime_error(
            "[MethodDispatcher] Method not registered: '" + name + "'. "
            "Make sure it was registered via on() / readOnly() / readWrite() "
            "in the DispatchSetupFn passed to bindNamed/singletonNamed.");
    }
    try {
        return it->second(instance, std::move(args));
    } catch (const std::bad_any_cast& e) {
        throw std::runtime_error(
            "[MethodDispatcher] Method '" + name + "' argument type mismatch: "
            + e.what()
            + ". Verify that each std::any in the args vector matches the "
              "exact type expected by the registered method signature "
              "(use std::decay_t; pass std::string not const char*).");
    }
}

std::any MethodDispatcher::getProperty(const std::string& name, void* instance) const
{
    return invoke("get:" + name, instance, {});
}

void MethodDispatcher::setProperty(const std::string& name,
                                   void*              instance,
                                   std::any           value) const
{
    invoke("set:" + name, instance, { std::move(value) });
}

// ── ServiceHandle ─────────────────────────────────────────────────────────────

ServiceHandle::ServiceHandle(std::shared_ptr<void>             instance,
                             std::shared_ptr<MethodDispatcher> dispatcher)
    : instance_(std::move(instance))
    , dispatcher_(std::move(dispatcher))
{}

std::any ServiceHandle::call(const std::string&    method,
                             std::vector<std::any> args) const
{
    if (!instance_) {
        throw std::runtime_error(
            "[ServiceHandle] Cannot call '" + method + "' on a null handle.");
    }
    if (!dispatcher_) {
        throw std::runtime_error(
            "[ServiceHandle] No MethodDispatcher registered for this service. "
            "Pass a setup lambda when calling bindNamed/singletonNamed.");
    }
    return dispatcher_->invoke(method, instance_.get(), std::move(args));
}

bool ServiceHandle::hasMethod(const std::string& method) const
{
    return dispatcher_ && dispatcher_->hasMethod(method);
}

std::any ServiceHandle::get(const std::string& propName) const
{
    if (!instance_) {
        throw std::runtime_error(
            "[ServiceHandle] Cannot get property '" + propName + "' on a null handle.");
    }
    if (!dispatcher_) {
        throw std::runtime_error(
            "[ServiceHandle] No MethodDispatcher registered for property: '" + propName + "'");
    }
    return dispatcher_->getProperty(propName, instance_.get());
}

void ServiceHandle::set(const std::string& propName, std::any value) const
{
    if (!instance_) {
        throw std::runtime_error(
            "[ServiceHandle] Cannot set property '" + propName + "' on a null handle.");
    }
    if (!dispatcher_) {
        throw std::runtime_error(
            "[ServiceHandle] No MethodDispatcher registered for property: '" + propName + "'");
    }
    dispatcher_->setProperty(propName, instance_.get(), std::move(value));
}

} // namespace Container
