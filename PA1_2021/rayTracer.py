#!/usr/bin/env python3
# -*- coding: utf-8 -*
# sample_python aims to allow seamless integration with lua.
# see examples below

import os
import sys
import pdb  # use pdb.set_trace() for debugging
import code # or use code.interact(local=dict(globals(), **locals()))  for debugging.
import xml.etree.ElementTree as ET
import numpy as np
from PIL import Image 

FAR_AWAY = sys.maxsize

class Color:
    def __init__(self, R, G, B):
        self.color=np.array([R,G,B]).astype(np.float)

    # Gamma corrects this color.
    # @param gamma the gamma value to use (2.2 is generally used).
    def gammaCorrect(self, gamma):
        inverseGamma = 1.0 / gamma;
        self.color=np.power(self.color, inverseGamma)

    def toUINT8(self):
        return (np.clip(self.color, 0,1)*255).astype(np.uint8)

class Sphere:
    def __init__(self, shader, center, radius):
        self.shader = shader
        self.center = center
        self.radius = radius

class Box:
    def __init__(self, shader, minPt, maxPt, normals):
        self.shader = shader
        self.minPt = minPt
        self.maxPt = maxPt
        self.normals = normals

class Shader:
    def __init__(self, type):
        self.type = type

class ShaderLambertian(Shader):
    def __init__(self, diffuse):
        self.diffuse = diffuse

class ShaderPhong(Shader):
    def __init__(self, diffuse, specular, exponent):
        self.diffuse = diffuse
        self.specular = specular
        self.exponent = exponent

class View:
    def __init__(self, viewPoint, viewDir, viewUp, viewProjNormal, viewWidth, viewHeight, projDistance, intensity):
        self.viewPoint = viewPoint
        self.viewDir = viewDir
        self.viewUp = viewUp
        self.viewProjNormal = viewProjNormal
        self.viewWidth = viewWidth
        self.viewHeight = viewHeight
        self.projDistance = projDistance
        self.intensity = intensity

class Light:
    def __init__(self, position, intensity):
        self.position = position
        self.intensity = intensity

def raytrace(list, ray, viewPoint):
    global FAR_AWAY
    m = FAR_AWAY

    idx = -1
    cnt = 0

    for i in list:
        if i.__class__.__name__ == 'Sphere':

            a = np.sum(ray * ray)
            b = np.sum((viewPoint - i.center) * ray)
            c = np.sum((viewPoint - i.center) ** 2) - i.radius ** 2

            if b **2 - a * c >= 0:
                if -b + np.sqrt(b ** 2 - a * c) >= 0:
                    if m >= (-b + np.sqrt(b ** 2 - a * c)) / a:
                        m = (-b + np.sqrt(b ** 2 - a * c)) / a
                        idx = cnt

                if -b - np.sqrt(b ** 2 - a * c) >= 0:
                    if m >= (-b - np.sqrt(b ** 2 - a * c)) / a:
                        m = (-b - np.sqrt(b ** 2 - a * c)) / a
                        idx = cnt

        elif i.__class__.__name__ == 'Box':
            flag = 1

            txmin = (i.minPt[0] - viewPoint[0]) / ray[0]
            txmax = (i.maxPt[0] - viewPoint[0]) / ray[0]

            if txmin > txmax:
                txmin, txmax = txmax, txmin
            
            tymin = (i.minPt[1] - viewPoint[1]) / ray[1]
            tymax = (i.maxPt[1] - viewPoint[1]) / ray[1]

            if tymin > tymax:
                tymin, tymax = tymax, tymin
            
            tzmin = (i.minPt[2] - viewPoint[2]) / ray[2]
            tzmax = (i.maxPt[2] - viewPoint[2]) / ray[2]

            if tzmin > tzmax:
                tzmin, tzmax = tzmax, tzmin
            

            if txmin > tymax or tymin > txmax:
                flag = 0

            if txmin < tymin:
                txmin = tymin

            if txmax > tymax:
                txmax = tymax

            if txmin > tzmax or tzmin > txmax:
                flag = 0

            if tzmin >= txmin:
                txmin = tzmin
            
            if tzmax < txmax:
                txmax = tzmax

            if flag == 1:
                if m >= txmin:
                    m = txmin
                    idx = cnt

        cnt = cnt + 1

    return [m, idx]

