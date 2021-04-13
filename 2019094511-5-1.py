import glfw
from OpenGL.GL import *
from OpenGL.GLU import *
import numpy as np

def render():
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    glEnable(GL_DEPTH_TEST)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)
    glLoadIdentity()

    myOrtho(-5,5, -5,5, -8,8)
    myLookAt(np.array([5,3,5]), np.array([1,1,-1]), np.array([0,1,0]))

    # glOrtho(-5,5, -5,5, -8,8)
    # gluLookAt(5,3,5, 1,1,-1, 0,1,0)

    drawFrame()

    glColor3ub(255, 255, 255)
    drawCubeArray()

def myLookAt(eye, at, up):
    a = eye - at
    w = a / np.sqrt(np.dot(a,a))
    b = np.cross(up, w)
    u = b / np.sqrt(np.dot(b,b))
    v = np.cross(w, u)

    M = np.identity(4)
    M[:3, 0] = u
    M[:3, 1] = v
    M[:3, 2] = w
    M[3, :3] = [np.dot(-u,eye), -np.dot(v,eye), -np.dot(w,eye)]

    glMultMatrixf(M)

def myOrtho(left, right, bottom, top, zNear, zFar):
    M = np.identity(4)
    M[0,0] = 2 / (right - left)
    M[1,1] = 2 / (top - bottom)
    M[2,2] = -2 / (zFar - zNear)
    M[:3,3] = [-(right + left) / (right - left), -(top + bottom) / (top - bottom), -(zFar + zNear) / (zFar - zNear)]

    glMultMatrixf(M)

def drawUnitCube():
    glBegin(GL_QUADS)
    glVertex3f( 0.5, 0.5,-0.5)
    glVertex3f(-0.5, 0.5,-0.5)
    glVertex3f(-0.5, 0.5, 0.5)
    glVertex3f( 0.5, 0.5, 0.5)

    glVertex3f( 0.5,-0.5, 0.5)
    glVertex3f(-0.5,-0.5, 0.5)
    glVertex3f(-0.5,-0.5,-0.5)
    glVertex3f( 0.5,-0.5,-0.5)

    glVertex3f( 0.5, 0.5, 0.5)
    glVertex3f(-0.5, 0.5, 0.5)
    glVertex3f(-0.5,-0.5, 0.5)
    glVertex3f( 0.5,-0.5, 0.5)

    glVertex3f( 0.5,-0.5,-0.5)
    glVertex3f(-0.5,-0.5,-0.5)
    glVertex3f(-0.5, 0.5,-0.5)
    glVertex3f( 0.5, 0.5,-0.5)

    glVertex3f(-0.5, 0.5, 0.5)
    glVertex3f(-0.5, 0.5,-0.5)
    glVertex3f(-0.5,-0.5,-0.5)
    glVertex3f(-0.5,-0.5, 0.5)

    glVertex3f( 0.5, 0.5,-0.5)
    glVertex3f( 0.5, 0.5, 0.5)
    glVertex3f( 0.5,-0.5, 0.5)
    glVertex3f( 0.5,-0.5,-0.5)

    glEnd()

def drawCubeArray():
    for i in range(5):
        for j in range(5):
            for k in range(5):
                glPushMatrix()
                glTranslatef(i,j,-k-1)
                glScalef(.5,.5,.5)
                drawUnitCube()
                glPopMatrix()

def drawFrame():
    glBegin(GL_LINES)
    glColor3ub(255, 0, 0)
    glVertex3fv(np.array([0.,0.,0.]))
    glVertex3fv(np.array([1.,0.,0.]))
    glColor3ub(0, 255, 0)
    glVertex3fv(np.array([0.,0.,0.]))
    glVertex3fv(np.array([0.,1.,0.]))
    glColor3ub(0, 0, 255)
    glVertex3fv(np.array([0.,0.,0]))
    glVertex3fv(np.array([0.,0.,1.]))
    glEnd()

def main():
    if not glfw.init():
        return
    
    window = glfw.create_window(480, 480, '2019094511-5-1', None, None)
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