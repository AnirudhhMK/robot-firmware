import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque
import threading
import uart_reader
# Parameters
x_len = 200         # Number of points to display
y_range = [-180, 180]  # Range of possible Y values to display

N = x_len

gyro = deque([0.0] * N, maxlen=N)
accel = deque([0.0] * N, maxlen=N)
comp = deque([0.0] * N, maxlen=N)
uart_reader.init_worker(gyro,accel,comp)
# Create figure for plotting
fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
xs = list(range(0, x_len))
ys = [0] * x_len
ax.set_ylim(y_range)

# Initialize communication with TMP102

# Create a blank line. We will update the line in animate
gyro_line, = ax.plot(xs, gyro, label="Gyro")
accel_line, = ax.plot(xs, accel, label="Accel")
comp_line, = ax.plot(xs, comp, label="Complementary")

ax.legend()
# Add labels
plt.title('Angle estimates over time')
plt.xlabel('Time')
plt.ylabel('Angle in degrees')

# This function is called periodically from FuncAnimation
def animate(frame):
    gyro_line.set_ydata(gyro)
    accel_line.set_ydata(accel)
    comp_line.set_ydata(comp)
    return gyro_line, accel_line, comp_line# Set up plot to call animate() function periodically

ani = animation.FuncAnimation(fig,
    animate,
    interval=50,
    blit=True)

thread = threading.Thread(target=uart_reader.uart_worker,daemon=True,name="telemetry worker")
thread2 = threading.Thread(target=uart_reader.send_command,daemon=True,name="commands worker")
thread.start()
thread2.start()

plt.show()