def shade(m, ray, view, list, idx, light):
    if idx == -1:
        return np.array([0,0,0])
    else:
        x = 0
        y = 0
        z = 0
        n = np.array([0,0,0])
        v = - m * ray

        if list[idx].__class__.__name__ == 'Sphere':
            n = view.viewPoint + m * ray - list[idx].center

            if (abs(np.sqrt(np.sum(n * n)) - list[idx].radius) > 0.000001):
                print('check', abs(np.sqrt(np.sum(n * n)) - list[idx].radius))

            n = n / np.sqrt(np.sum(n * n))

        elif list[idx].__class__.__name__ == "Box":
            point = view.viewPoint + m * ray
            diff = FAR_AWAY

            i = -1
            cnt = 0

            for normal in list[idx].normals:
                if abs(np.sum(normal[0:3] * point) - normal[3]) < diff:
                    diff = abs(np.sum(normal[0:3] * point) - normal[3])
                    i = cnt
                cnt = cnt + 1

            n = list[idx].normals[i][0:3]
            n = n / np.sqrt(np.sum(n * n))

        for i in light:
            light_i = v + i.position - view.viewPoint
            light_i = light_i / np.sqrt(np.sum(light_i * light_i))
            check = raytrace(list, -light_i, i.position)

            if check[1] == idx:
                if list[idx].shader.__class__.__name__ == 'ShaderPhong':
                    unit_v = v / np.sqrt(np.sum(v**2))
                    h = unit_v + light_i
                    h = h / np.sqrt(np.sum(h*h))
                    x = x + list[idx].shader.diffuse[0]*max(0,np.dot(n,light_i))*i.intensity[0] + list[idx].shader.specular[0] * i.intensity[0] * pow(max(0, np.dot(n, h)),list[idx].shader.exponent[0])
                    y = y + list[idx].shader.diffuse[1]*max(0,np.dot(n,light_i))*i.intensity[1] + list[idx].shader.specular[1] * i.intensity[1] * pow(max(0, np.dot(n, h)),list[idx].shader.exponent[0])
                    z = z + list[idx].shader.diffuse[2]*max(0,np.dot(n,light_i))*i.intensity[2] + list[idx].shader.specular[2] * i.intensity[2] * pow(max(0, np.dot(n, h)),list[idx].shader.exponent[0])

                elif list[idx].shader.__class__.__name__ == "ShaderLambertian":
                    x = x + list[idx].shader.diffuse[0] * i.intensity[0] * max(0, np.dot(light_i, n))
                    y = y + list[idx].shader.diffuse[1] * i.intensity[1] * max(0, np.dot(light_i, n))
                    z = z + list[idx].shader.diffuse[2] * i.intensity[2] * max(0, np.dot(light_i, n))
        
        res = Color(x, y, z)
        res.gammaCorrect(2.2)
        return res.toUINT8()

def getNormal(x, y, z):
    direction = np.cross((y - x),(z - x))
    d = np.sum(direction * z)
    return np.array([direction[0],direction[1],direction[2], d])

def getBoxNormals(minPt, maxPt):
    points = []
    normals = []

    points.append(np.array([minPt[0], minPt[1], maxPt[2]]))
    points.append(np.array([minPt[0], maxPt[1], minPt[2]]))
    points.append(np.array([maxPt[0], minPt[1], minPt[2]]))
    points.append(np.array([minPt[0], maxPt[1], maxPt[2]]))
    points.append(np.array([maxPt[0], minPt[1], maxPt[2]]))
    points.append(np.array([maxPt[0], maxPt[1], minPt[2]]))

    normals.append(getNormal(points[0], points[2], points[4]))
    normals.append(getNormal(points[1], points[2], points[5]))
    normals.append(getNormal(points[0], points[1], points[3]))
    normals.append(getNormal(points[0], points[4], points[3]))
    normals.append(getNormal(points[4], points[2], points[5]))
    normals.append(getNormal(points[3], points[5], points[1]))

    return normals

