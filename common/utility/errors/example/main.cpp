#include <cassert>
#include <iostream>
#include <memory>

#include "error.h"

class CustomDbError : public common::errors::Error {
public:
    using common::errors::Error::Error;
};

int main() {
    using namespace common::errors;

    RegisterCoder(200201, "database disconnected", "https://docs.example/errors/db-disconnect");
    RegisterCoder(300201, "database unavailable", "https://docs.example/errors/db-unavailable");

    auto root = std::make_shared<CustomDbError>(200201, "mysql ping failed", nullptr, ERR_SOURCE_LOC);
    auto wrapped = Wrap(root, 300201, "create order failed", ERR_SOURCE_LOC);

    std::cout << "compact: " << FormatError(wrapped, FormatStyle::kCompact) << "\n\n";
    std::cout << "chain:   " << FormatError(wrapped, FormatStyle::kChain) << "\n\n";
    std::cout << "verbose:\n" << FormatError(wrapped, FormatStyle::kVerbose) << "\n";

    assert(Is(wrapped, 200201));
    assert(Is(wrapped, 300201));
    assert(Cause(wrapped)->Code() == 200201);

    std::shared_ptr<CustomDbError> out;
    assert(As<CustomDbError>(wrapped, out));
    assert(out != nullptr);

    std::cout << "\nerrors example passed.\n";
    return 0;
}
