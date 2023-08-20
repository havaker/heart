import os
import sympy

from sympy import plotting, sin, cos
from sympy.utilities.codegen import codegen

u, v = sympy.symbols('u v', real=True)

# Julias's parametric heart surface equation (ref
# https://math.stackexchange.com/a/3935220):
x = sin(v) * (15 * sin(u) - 4 * sin(3 * u))
y = 8 * cos(v)
z = sin(v) * (15 * cos(u) - 5 * cos(2 * u) - 2 * cos(3 * u) - cos(4 * u))

# Convert the above equations to a single vector equation:
r = sympy.Matrix([x, y, z])

# Compute the partial derivatives of r:
r_u = r.diff(u)
r_v = r.diff(v)

# Compute the cross product of the partial derivatives:
n = r_u.cross(r_v)

# Normalize the cross product:
n = n / n.norm()

n = sympy.simplify(n)

# Generate C code for the **** surface using the codegen() function:
codegen(
    (("surface", r), ("normal", n)),
    language="C99",
    project="renderer",
    to_files=True,
)

# Plot if env var PLOT is set:
if "PLOT" in os.environ:
    plotting.plot3d_parametric_surface(
        x, y, z, (u, 0, 2 * sympy.pi), (v, 0, sympy.pi)
    )
