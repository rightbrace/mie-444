import time
import pygame
from pygame.locals import *
from math import pi, sin, cos, sqrt, atan2
from random import randint
import serial
import serial.tools.list_ports


WINDOW_SIZE = (800, 800) # px
SCALE = 0.5 # px / mm

BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
BLUE = (0, 255, 255)
RED = (255, 127, 127)

ROBOT_RADIUS = 100 # mm
SENSOR_ANGLES = [-pi / 2, -pi/4, 0, pi/4, pi/2]
KNOWN_VALUES = [0] * 5
BEARING = 0

ORIGIN = (WINDOW_SIZE[0]//2, WINDOW_SIZE[1]//2)



ports = serial.tools.list_ports.comports()
port = ""
match len(ports):
    case 0:
        print("No comport found!")
        exit()
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
serialPort = serial.Serial(port=port, timeout=0, baudrate=38400)

command_queue = []
idle = True

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

def command_turn(radians):
    degrees = int(radians * 180 / pi)
    return format_command("TURN", degrees)

def command_drive(mm):
    return format_command("DRIV", mm)

def command_ultra(which):
    return format_command("ULTR", which)

def command_halt():
    return format_command("HALT")

def handle_quit():
    pygame.quit()
    exit()

def handle_click(x, y):
    r, theta = r_theta(x, y)
    command_queue.clear()
    command_queue.append(command_turn(theta))
    command_queue.append(command_drive(r))

def handle_halt():
    global idle
    command_queue.clear()
    command_queue.append(command_halt())
    idle = True # Trigger send

def handle_ultra(which):
    handle_serial.ultra_request = which
    command_queue.append(command_ultra(which))

def handle_clear():
    global KNOWN_VALUES
    KNOWN_VALUES = [0]*5

def parse_range(string):
    which = int(ord(string[3]) - ord('0'))
    dx = int(string[4:8])
    dy = int(string[8:12])
    r = sqrt(dx*dx + dy*dy)
    KNOWN_VALUES[which] = r

def parse_comp(string):
    global BEARING
    BEARING = int(string[4:8])

def parse_halt():
    global idle
    idle = True
    print("Robot halted")

def handle_serial():

    line = current_msg
    print("Handling: " + line)
    
    if line[:4] == "HALT":
        if not idle:
            parse_halt()
    elif line[:3] == "RNG":
        parse_range(line)
    elif line[:4] == "COMP":
        parse_comp(line)
    else:
        return False
    return True

current_msg = ""
def read_bluetooth():
    global current_msg

    try:
        data = serialPort.read().decode("ascii")
        if len(data) == 0:
            return False
        current_msg += data
        # print(current_msg)
        if data == ";":
            handle_serial()
            current_msg = ""

        return True
    except Exception as e:
        print(e)

def serial_send(command):
    print("Sending command: ", end="")
    print(command)
    serialPort.write(command)

def next_command():
    global idle
    if len(command_queue) == 0:
        return False
    command = command_queue.pop(0)
    serial_send(command)
    idle = False
    return True


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
        if event.type == MOUSEBUTTONDOWN:
            handle_click(*event.pos)
        if event.type == KEYDOWN:
            if event.key == K_SPACE:
                handle_halt()
            elif event.key == K_u:
                handle_ultra(last_ultra_req)
                last_ultra_req = (last_ultra_req + 1) % 5
            elif event.key == K_c:
                handle_clear()
            elif event.key == K_RIGHTBRACKET:
                command_queue.append(command_turn(-30*pi/180))
            elif event.key == K_LEFTBRACKET:
                command_queue.append(command_turn(+30*pi/180))
            elif event.key == K_DOWN:
                command_queue.append(command_drive(-40))
            elif event.key == K_UP:
                command_queue.append(command_drive(40))
            
            elif event.key == K_LEFT:
                command_queue.append(command_turn(30*pi/180))
            elif event.key == K_RIGHT:
                command_queue.append(command_turn(-30*pi/180))

    # Handle all messages
    while read_bluetooth():
        pass

    # Send queued commands when appropriate
    if idle:
        next_command()
    
    # Clear screen
    pygame.draw.rect(display, BLACK, (0, 0, *WINDOW_SIZE))

    # Draw things
    # Robot
    pygame.draw.circle(display, WHITE, ORIGIN, ROBOT_RADIUS * SCALE, width=1)

    # Known sensor values
    for i, dist in enumerate(KNOWN_VALUES):
        angle = SENSOR_ANGLES[i]
        if dist >= 100:
            labelled_line(ORIGIN, (
                ORIGIN[0] + dist * SCALE * cos(angle),
                ORIGIN[1] + dist * SCALE * sin(angle)),
                f"{int(dist) - 100}",
                BLUE)
        
    # To cursor
    cursor = pygame.mouse.get_pos()
    r, theta = r_theta(*cursor)
    labelled_line(ORIGIN, pygame.mouse.get_pos(), f"{int(r)}<{int(theta*180/pi)}", (0, 255, 0))
        
    # # Compass
    # compass_end = (
    #     ORIGIN[0] + 200*cos((BEARING + 90) / 180 * pi),
    #     ORIGIN[1] - 200*sin((BEARING + 90) / 180 * pi),
    # )
    # labelled_line(ORIGIN, compass_end, f"{BEARING + 90}", RED)


    # Update display and wait for next frame
    pygame.display.update()
    clock.tick(60)
