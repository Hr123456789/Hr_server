//配置模块

#ifndef __SYLAR_CONFIG_H__
#define __SYLAR_CONFIG_H__

#include <memory>
#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <functional>

#include "thread.h"
#include "log.h"
#include "util.h"


namespace hr {

//配置变量的基类
class ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;

    //构造函数
    // name 配置参数名称 [0-9 a-z]
    // description 配置参数描述
    ConfigVarBase(const std::string& name, const std::string& description)
        :m_name(name)
        ,m_description(description) {
        //将name全部变成小写
        std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
    }

    //析构函数
    virtual ~ConfigVarBase() {}

    //返回配置参数名称
    const std::string& getName() const {return m_name;}

    //返回配置参数的描述
    const std::string& getDescription () const {return m_description;}

    //转成字符串
    virtual std::string toString() = 0;

    //从字符串初始化值
    virtual bool fromString(const std::string& val) = 0;

    //返回配置参数值的类型名称
    virtual std::string getTypeName() const = 0;

protected:
    //配置参数的名称
    std::string m_name;
    //配置残数的描述
    std::string m_description;
};

//类型转换模板类(F 源类型， T 目标类型)
template<class F, class T>
class LexicalCast {
public:
    //类型转换
    // v 源类型值
    // 返回v转换后的目标类型
    // 当类型不可转换时抛出异常
    T operator()(const F& v) {
        return boost::lexical_cast<T>(v);
    }
};

//类型转换模板类偏特化(YAML String 转换成std::vector<T>)
template<class T>
class LexicalCast<std::string, std::vector<T>> {
public:
    std::vector<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::vector<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss <<  node[i];
            //?????
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec; 
    }
};

