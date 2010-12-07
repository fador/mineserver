#ifndef _KIT_H
#define _KIT_H

#include <string>
#include <vector>

struct Kit {
  std::string name;
  std::vector<int> items;
  int permissions;

  Kit(std::string name, std::vector<int> items, int permissions)
  : name(name),
    items(items),
    permissions(permissions)
  {}
};

#endif /* _KIT_H */
