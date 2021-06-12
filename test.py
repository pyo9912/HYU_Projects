import glfw
from OpenGL.GL import *
import numpy as np

def drawBox(height):
    glBegin(GL_POLYGON)
    glColor3f(0.5, 0.5, 0.5);
    glVertex3fv(np.array([0.,height,0]))
    glVertex3fv(np.array([0.,0,0]))
    glVertex3fv(np.array([height,0,0]))
    glVertex3fv(np.array([height,height,0]))
    glEnd()

def render():
    glClear(GL_COLOR_BUFFER_BIT)
    glLoadIdentity()
    width = 0.3
    glRotatef(-45,0,0,1)
    drawBox(width)

    glTranslatef(2*width,0,0)
    drawBox(width)
    glFlush

def main():
    if not glfw.init():
        return

    window = glfw.create_window(480, 480, "test",None, None)
    if not window:
        glfw.terminate()
        return

    glfw.make_context_current(window)

    while not glfw.window_should_close(window):
        glfw.poll_events()
        
        render()
        
        glfw.swap_buffers(window)

    glfw.terminate()

if __name__ == "__main__":
    main()