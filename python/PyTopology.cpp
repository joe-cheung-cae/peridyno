#include "PyTopology.h"

void pybind_topology(py::module& m)
{
	declare__height_field_to_triangle_set<dyno::DataType3f>(m, "3f");
}