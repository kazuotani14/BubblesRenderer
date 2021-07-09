import numpy as np
import matplotlib
matplotlib.use('TKAgg')

import matplotlib.pyplot as plt
from matplotlib import animation

from particle_data import box_size, num_particles, data

fig = plt.figure()
border = 2
ax = plt.axes(xlim=(-border, box_size+border), ylim=(-border, box_size+border))
line, = ax.plot([], [], '.', markersize=15)

# initialization function: plot the background of each frame
def init():
    line.set_data([], [])
    return line,

# animation function.  This is called sequentially
def animate(i):
    start_idx = num_particles * i
    end_idx = start_idx + num_particles
    line.set_data(data[start_idx : end_idx, 0], data[start_idx : end_idx, 1])
    return line,

# call the animator.  blit=True means only re-draw the parts that have changed.
anim = animation.FuncAnimation(fig, animate, init_func=init,
                               interval=10, repeat=True)

plt.show()