//类型转换模板偏特化(std::vector<T>转换成YAML String)
template<class T>
class LexicalCast<std::vector<T>,std::string> {
public:
    std::string operator()(const std::vector<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

//类型转换模板类偏特化(YAML String 转换成std::list<T>)
template<class T>
class LexicalCast<std::string, std::list<T>> {
public:
    std::list<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::list<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

//类型转换模板偏特化(std::list<T> 转换成YAML String)
template<class T>
class LexicalCast<std::list<T>, std::string> {
public:
    std::string operator()(const std::list<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

//类型转换模板偏特化(YAML String 转换成std::set<T>)
template<class T>
class LexicalCast<std::string, std::set<T>> {
public:
    std::set<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::set<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

//类型转换模板偏特化(std::set<T> 转换成 YAML String)
template<class T>
class LexicalCast<std::set<T>, std::string> {
public:
    std::string operator()(const std::set<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

//类型转换模板偏特化(YAML String 转换成std::unorded_set<T>)
template<class T>
class LexicalCast<std::string, std::unordered_set<T>> {
public:
    std::unordered_set<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_set<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

//类型转换模板偏特化(std::unorded_set<T> 转换成 YAML String)
template<class T>
class LexicalCast<std::unordered_set<T>, std::string> {
public:
    std::string operator()(const std::unordered_set<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

//类型转换模板偏特化(YAML:: String 转换成 std::map<std::string, T>)
template<class T>
class LexicalCast<std::string, std::map<std::string, T>> {
public:
    std::map<std::string, T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::map<std::string, T> vec;
        std::stringstream ss;
        for(auto it = node.begin();
                it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(),
                        LexicalCast<std::string, T>()(ss.str())));
        }
        return vec;
    }
};

//类型转换模板偏特化(std::map<std::string, T> 转换成 YAML String)
template<class T>
class LexicalCast<std::map<std::string, T>, std::string> {
public:
    std::string operator()(const std::map<std::string, T>& v) {
        YAML::Node node(YAML::NodeType::Map);
        for(auto& i : v) {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

//类型转换模板偏特化(YAML:: String 转换成 std::unordered_map<std::string, T>)
template<class T>
class LexicalCast<std::string, std::unordered_map<std::string, T>> {
public:
    std::unordered_map<std::string, T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_map<std::string, T> vec;
        std::stringstream ss;
        for(auto it = node.begin();
                it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(),
                        LexicalCast<std::string, T>()(ss.str())));
        }
        return vec;
    }
};

//类型转换模板偏特化(std::unordered_map<std::string, T> 转换成 YAML String)
template<class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
public:
    std::string operator()(const std::unordered_map<std::string, T>& v) {
        YAML::Node node(YAML::NodeType::Map);
        for(auto& i : v) {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

//配置参数模板子类， 保存对应类型的参数值
// T 参数的具体类型
//  FromStr 从std::string转换成T类型的仿函数
//  ToStr 从T转换成std::string的仿函数
//  std::string 为YAML格式的字符串
template<class T, class FromStr = LexicalCast<std::string, T>
                ,class ToStr = LexicalCast<T, std::string>>
class ConfigVar : public ConfigVarBase {
public:
    typedef RWMutex RWMutexType;
    typedef std::shared_ptr<ConfigVar> ptr;
    typedef std::function<void (const T& old_value, const T& new_value)> on_change_cb;

    //通过参数名，参数值，描述构造ConfigVar
    // name 参数名称有效字符为[0-9 a-z]
    // default_value 参数的默认值
    ConfigVar(const std::string& name
            ,const T& default_value
            ,const std::string& description = "")
        :ConfigVarBase(name, description)
        ,m_val(default_value) {
    }

    //将数值转换成YAML String
    //当转换失败抛异常
    std::string toString() override {
        try {
            RWMutexType::ReadLock lock(m_mutex);
            return ToStr()(m_val);
        } catch (std::exception& e) {
            HR_LOG_ERROR(HR_LOG_ROOT()) << "ConfigVar::toString exception "
                << e.what() << " convert: " << TypeToName<T>() << " to string"
                << " name=" << m_name;
        }
        return "";
    }

    //从YAML String 转成参数的值
    // 当转换失败抛出异常
    bool fromString(const std::string& val) override {
        try {
            setValue(FromStr()(val));
        } catch (std::exception& e) {
            HR_LOG_ERROR(HR_LOG_ROOT()) << "ConfigVar::fromString exception "
                << e.what() << " convert: string to " << TypeToName<T>()
                << " name=" << m_name
                << " - " << val;
        }
        return false;
    }

    //获取当前参数的值
    const T getValue() {
        RWMutexType::ReadLock lock(m_mutex);
        return m_val;
    }

    //设置当前参数的值
    //如果参数的值有发生变化，则通知对应的注册回调函数
    void setValue(const T& v) {
        {
            RWMutexType::ReadLock lock(m_mutex);
            if(v == m_val) {
                return;
            }
            for(auto& i : m_cbs) {
                i.second(m_val, v);
            }
        }
        RWMutexType::WriteLock lock(m_mutex);
        m_val = v;
    }

    //返回参数值的类型名称
    std::string getTypeName() const override {return TypeToName<T>();}

    //添加变化回调函数
    //返回该回调函数对应的唯一id，用于删除回调
    uint64_t addListener(on_change_cb cb) {
        static uint64_t s_fun_id = 0;
        RWMutexType::WriteLock lock(m_mutex);
        ++s_fun_id;
        m_cbs[s_fun_id] = cb;
        return s_fun_id;
    }

    //删除回调函数
    // key 回调函数的唯一id
    void delListener(uint64_t key) {
        RWMutexType::WriteLock lock(m_mutex);
        m_cbs.erase(key);
    }

    //获取回调函数
    // key 回调函数的唯一id
    // 如果参在返回对应的回调函数，否则返回nullptr
    on_change_cb getListener(uint64_t key) {
        RWMutexType::ReadLock lock(m_mutex);
        auto it = m_cbs.find(key);
        return it == m_cbs.end() ? nullptr : it->second;
    }

    //清理所有的回调函数
    void clearListener() {
        RWMutexType::WriteLock lock(m_mutex);
        m_cbs.clear();
    }

private:
    RWMutexType m_mutex;
    T m_val;
    //变更回调函数组， uint64_t key, 要求唯一， 一般可以用hash
    std::unordered_map<uint64_t, on_change_cb> m_cbs;
};

class Config {
public:
    typedef std::unordered_map<std::string, ConfigVarBase::ptr> ConfigVarMap;
    typedef RWMutex RWMutexType;

    //获取/创建对应参数名的配置名称
    // name 配置参数名称
    // deafult_value 参数默认值
    // description 参数描述
    /*
    获取参数名为name的配置参数，如果存在直接返回
    如果不存在，创建参数配置并用deafult_value赋
    */ 
    // return 返回对应配置参数，如果参数名存在但是类型不匹配则返回Nullptr
    // 如果参数名包含非法字符[0-9 a-z] 抛出异常std::invalid_argument
    template <class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name,
            const T& deafult_value, const std::string& description = "") {
        RWMutexType::WriteLock lock(GetMutex());
        auto it = GetDatas().find(name);
        if(it != GetDatas().end()) {
            auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
            if(tmp) {
                HR_LOG_INFO(HR_LOG_ROOT()) << "Lookup name=" << name << " exists";
                return tmp;
            } else {
                HR_LOG_ERROR(HR_LOG_ROOT()) << "Lookup name=" << name << " exists but type not "
                        << TypeToName<T>() << " real_type=" << it->second->getTypeName()
                        << " " << it->second->toString();
                return nullptr;
            }
        }

        //参数非法
        if(name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678")
                != std::string::npos) {
            HR_LOG_ERROR(HR_LOG_ROOT()) << "Lookup name invalid " << name;
            throw std::invalid_argument(name);
        }

        //如果不存在则创建
        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, deafult_value, description));
        GetDatas()[name] = v;
        return v;
    }

    //查找配置参数
    // name  配置参数名称
    // 返回配置参数名为name de配置参数
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name) {
        RWMutexType::ReadLock lock(GetMutex());
        auto it = GetDatas().find(name);
        if(it == GetDatas().end()) {
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
    }

    //使用YAML::Node初始化配置模块
    static void LoadFromYaml(const YAML::Node& root);

    //加载path文件夹里面的配置文件
    static void LoadFromConfDir(const std::string& path, bool force = false);

    //查找配置参数，返回配置参数的基类
    // name 配置参数名称
    static ConfigVarBase::ptr LookupBase(const std::string& name);

    //遍历配置模块里面所有配置项
    // cb 配置项回调函数
    static void Visit(std::function<void(ConfigVarBase::ptr)> cb);

private:
    //返回所有的配置项
    static ConfigVarMap& GetDatas() {
        static ConfigVarMap s_datas;
        return s_datas;
    }

    //配置项的RWMutex
    static RWMutexType& GetMutex() {
        static RWMutexType s_mutex;
        return s_mutex;
    }
};

}

#endif