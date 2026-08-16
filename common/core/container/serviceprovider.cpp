#include "serviceprovider.h"
#include "application.h"

namespace Container {

ServiceProvider::ServiceProvider(Application& app)
    : app_(app)
{
}

} // namespace Container
