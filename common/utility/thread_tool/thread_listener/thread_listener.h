#ifndef __COMMON_UTILITY_THREAD_LISTENER_H__
#define __COMMON_UTILITY_THREAD_LISTENER_H__

#include <map>
#include <string>
#include <future>
#include <functional>
#include <3rdparty/libhv/include/hv/hthreadpool.h>
#include <common/utility/errors/include/error.h>

namespace common
{

namespace utility
{

// 定义结果集类型
using ResultSet = std::map<std::string, common::errors::ErrorPtr>;

// 定义结果异步回调函数类型
using ResultCBFunc = std::function<void(bool bigResult, ResultSet resultSet)>;

class ThreadListener {
public:
    ThreadListener() = default;
    ~ThreadListener() = default;
public:
    void Start(int minTaskNum, int maxTaskNum) {
        threadPool_.setMinThreadNum(minTaskNum);
        threadPool_.setMaxThreadNum(maxTaskNum);
        threadPool_.start();
    }

    void WaitStop() {
        threadPool_.wait();
        threadPool_.stop();
    }

    void Stop() {
        threadPool_.stop();
    }

    // this API must be called before AsyncWait otherwise you use the SyncWait
    void RegisterResultCB(ResultCBFunc resultCBFunc) {
        this->resultCBFunc_ = resultCBFunc;
    }

    /*
        * Commit(fn, args...)
        * Commit(std::bind(&Class::mem_fn, &obj))
    */
    template<class Fn, class... Args>
    void CommitTask(const std::string &taskName, Fn&& fn, Args&& ...args) {
        TaskFutureMap_.emplace(std::make_pair(taskName, threadPool_.commit(std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...))));
    }

    // will wait all task done and return big result(one of tasks failed, result = false)
    bool SyncWait(ResultSet &resultSet) {
        bool bret = wait(resultSet);
        return bret;
    }

    // will return just now, all task done will callback your register API and provide result
    bool AsyncWait() {
        threadPool_.commit(std::bind((bool(ThreadListener::*)()) &ThreadListener::wait, this));
        return true;
    }

private:
    bool wait(ResultSet &resultSet) {
        bool bret = true;
        for (auto iter = TaskFutureMap_.begin(); iter != TaskFutureMap_.end(); ++iter) {
            common::errors::ErrorPtr r = iter->second.get();
            resultSet.insert(std::make_pair(iter->first, r));
            if (r != nullptr) {
                bret = false;
            }
        }
        TaskFutureMap_.clear();
        return bret;
    }

    bool wait() {
        bool bret = true;
        ResultSet resultSet;
        for (auto iter = TaskFutureMap_.begin(); iter != TaskFutureMap_.end(); ++iter) {
            common::errors::ErrorPtr r = iter->second.get();
            resultSet.insert(std::make_pair(iter->first, r));
            if (r != nullptr) {
                bret = false;
            }
        }
        TaskFutureMap_.clear();
        
        // callback ... 
        if (resultCBFunc_) {
            resultCBFunc_(bret, resultSet);
        }

        return bret;
    }
private:
    HThreadPool threadPool_;
    std::map<std::string, std::future<common::errors::ErrorPtr>> TaskFutureMap_;
    ResultCBFunc resultCBFunc_ = nullptr;
};

} // end of namespace utility

}

#endif
