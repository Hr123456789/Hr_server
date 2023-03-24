#include "../sylar/sylar.h"

hr::ConfigVar<int>::ptr g_int_value_config =
    hr::Config::Lookup("system.port", (int)8080, "system port");

// hr::ConfigVar<float>::ptr g_int_valuex_config =
//     hr::Config::Lookup("system.port", (float)8080, "system port");

hr::ConfigVar<float>::ptr g_float_value_config =
    hr::Config::Lookup("system.value", (float)10.2f, "system value");

hr::ConfigVar<std::vector<int> >::ptr g_int_vec_value_config =
    hr::Config::Lookup("system.int_vec", std::vector<int>{1,2}, "system int vec");

hr::ConfigVar<std::list<int> >::ptr g_int_list_value_config =
    hr::Config::Lookup("system.int_list", std::list<int>{1,2}, "system int list");

hr::ConfigVar<std::set<int> >::ptr g_int_set_value_config =
    hr::Config::Lookup("system.int_set", std::set<int>{1,2}, "system int set");

hr::ConfigVar<std::unordered_set<int> >::ptr g_int_uset_value_config =
    hr::Config::Lookup("system.int_uset", std::unordered_set<int>{1,2}, "system int uset");

hr::ConfigVar<std::map<std::string, int> >::ptr g_str_int_map_value_config =
    hr::Config::Lookup("system.str_int_map", std::map<std::string, int>{{"k",2}}, "system str int map");

hr::ConfigVar<std::unordered_map<std::string, int> >::ptr g_str_int_umap_value_config =
    hr::Config::Lookup("system.str_int_umap", std::unordered_map<std::string, int>{{"k",2}}, "system str int map");


void print_yaml(const YAML::Node& node, int level) {
    if(node.IsScalar()) {
        HR_LOG_INFO(HR_LOG_ROOT()) << std::string(level * 4, ' ')
            << node.Scalar() << " - " << node.Type() << " - " << level;
    } else if(node.IsNull()) {
        HR_LOG_INFO(HR_LOG_ROOT()) << std::string(level * 4, ' ')
            << "NULL - " << node.Type() << " - " << level;
    } else if(node.IsMap()) {
        for(auto it = node.begin();
                it != node.end(); ++it) {
            HR_LOG_INFO(HR_LOG_ROOT()) << std::string(level * 4, ' ')
                    << it->first << " - " << it->second.Type() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    } else if(node.IsSequence()) {
        for(size_t i = 0; i < node.size(); ++i) {
            HR_LOG_INFO(HR_LOG_ROOT()) << std::string(level * 4, ' ')
                << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}

void test_yaml() {
    YAML::Node root = YAML::LoadFile("../bin/config/log.yml");
    print_yaml(root, 0);
}

int main(void) {
    test_yaml();
    return 0;
}