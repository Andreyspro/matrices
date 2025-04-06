#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

std::string new_uuid()
{
    boost::uuids::random_generator gen;
    return boost::uuids::to_string(gen()).substr(0,6);
}