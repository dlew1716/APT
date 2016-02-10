import numpy as np
from scipy.interpolate import RectBivariateSpline

x = np.array([1,2,3,4])
y = np.array([1,2,3,4,5])
vals = np.array([
    [4,1,4,4,2],
    [4,2,3,2,6],
    [3,7,4,3,5],
    [2,4,5,3,4]
])

rect_B_spline = RectBivariateSpline(x, y, vals)

a = np.array([3.2, 3.3, 3.5])
b = np.array([1.2, 2.5, 3.1])

print(rect_B_spline.ev(a, b))
print np.arange(0,10)