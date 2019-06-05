#ifndef OVEREDITOR_COLLECTION_UTILITY_H
#define OVEREDITOR_COLLECTION_UTILITY_H


#include <vector>
#include <overeditor/utility/string_utility.h>

#define LOG_VECTOR(header, v, indent_level) \
LOG_INFO << header;\
for (auto value : v) {\
LOG_INFO << INDENTATION(indent_level) << value;\
}

#define LOG_VECTOR_NO_HEADER(v, indent_level) \
for (auto value : v) {\
LOG_INFO << INDENTATION(indent_level) << value;\
}

#define LOG_VECTOR_WITH(header, v, indent_level, selector) \
LOG_INFO << header;\
for (auto& value : v) {\
LOG_INFO << INDENTATION(indent_level) << selector;\
}
namespace overeditor::utility::collection_utility {

    template<typename T>
    void add_range(const std::vector<T> &from, std::vector<T> &to) {
        to.reserve(from.size());
        to.insert(to.end(), from.begin(), from.end());
    }
}
#endif
