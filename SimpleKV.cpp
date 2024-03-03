#include "./SimpleKV.hpp"
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>
#include "SimpleKV.hpp"

using namespace std;

namespace simplekv {

// General Operations

vector<string> SimpleKV::namespaces() {
  // need to iterate through the kvstore and return all namespaces

  vector<string> res{};
  for (const auto& pair : kv_store) {
    res.push_back(pair.first);
  }
  return res;
}

vector<string> SimpleKV::keys(const string& nspace) {
  // we need to iterate through namespaces in kvstore and return all key names
  // in that specific namespace
  vector<string> res{};
  // for the namespace we are looking for, iterate through the kvstore and
  // return all keys in that namespace
  for (const auto& pair : kv_store) {
    // if that namespace is the one we are looking for, iterate through the keys
    // in that namespace and return all keys
    if (pair.first == nspace) {
      // iterate through the keys in that namespace and return all keys
      for (const auto& keypair : pair.second) {
        // add to our new list res
        res.push_back(keypair.first);
      }
    }
  }
  return res;
}

bool SimpleKV::ns_exists(const string& nspace) {
  //use the find function to see if we can find the nspace. If we can, then we return true (i.e the end function will return false). If not it will return false. 
    return kv_store.find(nspace) != kv_store.end();
}


bool SimpleKV::key_exists(const string& nspace, const string& key) {
  // iterate through kvstore and check if the key exists
  for (const auto& pair : kv_store) {
    // if the nspace is found
    if (pair.first == nspace) {
      // then iterate through the keys in the nspace
      for (const auto& keypair : pair.second) {
        // if we find the key, return true
        if (keypair.first == key) {
          return true;
        }
      }
    }
  }
  // otherwise, return false
  return false;
}

value_type_info SimpleKV::type(const std::string& nspace,
                               const std::string& key) {
  // use the find function to store an iter to the namespace
  auto first_iter = kv_store.find(nspace);
  // now check if the namespace exists
  if (first_iter == kv_store.end()) {
    // if the namespace doesn't exist, return none
    return value_type_info::none;
  }
  // search for the key in the namespace
  auto& key_map = first_iter->second;
  // use the find function to store an iter to the key
  auto key_iter = key_map.find(key);
  // if the key_iter is at the end of the key_map, return none
  if (key_iter == key_map.end()) {
    return value_type_info::none;
  }
  // if we do find the key, check if list or string
  if (holds_alternative<vector<string>>(key_iter->second)) {
    // if it is a list, then we return the list
    return value_type_info::list;
  }
  if (holds_alternative<string>(key_iter->second)) {
    // if it is a string, then we return the string
    return value_type_info::string;
  }
  // if the key is not a list or string, return none
  return value_type_info::none;
}

bool SimpleKV::del(const string& nspace, const string& key) {
  // iterate through our kvstore to get the namespace
  // instead of nested for loops lets try to use find
  auto first_iter = kv_store.find(nspace);
  // check to see if the namespace is at the end of the kv_store
  if (first_iter == kv_store.end()) {
    // if the namespace is at the end of the kv_store, then we return false
    return false;
  }
  // if the namespace is found, then find the key
  auto& key_map = first_iter->second;
  auto key_iter = key_map.find(key);

  // if the key is not found, then return false
  if (key_iter == key_map.end()) {
    return false;
  }
  // if the key is found, then we delete the key
  key_map.erase(key_iter);
  // if after erasing the key, our namespace is empty, we should delete the
  // namespace
  if (key_map.empty()) {
    kv_store.erase(first_iter);
  }
  // return true if the key was deleted
  return true;
}

// string operations

optional<string> SimpleKV::sget(const string& nspace, const string& key) {
  // iterate through our kvstore to get the namespace
  for (const auto& pair : kv_store) {
    if (pair.first == nspace) {
      // then iterate through the keys in that namespace
      for (const auto& keypair : pair.second) {
        // if we find the key, then return the value
        if (keypair.first == key) {
          if (std::holds_alternative<std::string>(keypair.second)) {
            return std::get<std::string>(keypair.second);
          }
          return nullopt;
        }
      }
    }
  }
  return nullopt;
}

void SimpleKV::sset(const string& nspace,
                    const string& key,
                    const string& value) {
  // use the find function to store an iter to the namespace
  auto first_iter = kv_store.find(nspace);
  // if the namespace is not at the end of the kv_store then we can continue
  if (first_iter != kv_store.end()) {
    // this means that we've found the namespace, so we should check to see if
    // the key exists
    auto& key_map = first_iter->second;
    // use the find function to store an iter to the key
    auto key_iter = key_map.find(key);
    if (key_iter != key_map.end()) {
      // if the key is found, then we set the value
      key_iter->second = value;
    } else {
      // otherwise we add the key and value to the namespace
      key_map[key] = value;
    }
  } else {
    // otherwase we need to create a new namespace and key-value pair
    kv_store[nspace] = {{key, value}};
  }
}

// list operations

ssize_t SimpleKV::llen(const string& nspace, const string& key) {
  // iterate through our kvstore to get the namespace
  for (const auto& pair : kv_store) {
    if (pair.first == nspace) {
      // then lets find the key in that nspace
      for (const auto& keypair : pair.second) {
        if (keypair.first == key) {
          // if we find the key then we return the size of the associated list
          // and -1 if its a string
          if (holds_alternative<vector<string>>(keypair.second)) {
            return static_cast<ssize_t>(
                get<vector<string>>(keypair.second).size());
          }
          return -1;
        }
      }
    }
  }
  return -1;
}

optional<string> SimpleKV::lindex(const string& nspace,
                                  const string& key,
                                  size_t index) {
  // use the find function to store an iter to the namespace
  auto first_iter = kv_store.find(nspace);
  // if the namespace is not at the end of the kv_store then we can continue
  if (first_iter == kv_store.end()) {
    // if the namespeace is not found, return nullopt
    return std::nullopt;
  }
  // search for the key in the namespace
  auto& key_map = first_iter->second;
  // use the find function to store an iter to the key
  auto key_iter = key_map.find(key);
  if (key_iter == key_map.end()) {
    // if the key ism t found, return nullopt
    return std::nullopt;
  }
  // if we do find the key, check if list
  if (holds_alternative<vector<string>>(key_iter->second)) {
    // get the list and store it in a const
    const auto& list = get<vector<string>>(key_iter->second);
    if (index < list.size()) {
      // return the value at the specified index
      return list[index];
    }
  }
  // if the key is not a list or the index is out of bounds, return nullopt
  return std::nullopt;
}

optional<vector<string>> SimpleKV::lmembers(const string& nspace,
                                            const string& key) {
  // iterate through our kvstore to get the namespace
  for (const auto& pair : kv_store) {
    if (pair.first == nspace) {
      // then lets find the key in that nspace
      for (const auto& keypair : pair.second) {
        if (keypair.first == key) {
          // if we find the key, then we check to see if it is a list
          if (std::holds_alternative<std::vector<std::string>>(
                  keypair.second)) {
            // if it is a list, then we return the list
            return std::get<std::vector<std::string>>(keypair.second);
          }
          // otherwise we return nullopt
          return nullopt;
        }
      }
    }
  }
  return nullopt;
}

bool SimpleKV::lset(const string& nspace,
                    const string& key,
                    size_t index,
                    const string& value) {
  // if the index is negative, then we return false
  if (index < 0) {
    return false;
  }
  // iterate through our kvstore to get the namespace
  for (auto& pair : kv_store) {
    if (pair.first == nspace) {
      // lets fine the key in that namespace
      for (auto& keypair : pair.second) {
        if (keypair.first == key) {
          // then if we have the key, let's check to see if it is a list
          if (std::holds_alternative<std::vector<std::string>>(
                  keypair.second)) {
            // if it is a list, then we check to see if the index is in bounds
            if (index <
                std::get<std::vector<std::string>>(keypair.second).size()) {
              // if it is in bounds, then we set the value at that index
              std::get<std::vector<std::string>>(keypair.second).at(index) =
                  value;
              return true;
            } 
              // otherwise we return false
              return false;
          }
          // otherwise we return false
          return false;
        }
      }
    }
  }

  return false;
}

bool SimpleKV::lpush(const string& nspace,
                     const string& key,
                     const string& value) {
  // use the find function to get the namespace and set it to an iter
  auto nspace_iter = kv_store.find(nspace);
  // if the namespace is not at the end of the kv_store then we can continue
  if (nspace_iter != kv_store.end()) {
    // we've found the namespace, so let's look for the key using find, first
    // let's set the key_map to a reference
    auto& key_map = nspace_iter->second;
    auto key_iter = key_map.find(key);
    // if the key is not at the end of the key_map then we can continue
    if (key_iter != key_map.end()) {
      if (holds_alternative<vector<string>>(key_iter->second)) {
        // get the list
        get<vector<string>>(key_iter->second)
            .insert(get<vector<string>>(key_iter->second).begin(), value);
        return true;
      }
        // the key must exist but it isn't a list (its a string)
        return false;
    }
    // otherwise the key doesn't exist, so we create a list
    key_map[key] = vector<string>{value};
  }
  // now if the namespace doesn't exist we have to create a new namespace, key
  // and list
  else {
    kv_store[nspace] = {{key, vector<string>{value}}};
  }
  return true;
}

optional<string> SimpleKV::lpop(const string& nspace, const string& key) {
  // trying to use the find function to find the namespace and store it in a
  // iter
  auto first_iter = kv_store.find(nspace);
  // if the namespace is not at the end of the kv_store then we can continue
  if (first_iter != kv_store.end()) {
    auto& key_map = first_iter->second;
    // trying to use the find function to find the key and store it in another
    // iter
    auto second_iter = key_map.find(key);
    // if the key is not at the end of the key_map then we can continue
    if (second_iter != key_map.end() &&
        holds_alternative<std::vector<std::string>>(second_iter->second)) {
      // get the list
      auto& list = get<std::vector<std::string>>(second_iter->second);
      // if the list is empty, pop the value and erase the key
      if (!list.empty()) {
        // pop the value
        string popValue = list.front();
        // erase the key
        list.erase(list.begin());
        // if the list is empty, erase the key
        if (list.empty()) {
          key_map.erase(second_iter);
          if (key_map.empty()) {
            kv_store.erase(first_iter);
          }
        }
        return popValue;
      }
    }
  }
  return std::nullopt;
}

bool SimpleKV::rpush(const string& nspace,
                     const string& key,
                     const string& value) {
  // lets use find to find the namespace
  auto first_iter = kv_store.find(nspace);
  // if the namespace is not at the end of the kv_store then we can continue
  if (first_iter != kv_store.end()) {
    auto& key_map = first_iter->second;
    // now lets find the key in that key_map and store it in another iter
    auto second_iter = key_map.find(key);
    // if the key exists, and the value is a list, then we can continue
    if (second_iter != key_map.end()) {
      if (holds_alternative<vector<string>>(second_iter->second)) {
        // get the list
        get<vector<string>>(second_iter->second).push_back(value);
        return true;
        // if the list is empty, push the value and erase the key
      }
      // the key must exist but it isn't a list
      return false;
    }
    // the key doesn't exist, so we create a list and push the value
    key_map[key] = vector<string>{value};
    return true;
  }
  // the namespace doesn't exist, so we create a new namespace, key, and list
  kv_store[nspace] = {{key, vector<string>{value}}};
  return true;
}

optional<string> SimpleKV::rpop(const string& nspace, const string& key) {
  // lets use the find function and store that on an iter
  auto first_iter = kv_store.find(nspace);
  // if that nspace isn't at the back of the kv_store then we can continue
  if (first_iter != kv_store.end()) {
    auto second_iter = first_iter->second.find(key);
    // if the key is not at the back of the key_map then we can continue
    if (second_iter != first_iter->second.end() &&
        holds_alternative<vector<string>>(second_iter->second)) {
      // get the list
      auto& list = get<vector<string>>(second_iter->second);
      // if the list is not empty, pop the value and erase the key
      if (!list.empty()) {
        // pop the value
        string pop = list.back();
        // erase the key
        list.pop_back();
        // if the list is empty, erase the key
        if (list.empty()) {
          first_iter->second.erase(second_iter);
        }
        // if the namespace would also be empty, erase the namespace
        if (first_iter->second.empty()) {
          kv_store.erase(first_iter);
          if (kv_store.empty()) {
            kv_store.erase(first_iter);
          }
        }
        return pop;
      }
      // if either the namespace or the key is not found, return nullopt
      return nullopt;
    }
  }
  return nullopt;
}

optional<vector<string>> SimpleKV::lunion(const string& nspace1,
                                          const string& key1,
                                          const string& nspace2,
                                          const string& key2) {
  // check the types of the keys and if they are strings, return nullopt
  auto type1 = type(nspace1, key1);
  auto type2 = type(nspace2, key2);
  if (type1 == value_type_info::string || type2 == value_type_info::string) {
    return nullopt;
  }
  // now that we know that they are not strings, we can get the lists
  auto list1 = lmembers(nspace1, key1).value_or(vector<string>{});
  auto list2 = lmembers(nspace2, key2).value_or(vector<string>{});
  // declare a set for unique values across the two lists
  unordered_set<string> unionSet(list1.begin(), list1.end());
  // insert the values of list2 in the set
  unionSet.insert(list2.begin(), list2.end());
  // declare a vector list to store the union set, and put in the unionSet
  vector<string> unionList(unionSet.begin(), unionSet.end());
  return unionList;
}

optional<vector<string>> SimpleKV::linter(const string& nspace1,
                                          const string& key1,
                                          const string& nspace2,
                                          const string& key2) {
  // using the lmembers function to get the lists
  auto list1 = lmembers(nspace1, key1);
  auto list2 = lmembers(nspace2, key2);
  // check that both lists exist and that they are both lists
  if (!list1 || !list2 || type(nspace1, key1) != value_type_info::list ||
      type(nspace2, key2) != value_type_info::list) {
    return nullopt;
  }

  // use an unordered set to get unique values across the two lists
  unordered_set<string> interSet(list2->begin(), list2->end());
  // declare a set to keep track the added values that we can check against for
  // duplicates
  unordered_set<string> addedSet;
  vector<string> interList;

  for (const auto& value : *list1) {
    if (interSet.find(value) != interSet.end() &&
        addedSet.insert(value).second) {
      // only add to our final list if its found in both and hasn't alreayd been
      // added
      interList.push_back(value);
    }
  }
  return interList;
}

optional<vector<string>> SimpleKV::ldiff(const string& nspace1,
                                         const string& key1,
                                         const string& nspace2,
                                         const string& key2) {
  // check the types of the keys and if they are strings, return nullopt
  auto type1 = type(nspace1, key1);
  auto type2 = type(nspace2, key2);
  if (type1 == value_type_info::string || type2 == value_type_info::string) {
    return nullopt;
  }
  // now that we know that they are not strings, we can get the lists
  auto list1 = lmembers(nspace1, key1).value_or(vector<string>{});
  auto list2 = lmembers(nspace2, key2).value_or(vector<string>{});

  unordered_set<string> diffSet(list2.begin(), list2.end());
  vector<string> diffList;

  // store the elements that we've already added so that we can handle
  // duplicates
  unordered_set<string> addedSet;

  // iterate through list1 and add the elements that are not in list2 to
  // diffList
  for (const auto& value : list1) {
    if (diffSet.find(value) == diffSet.end() && addedSet.insert(value).second) {
      diffList.push_back(value);
    }
  }
  return diffList;
}
}  // namespace simplekv
// namespace simplekv
