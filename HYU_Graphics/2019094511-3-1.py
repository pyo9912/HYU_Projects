import glfw
from OpenGL.GL import *
import numpy as np

def render(flag):
    glClear(GL_COLOR_BUFFER_BIT)
    glLoadIdentity()
    glBegin(flag)

    for i in range(0,12):
        glVertex2f(np.cos(30*i*np.pi/180), np.sin(30*i*np.pi/180))

    glEnd()


def key_callback(window, key, scancode, action, mods):
    if key==glfw.KEY_1:
        if action==glfw.PRESS:
            render(GL_POINTS)

    elif key==glfw.KEY_2:
        if action==glfw.PRESS:
            render(GL_LINES)

    elif key==glfw.KEY_3:
        if action==glfw.PRESS:
            render(GL_LINE_STRIP)

    elif key==glfw.KEY_4:
        if action==glfw.PRESS:
            render(GL_LINE_LOOP)
            
    elif key==glfw.KEY_5:
        if action==glfw.PRESS:
            render(GL_TRIANGLES)

    elif key==glfw.KEY_6:
        if action==glfw.PRESS:
            render(GL_TRIANGLE_STRIP)

    elif key==glfw.KEY_7:
        if action==glfw.PRESS:
            render(GL_TRIANGLE_FAN)

    elif key==glfw.KEY_8:
        if action==glfw.PRESS:
            render(GL_QUADS)

    elif key==glfw.KEY_9:
        if action==glfw.PRESS:
            render(GL_QUAD_STRIP)

    elif key==glfw.KEY_0:
        if action==glfw.PRESS:
            render(GL_POLYGON)


def main():
    if not glfw.init():
        return
    
    window = glfw.create_window(480, 480, "2019094511-3-1", None, None)
    if not window:
        glfw.terminate()
        return

    
    glfw.make_context_current(window)
    render(GL_LINE_LOOP)
    while not glfw.window_should_close(window):
        glfw.poll_events()
        
        glfw.set_key_callback(window, key_callback)
        glfw.swap_buffers(window)

    glfw.terminate()

if __name__ == "__main__":
    main()