def main():


    tree = ET.parse(sys.argv[1])
    root = tree.getroot()

    list = []
    light = []

    # set default values
    viewPoint = np.array([0,0,0]).astype(np.float)
    viewDir=np.array([0,0,-1]).astype(np.float)
    viewUp=np.array([0,1,0]).astype(np.float)
    viewProjNormal=-1*viewDir  # you can safely assume this. (no examples will use shifted perspective camera)
    viewWidth=1.0
    viewHeight=1.0
    projDistance=1.0
    intensity=np.array([1,1,1]).astype(np.float)  # how bright the light is.
    # print(np.cross(viewDir, viewUp))

    imgSize=np.array(root.findtext('image').split()).astype(np.int)

    for c in root.findall('camera'):
        viewPoint = np.array(c.findtext('viewPoint').split()).astype(np.float)
        viewDir = np.array(c.findtext('viewDir').split()).astype(np.float)
        if (c.findtext('projNormal')):
            viewProjNormal = np.array(c.findtext('projNormal').split()).astype(np.float)
        if (c.findtext('projDistance')):
            projDistance = np.array(c.findtext('projDistance').split()).astype(np.float)
        viewUp = np.array(c.findtext('viewUp').split()).astype(np.float)
        viewWidth = np.array(c.findtext('viewWidth').split()).astype(np.float)
        viewHeight = np.array(c.findtext('viewHeight').split()).astype(np.float)
        # print('viewpoint', viewPoint)

    view = View(viewPoint, viewDir, viewUp, viewProjNormal, viewWidth, viewHeight, projDistance, intensity)
    for c in root.findall('shader'):
        diffuseColor_c=np.array(c.findtext('diffuseColor').split()).astype(np.float)
        print('name', c.get('name'))
        print('diffuseColor', diffuseColor_c)
    #code.interact(local=dict(globals(), **locals()))  

    for c in root.findall('surface'):
        type_c = c.get('type')
        if type_c == 'Sphere':
            center_c = np.array(c.findtext('center').split()).astype(np.float)
            radius_c = np.array(c.findtext('radius')).astype(np.float)
            ref = ''
            for child in c:
                if child.tag == 'shader':
                    ref = child.get('ref')
            for d in root.findall('shader'):
                if d.get('name') == ref:
                    diffuse_d = np.array(d.findtext('diffuseColor').split()).astype(np.float)
                    type_d = d.get('type')
                    if type_d == 'Lambertian':
                        shader = ShaderLambertian(diffuse_d)
                        list.append(Sphere(shader, center_c, radius_c))
                    elif type_d == 'Phong':
                        exponent_d = np.array(d.findtext('exponent').split()).astype(np.float)
                        specular_d = np.array(d.findtext('specularColor').split()).astype(np.float)
                        shader = ShaderPhong(diffuse_d, specular_d, exponent_d)
                        list.append(Sphere(shader, center_c, radius_c))
        
        elif type_c == 'Box':
            minPt_c = np.array(c.findtext("minPt").split()).astype(np.float)
            maxPt_c = np.array(c.findtext("maxPt").split()).astype(np.float)

            normals_c = getBoxNormals(minPt_c, maxPt_c)
            ref = ''
            for child in c:
                if child.tag == 'shader':
                    ref = child.get('ref')
            for d in root.findall('shader'):
                if d.get('name') == ref:
                    diffuse_d = np.array(d.findtext('diffuseColor').split()).astype(np.float)
                    type_d = d.get('type')
                    if type_d == 'Lambertian':
                        shader = ShaderLambertian(diffuse_d)
                        list.append(Box(shader, minPt_c, maxPt_c, normals_c))
                    elif type_d == 'Phong':
                        exponent_d = np.array(d.findtext('exponent').split()).astype(np.float)
                        specular_d = np.array(d.findtext('specularColor').split()).astype(np.float)
                        shader = ShaderPhong(diffuse_d, specular_d, exponent_d)
                        list.append(Box(shader, minPt_c, maxPt_c, normals_c))

    for c in root.findall('light'):
        position_c = np.array(c.findtext('position').split()).astype(np.float)
        intensity_c = np.array(c.findtext('intensity').split()).astype(np.float)
        light.append(Light(position_c, intensity_c))

    # Create an empty image
    channels=3
    img = np.zeros((imgSize[1], imgSize[0], channels), dtype=np.uint8)
    # pdb.set_trace() # python debug
    img[:,:]=0
    
    # replace the code block below!
    for i in np.arange(imgSize[1]): 
        white=Color(1,1,1)
        red=Color(1,0,0)
        blue=Color(0,0,1)
        img[10][i]=white.toUINT8()
        img[i][i]=red.toUINT8()
        img[i][0]=blue.toUINT8()

    for x in np.arange(imgSize[0]): 
        img[5][x]=[255,255,255]

    w = view.viewDir
    u = np.cross(w, view.viewUp)
    v = np.cross(w, u)

    unit_w = w / np.sqrt(np.sum(w * w))
    unit_u = u / np.sqrt(np.sum(u * u))
    unit_v = v / np.sqrt(np.sum(v * v))

    pixel_x = view.viewWidth / imgSize[0]
    pixel_y = view.viewHeight / imgSize[1]

    start = unit_w * view.projDistance - unit_u * pixel_x * ((imgSize[0]/2) + 1/2) - unit_v * pixel_y * ((imgSize[1]/2) + 1/2)

    for x in np.arange(imgSize[0]):
        for y in np.arange(imgSize[1]):
            ray = start + unit_u * x * pixel_x + pixel_y * y * unit_v
            tmp = raytrace(list, ray, view.viewPoint)
            img[y][x] = shade(tmp[0], ray, view, list, tmp[1], light)

    rawimg = Image.fromarray(img, 'RGB')
    #rawimg.save('out.png')
    rawimg.save(sys.argv[1]+'.png')
    
if __name__=="__main__":
    main()
