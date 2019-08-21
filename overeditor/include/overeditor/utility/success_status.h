#ifndef OVEREDITOR_SUCCESS_STATUS_H
#define OVEREDITOR_SUCCESS_STATUS_H

#include <string>
#include <vector>
#include <plog/Log.h>

namespace overeditor {
    class SuccessStatus {
    private:
        std::vector<std::string> errors;
    public:
        SuccessStatus() : errors() {}

        SuccessStatus(std::initializer_list<std::string> initialErrors) : errors(initialErrors) {
        }

        const bool isSuccessful() const {
            return errors.empty();
        }

        const std::vector<std::string> &getErrors() const {
            return errors;
        }

        void addError(const std::string &error) {
            errors.push_back(error);
        }

        void printErrors() {
            if (errors.empty()) {
                LOG_INFO << "No errors.";
                return;
            }
            LOG_ERROR << "Errors (" << errors.size() << "):";
            for (const std::string &str : errors) {
                LOG_ERROR << "* " << str;
            }
        }

    };
}

#endif
