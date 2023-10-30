import time
import pygame
from pygame.locals import *
from math import pi, sin, cos, sqrt, atan2
from random import randint


WINDOW_SIZE = (800, 800) # px
SCALE = 0.5 # px / mm

BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
BLUE = (0, 255, 255)
RED = (255, 127, 127)

ROBOT_RADIUS = 120 # mm
SENSOR_ANGLES = [4*pi / 4, 3*pi/4, 2*pi/4, 1*pi/4, 0*pi/4]
KNOWN_VALUES = [0] * 5

ORIGIN = (WINDOW_SIZE[0]//2, WINDOW_SIZE[1]//2)

command_queue = []
idle = True

def r_theta(x, y):
    dx, dy = x - WINDOW_SIZE[0]/2, y - WINDOW_SIZE[1]/2
    r = sqrt(dx*dx + dy*dy)
    theta = atan2(dy, dx)
    return r, theta

def command_turn(radians):
    comm = b"TURN"
    degrees = int(radians * 180 / pi)
    LSB = degrees & 0xff
    MSB = (degrees & 0xff00) >> 8
    comm = comm + bytes([MSB])
    comm = comm + bytes([LSB])
    comm = comm + b";"
    return comm

def command_drive(mm):
    comm = b"DRIV"
    LSB = int(mm) & 0xff
    MSB = (int(mm) & 0xff00) >> 8
    comm = comm + bytes([MSB])
    comm = comm + bytes([LSB])
    comm = comm + b";"
    return comm

def command_ultra(which):
    return b"ULTR" + bytes([which]) + b";"

def command_halt():
    return b"HALT;"


def handle_quit():
    pygame.quit()
    exit()

def handle_click(x, y):
    r, theta = r_theta(x, y)
    command_queue.clear()
    command_queue.append(command_turn(r))
    command_queue.append(command_drive(theta))

def handle_halt():
    global idle
    command_queue.clear()
    command_queue.append(command_halt())
    idle = True # Trigger send

def handle_ultra(which):
    handle_serial.ultra_request = which
    command_queue.append(command_ultra(which))

def parse_ultra(string):
    which = string[4]
    MSB = string[5]
    LSB = string[6]
    dist = MSB << 8 | LSB
    KNOWN_VALUES[which] = dist
    print(f"Got back ultra {which} = {dist}")

def parse_halt():
    global idle
    idle = True
    print("Robot halted")

def handle_serial():

    if time.monotonic() - handle_serial.last_time > 2:
        if handle_serial.ultra_request == None:
            handle_serial.last_time = time.monotonic()
            line = b"HALT;"
        else:
            dist = randint(ROBOT_RADIUS + 10, ROBOT_RADIUS + 300);
            line = b"ULTR"
            line = line + bytes([int(handle_serial.ultra_request), (dist & 0xff00) >> 8, dist & 0xff])
            line = line + b";"
            handle_serial.ultra_request = None
        print("Got serial: ", line)
    else:
        line = b""
    
    if line[:4] == b"HALT":
        if not idle:
            parse_halt()
    elif line[:4] == b"ULTR":
        parse_ultra(line)
    else:
        return False
    return True

handle_serial.last_time = time.monotonic()
handle_serial.ultra_request = None

def serial_send(command):
    # TODO
    print("Sending command: ", end="")
    print(command)

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
        off = 20
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
font = pygame.font.SysFont("Arial", 10)
while True:
    # Process incoming events
    for event in pygame.event.get():
        if event.type == QUIT:
            handle_quit()
        if event.type == MOUSEBUTTONDOWN:
            handle_click(*event.pos)
        if event.type == KEYDOWN:
            if event.key == K_x:
                handle_halt()
            elif event.key == K_u:
                handle_ultra(last_ultra_req)
                last_ultra_req = (last_ultra_req + 1) % 5

    # Handle all messages
    while handle_serial():
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
        labelled_line(ORIGIN, (
            ORIGIN[0] + dist * SCALE * cos(angle),
            ORIGIN[1] + dist * SCALE * sin(angle)),
            f"{KNOWN_VALUES[i]}",
            BLUE)
        
    # To cursor
    cursor = pygame.mouse.get_pos()
    r, theta = r_theta(*cursor)
    labelled_line(ORIGIN, pygame.mouse.get_pos(), f"{int(r)}<{int(theta*180/pi)}", RED)
        


    # Update display and wait for next frame
    pygame.display.update()
    clock.tick(60)