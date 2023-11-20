import time
import pygame
from pygame.locals import *
from math import pi, sin, cos, sqrt, atan2
import serial
import serial.tools.list_ports
from datetime import datetime


WINDOW_SIZE = (800, 800) # px
SCALE = 0.3 # px / mm

BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
BLUE = (0, 255, 255)
RED = (255, 127, 127)
GREEN = (127, 255, 127)

ROBOT_RADIUS = 100 # mm
SENSOR_ANGLES = [pi, pi*3/4, pi/2, pi/4, 0, -pi/2]
KNOWN_VALUES = [0] * 6
OLD_VALUES = [0] * 6
READINGS = []

ORIGIN = (WINDOW_SIZE[0]//2, WINDOW_SIZE[1]//2)

FLOOR = False

X = 0
Y = 0
BEARING = 0

ROTATION_AMOUNT = 10
ROLL_AMOUNT = 50
SAFETY = False

ports = serial.tools.list_ports.comports()
port = ""
match len(ports):
    case 0:
        print("No comport found!")
        # exit()
    case 1:
        port = ports[0].device
    case _:
        print("Select port:")
        for i, port in enumerate(ports):
            print(f"{i} {port.device}")
        idx = int(input(">").strip())
        port = ports[idx].device

print(f"Using port {port}")

# serialPort = serial.Serial(port=port, baudrate=9600, timeout=0, parity=serial.PARITY_EVEN, stopbits=1)
try:
    serialPort = serial.Serial(port=port, timeout=0, baudrate=9600)
except Exception as e:
    print(e)

def to_global(local_x, local_y):
    global X, Y, BEARING
    c = cos(pi / 180 * BEARING)
    s = sin(pi / 180 * BEARING)
    dxg = c * local_x - s * local_y
    dyg = s * local_x + c * local_y
    return dxg + X, dyg + Y

def to_local(global_x, global_y):
    global X, Y, BEARING
    rx = global_x - X
    ry = global_y - Y
    c = cos(-pi / 180 * BEARING)
    s = sin(-pi / 180 * BEARING)
    x = c * rx - s * ry
    y = s * rx + c * ry
    return x, y

def r_theta(x, y):
    dx, dy = x - WINDOW_SIZE[0]/2, y - WINDOW_SIZE[1]/2
    r = sqrt(dx*dx + dy*dy)
    theta = -atan2(dy, dx)
    return r, theta

def format_int(x):
    if x < -999 or x > 999:
        raise Exception(f"{x} is out of bounds")
    str = "+" if x >= 0 else "-"
    str += f"{int(abs(x)):03d}"
    return str

def format_command(name, a=None, b=None):
    comm = name
    if a != None:
        comm += format_int(a)
    else:
        comm += "...."
    if b != None:
        comm += format_int(b)
    else:
        comm += "...."
    comm += ";"
    return comm.encode("ascii")

def command_turn(degrees):
    return format_command("TURN", degrees)

def command_drive(mm):
    return format_command("DRIV", mm)

def command_ultra(which):
    return format_command("ULTR", which)

def command_halt():
    return format_command("HALT")

gripper = False
def command_toggle_gripper():
    global gripper
    gripper = not gripper
    return format_command("GRIP", -90 if gripper else 0, 0)


def command_set_safety(safety):
    if safety:
        return format_command("SAFE", 1)
    else:
        return format_command("SAFE", 0)

def command_scan(step_size=6):
    return format_command("SCAN", step_size)

def handle_quit():
    pygame.quit()
    exit()


def handle_halt():
    serial_send(b"............;")
    serial_send(command_halt)

def handle_clear():
    global READINGS
    READINGS.clear()

def parse_range(string):
    global KNOWN_VALUES, READINGS
    which = int(ord(string[3]) - ord('0'))
    dx = int(string[4:8])
    dy = int(string[8:12])
    r = sqrt(dx*dx + dy*dy)
    OLD_VALUES[which] = KNOWN_VALUES[which]
    KNOWN_VALUES[which] = r
    
    gx, gy = to_global(dx, dy)
    similar = False
    for old in READINGS:
        if (old.x - gx)**2 + (old.y-gy)**2 < 3**2:
            similar = True
            break
    if not similar:
        READINGS.append(Reading(gx, gy, z=1 if which != 6 else 0))

def parse_floor(string):
    global FLOOR
    value = int(string[4:8])
    FLOOR = value != 0

def parse_halt():
    global idle
    idle = True
    print("\033[31mRobot halted\033[0m")

def parse_bearing(line):
    global BEARING
    BEARING = int(line[4:8])


def parse_position(line):
    global X, Y
    X = int(line[2:7])
    Y = int(line[7:12])

def parse_safety(line):
    global SAFETY
    SAFETY = int(line[4:8]) != 0

def handle_serial(line):

    # if "RNG" not in line:
    print("\033[34mHandling: " + line + "\033[0m")
    
    if line[:4] == "HALT":
        parse_halt()
    elif line[:3] == "RNG":
        parse_range(line)
    elif line[:4] == "FLOR":
        parse_floor(line)
    elif line[:4] == "BRNG":
        parse_bearing(line)
    elif line[:2] == "PN":
        parse_position(line)
    elif line[:4] == "SAFE":
        parse_safety(line)
    else:
        return False
    return True

class Reading:
    def __init__(self, global_x, global_y, z=1):
        self.timestamp = time.monotonic()
        self.x = global_x
        self.y = global_y
        self.z = z

class Buffer:
    def __init__(self):
        self._message = ""
    
    def push(self, byte):
        try:
            self._message += bytes([byte]).decode("ascii")
        except Exception as e:
            print(f"Exception on byte {byte}, {self._message=}")
            print(e)
            self._message = ""
            # exit() 

    def message(self):
        if len(self._message) == 0:
            return
        if self._message[-1] == ";":
            msg = self._message
            self._message = ""
            return msg
        return None

CURRENT_MESSAGE = Buffer()
def read_bluetooth():
    global CURRENT_MESSAGE
    data = serialPort.read()
    if len(data) == 0:
        return False
    CURRENT_MESSAGE.push(data[0])
    message = CURRENT_MESSAGE.message()
    if message != None:
        handle_serial(message)
    return True

def serial_send(command):
    print("\033[33mSending command: ", end="")
    print(command)
    print("\033[0m")
    serialPort.write(command)


def labelled_line(start, end, label, color):
    pygame.draw.line(display, color, start, end)
    delta = (end[0] - start[0], end[1] - start[1])
    l = sqrt(delta[0]**2 + delta[1]**2)
    if l > 0:
        normal = (-delta[1] / l, delta[0] / l)
        off = 10
        midpoint = ((end[0] + start[0]) / 2 + normal[0] * off, (end[1] + start[1]) / 2 + normal[1]*off)
    else:
        midpoint = start
    text_surface = font.render(label, False, color)
    display.blit(text_surface, midpoint)


def text(str, x, y, color=WHITE):
    text_surface = font.render(str, False, color)
    display.blit(text_surface, (x, y))

pygame.init()

display = pygame.display.set_mode(WINDOW_SIZE)
pygame.display.set_caption("Robot Controller")

last_ultra_req = 0

clock = pygame.time.Clock()
pygame.font.init()
font = pygame.font.SysFont("Arial", 16)
while True:
    # Process incoming events
    for event in pygame.event.get():
        if event.type == QUIT:
            handle_quit()
        if event.type == KEYDOWN:
            if event.key == K_SPACE:
                serial_send(command_halt())
            elif event.key == K_c:
                handle_clear()
            elif event.key == K_DOWN:
                serial_send(command_drive(-ROLL_AMOUNT))
            elif event.key == K_UP:
                serial_send(command_drive(ROLL_AMOUNT))
            elif event.key == K_LEFT:
                serial_send(command_turn(ROTATION_AMOUNT))
            elif event.key == K_RIGHT:
                serial_send(command_turn(-ROTATION_AMOUNT))
            elif event.key == K_d:
                if pygame.key.get_pressed()[K_LSHIFT]:
                    ROTATION_AMOUNT += 15
                else:
                    ROTATION_AMOUNT += 5
            elif event.key == K_a:
                if pygame.key.get_pressed()[K_LSHIFT]:
                    ROTATION_AMOUNT -= 15
                else:
                    ROTATION_AMOUNT -= 5
            elif event.key == K_w:
                if pygame.key.get_pressed()[K_LSHIFT]:
                    ROLL_AMOUNT += 50
                else:
                    ROLL_AMOUNT += 10
            elif event.key == K_s:
                if pygame.key.get_pressed()[K_LSHIFT]:
                    ROLL_AMOUNT -= 50
                else:
                    ROLL_AMOUNT -= 10
            elif event.key == K_h:
                serial_send(command_set_safety(not SAFETY))
            elif event.key == K_g:
                serial_send(command_toggle_gripper())
        elif event.type == MOUSEWHEEL:
            SCALE *= (1 + 0.01 * event.y)

    # Handle all messages
    try:
        while read_bluetooth():
            pass
    except Exception as e:
        pass

    # Clear screen
    pygame.draw.rect(display, BLACK, (0, 0, *WINDOW_SIZE))

    # Draw things
    # Robot
    pygame.draw.circle(display, WHITE, ORIGIN, ROBOT_RADIUS * SCALE, width = 0 if FLOOR else 1)
    pygame.draw.circle(display, (127, 127, 127), (ORIGIN[0], ORIGIN[1] - SCALE * ROLL_AMOUNT), ROBOT_RADIUS * SCALE, width = 1)

    # Known sensor values
    for i, dist in enumerate(OLD_VALUES):
        OLD_VALUES[i] += (KNOWN_VALUES[i] - OLD_VALUES[i]) * 0.1
        angle = SENSOR_ANGLES[i]
        if dist >= 100:
            labelled_line(ORIGIN, (
                ORIGIN[0] + dist * SCALE * cos(angle),
                ORIGIN[1] - dist * SCALE * sin(angle)),
                f"{int(dist) - 100}",
                BLUE)
            


    # Compass
    # compass_end = (
    #     ORIGIN[0] + 200*cos((BEARING) / 180 * pi),
    #     ORIGIN[1] - 200*sin((BEARING) / 180 * pi),
    # )
    # labelled_line(ORIGIN, compass_end, f"{BEARING}", RED)

    # Turn indicator
    turn_end = (
        ORIGIN[0] + 50*cos((ROTATION_AMOUNT+90) / 180 * pi),
        ORIGIN[1] - 50*sin((ROTATION_AMOUNT+90) / 180 * pi),
    )
    labelled_line(ORIGIN, turn_end, f"", RED)

    turn_end = (
        ORIGIN[0] + 50*cos((-ROTATION_AMOUNT+90) / 180 * pi),
        ORIGIN[1] - 50*sin((-ROTATION_AMOUNT+90) / 180 * pi),
    )
    labelled_line(ORIGIN, turn_end, f"", RED)

    # Draw previous readings
    for reading in READINGS:
        lx, ly = to_local(reading.x, reading.y)
        x = WINDOW_SIZE[0] / 2 + lx * SCALE
        y = WINDOW_SIZE[1] / 2 - ly * SCALE
        pygame.draw.rect(display, WHITE if reading.z == 1 else BLUE, (x-2, y-2, 4, 4))

    text(f"Roll {ROLL_AMOUNT}mm [S-/W+] (Arrow keys)", 10, 10)
    text(f"Turn {ROTATION_AMOUNT}deg [A-/D+] (Arrow keys)", 10, 30)
    text(f"Safety {'ON' if SAFETY else 'OFF'} (H)", 10, 50, color=GREEN if SAFETY else RED)
    text(f"Hoist Flag (G)", 10, 70)
    text(f"Clear Data (C)", 10, 90)
    text(f"Halt (SPACE)", 10, 110)

    # Update display and wait for next frame
    pygame.display.update()
    clock.tick(60)
