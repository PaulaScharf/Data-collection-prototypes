import sys
import pygame
from pygame.locals import *
from random import randint
import threading
import os
import numpy as np
from sklearn import metrics
from sklearn.cluster import DBSCAN
no_of_clusters = 0

def cluster_lidar(file):
    lidar_lines = file.readlines()
    to_be_clustered = lidar_lines
    to_be_clustered.pop(0) #(pygame.math.Vector2(min(float(line.split(', ')[2])/10,250.0), 0).rotate((float(line.split(', ')[1])) % 360))
    to_be_clustered = [[float(line.split(', ')[0]), (pygame.math.Vector2(min(float(line.split(', ')[2])/10,250.0), 0).rotate((float(line.split(', ')[1])) % 360))[0], (pygame.math.Vector2(min(float(line.split(', ')[2])/10,250.0), 0).rotate((float(line.split(', ')[1])) % 360))[1]] for line in to_be_clustered]
    # print(lidar_mid_lines)

    db = DBSCAN(eps=25, min_samples=10).fit(to_be_clustered)
    labels = db.labels_

    # Number of clusters in labels, ignoring noise if present.
    n_clusters_ = len(set(labels)) - (1 if -1 in labels else 0)
    n_noise_ = list(labels).count(-1)

    print("Estimated number of clusters: %d" % n_clusters_)
    print("Estimated number of noise points: %d" % n_noise_)

    # lidar_mid_lines = lidar_mid_file.readlines()
    for i, line in enumerate(lidar_lines):
        lidar_lines[i] = lidar_lines[i].replace("\n","") +str(labels[i])
    # no_of_clusters += n_clusters_
    return lidar_lines

def get_time(line):
    return int(line.split(', ')[0].split('.')[0])

lidar_top_file = open('data/1/lidar_top.txt', 'r')
lidar_top_lines = cluster_lidar(lidar_top_file)
lidar_top_lines = [s + ', 0' for s in lidar_top_lines]
lidar_mid_file = open('data/1/lidar_mid.txt', 'r')
lidar_mid_lines = cluster_lidar(lidar_mid_file)
lidar_mid_lines = [s + ', 1' for s in lidar_mid_lines]
lidar_bot_file = open('data/1/lidar_bottom.txt', 'r')
lidar_bot_lines = cluster_lidar(lidar_bot_file)
lidar_bot_lines = [s + ', 2' for s in lidar_bot_lines]

img_files = os.listdir('data/1/yolo-result')
img_files = [s + ', , , , 3' for s in img_files]

lines = lidar_top_lines + lidar_mid_lines + lidar_bot_lines + img_files
lines.sort(key=get_time)
# print(lines)

pygame.init()

sysfont = pygame.font.get_default_font()
font = pygame.font.SysFont(None, 16)

screen = pygame.display.set_mode((1900, 900))
FPSCLOCK = pygame.time.Clock()
RED = pygame.Color("red")
YELLOW = pygame.Color("yellow")
GREEN = pygame.Color("green")
startpoint_0 = pygame.math.Vector2(0, 150)
startpoint_1 = pygame.math.Vector2(0, 450)
startpoint_2 = pygame.math.Vector2(0, 750)
endpoint = pygame.math.Vector2(200, 0)
angle = 0
done = False
prev_timestamp = 0
prev_angle_0=0
prev_angle_1=0
prev_angle_2=0

for line in lines:
    line_split = line.split(', ')
    type = line_split[4]
    current_timestamp = get_time(line)
    if int(type) == 3:
        imp = pygame.image.load('data/1/yolo-result/' + line_split[0]).convert()
        screen.blit(imp, (400, 0))

        pygame.draw.rect(screen,(0,0,0),(5,5,40,10))
        img = font.render(str(current_timestamp), True, GREEN)
        screen.blit(img, (5, 5))

        pygame.display.flip()
        if(prev_timestamp!=0):
            time_diff = current_timestamp - prev_timestamp + 1
            FPSCLOCK.tick(1000/time_diff)
            prev_timestamp = current_timestamp
        else:
            prev_timestamp = current_timestamp
            FPSCLOCK.tick(30)
    elif line_split[0] != 'Time' and ((float(line_split[1]) > 310 and float(line_split[1]) <360) or float(line_split[1]) < 70):
        current_timestamp = int(line_split[0])
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                done = True

        if int(type) == 0:
            current_startpoint = startpoint_0
            current_prev_angle = prev_angle_0
            rect_top = 0
        elif int(type) == 1:  
            current_startpoint = startpoint_1
            current_prev_angle = prev_angle_1
            rect_top = 300
        else:
            current_startpoint = startpoint_2
            current_prev_angle = prev_angle_2
            rect_top = 600
        # % 360 to keep the angle between 0 and 360.
        angle = (float(line_split[1])-10) % 360 
        # The current endpoint is the startpoint vector + the
        # rotated original endpoint vector.
        endpoint = pygame.math.Vector2(min(float(line_split[2])/10,250.0), 0)
        current_endpoint = current_startpoint + endpoint.rotate(angle)

        if(angle-current_prev_angle>10):
            pygame.draw.rect(screen,(0,0,0),(0,rect_top,300,300))
        if int(type) == 0:
            prev_angle_0 = angle
        elif int(type) == 1:  
            prev_angle_1 = angle
        else:
            prev_angle_2 = angle
        pygame.draw.line(screen, (0,0,0), current_startpoint, current_endpoint, 2)
        if int(line_split[3]) > -1 and (angle<300 or angle>340):
            pygame.draw.line(screen, YELLOW, current_startpoint, current_endpoint, 2)
        else:
            pygame.draw.line(screen, RED, current_startpoint, current_endpoint, 2)

        pygame.draw.rect(screen,(0,0,0),(5,5,40,10))
        img = font.render(str(current_timestamp), True, GREEN)
        screen.blit(img, (5, 5))

        pygame.display.flip()
        if(prev_timestamp!=0):
            time_diff = current_timestamp - prev_timestamp + 1
            FPSCLOCK.tick(1000/time_diff)
            prev_timestamp = current_timestamp
        else:
            prev_timestamp = current_timestamp
            FPSCLOCK.tick(30)

pygame.quit()
sys.exit()