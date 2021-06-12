import glfw
from OpenGL.GL import *
import numpy as np

def MyDisplay():
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0)
    glViewport(0, 0, 300, 300)
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT)
    glColor3f(0.5, 0.5, 0.5)
    glLoadIdentity();

    
    glTranslatef(0.5,-0.5,0)
    glRotatef(-45,0,0,1)

    w = np.sqrt(2*(0.5*0.5))*0.5
    glBegin(GL_POLYGON)
    glVertex3f(w,w,0)
    glVertex3f(-w,w,0)
    glVertex3f(-w,-w,0)
    glVertex3f(w,-w,0)
    glEnd()
    glFlush();

def main():
    if not glfw.init():
        return

    window = glfw.create_window(300, 300, "test",None, None)
    if not window:
        glfw.terminate()
        return

    glfw.make_context_current(window)

    while not glfw.window_should_close(window):
        glfw.poll_events()
        
        MyDisplay()
        
        glfw.swap_buffers(window)

    glfw.terminate()

if __name__ == "__main__":
    main()