from pynput import keyboard

# program to capture single image from webcam in python
  
# importing OpenCV library
import cv2

import datetime
import time
import os
import sys
import threading
import copy

import matplotlib.pyplot as plt
import numpy as np

import tkinter as tk
from tkinter import Scale
from PIL import Image, ImageTk
from step_gui import WeightSelectorApp
  
num_picts = 50000
interval = 0.1

start_step = 0
last_recording_of_step = 0
currently_stepping = False
stop_recording = False
stopped_recording = threading.Event()
save_dir = ''
port = '/dev/ttyACM0'
init_form_view = False
gui_file_path = ''
gui_file_dir = ''

import serial

# try:
#     arduino = serial.Serial(port=port,   baudrate=115200, timeout=.1)
# except:
#     print('arduino on port ACM1')
#     port = '/dev/ttyACM1'
#     arduino = serial.Serial(port=port,   baudrate=115200, timeout=.1)

# Yolo
import argparse
import time
from pathlib import Path

import cv2
import torch
import torch.backends.cudnn as cudnn
from numpy import random

from models.experimental import attempt_load
from utils.datasets import LoadStreams, LoadImages
from utils.general import check_img_size, check_requirements, check_imshow, non_max_suppression, apply_classifier, \
    scale_coords, xyxy2xywh, strip_optimizer, set_logging, increment_path
from utils.plots import plot_one_box
from utils.torch_utils import select_device, load_classifier, time_synchronized, TracedModel

# options
weights='yolov7.pt' #model.pt path(s)
source='2' #
img_size=640 #inference size (pixels)
conf_thres=0.25 #object confidence threshold
iou_thres=0.45 #IOU threshold for NMS
device_opt='' #cuda device, i.e. 0 or 0,1,2,3 or cpu
view_img=False #display results
save_txt=False #save results to *.txt
save_conf=False #save confidences in --save-txt labels
nosave=False #do not save images/videos
classes=[0] #filter by class: --class 0, or --class 0 2 3
agnostic_nms=False #class-agnostic NMS
augment=False #augmented inference
update=False #update all models
project='runs/timestamps' #save results to project/name
name='exp' #save results to project/name
exist_ok=False #existing project/name ok, do not increment
trace=True #don`t trace model
    
def list_ports():
    """
    Test the ports and returns a tuple with the available ports and the ones that are working.
    """
    non_working_ports = []
    dev_port = 0
    working_ports = []
    available_ports = []
    while len(non_working_ports) < 6: # if there are more than 5 non working ports stop the testing. 
        camera = cv2.VideoCapture(dev_port)
        if not camera.isOpened():
            non_working_ports.append(dev_port)
            # print("Port %s is not working." %dev_port)
        else:
            is_reading, img = camera.read()
            w = camera.get(3)
            h = camera.get(4)
            if is_reading:
                # print("Port %s is working and reads images (%s x %s)" %(dev_port,h,w))
                working_ports.append(dev_port)
            else:
                # print("Port %s for camera ( %s x %s) is present but does not reads." %(dev_port,h,w))
                available_ports.append(dev_port)
        dev_port +=1
    return available_ports,working_ports,non_working_ports

def run_step_gui():
    global init_form_view
    root = tk.Tk()
    gui_step = WeightSelectorApp(root)
    while not stop_recording:
        root.update()
        if init_form_view:
            gui_step.init_form_view(root,gui_file_path,webcam_in_folder(gui_file_dir))
            gui_step.set_step_folder(gui_file_dir)
            init_form_view = False
    gui_step.destroy_all_frames()   

def record_vibrations_txt(save_dir):
    global stopped_recording, stop_recording, init_form_view, gui_file_path, gui_file_dir
    file = open('testi.txt', "r")
    # root = tk.Tk()
    # gui_step = WeightSelectorApp(root)
    # root.update()
    backlog = []
    (save_dir / "no_step").mkdir(parents=True, exist_ok=True)
    (save_dir / "no_step" / "imgs").mkdir(parents=True, exist_ok=True)
    while not stop_recording:
        # root.update()
        no_step_path = Path(increment_path((save_dir / "no_step" / "no_step"), exist_ok=False, ending='.txt'))  # increment run
        file = open(no_step_path, "w")
        file.write("timestamp,millis,AccX.AccY,AccZ,GyroX,GyroY,GyroZ\n")
        while not currently_stepping:
            time.sleep(0.001)
        final = int(last_recording_of_step)
        # with open('testi.txt') as myFile:
        #     for num, line in enumerate(myFile, 1):
        #         if lookup in line:
        #             print 'found at line:', num

        file.close()
        # if not serial_response:
        #     terminate_script('manual')

        if not stop_recording:
            serial_response = False
            file_dir = str(save_dir / str(start_step))
            (save_dir / str(start_step)).mkdir(parents=True, exist_ok=True)
            file_path = str(save_dir / str(start_step) / "step.txt")
            first = int(start_step)
            file = open(file_path, "w")
            file.write("timestamp,millis,AccX.AccY,AccZ,GyroX,GyroY,GyroZ\n")
            
            while currently_stepping:
                final = int(last_recording_of_step)
                # root.update()

            write = False
            with open('testi.txt') as myFile:
                if final != 0:
                    for _, line in enumerate(myFile, 1):
                        if not write and (str(first-1) in line or str(first) in line):
                            write = True
                        if write and str(final+1) in line: 
                            write = False
                        if write:
                            serial_response = True
                            file.write(line)
            file.close()
            print(str(first-1))
            print(str(final+1))
            if not serial_response:
                print('no data')
            else:   
                print('step!')
                try:
                    data = np.genfromtxt(file_path, delimiter=',', skip_header=1)  # Skip header

                    # Extract timestamp and acceleration data
                    start_time = data[0, 1]
                    timestamp = [dat-start_time for dat in data[:, 1]]
                    acc_x = data[:, 2]
                    acc_y = data[:, 3]
                    acc_z = data[:, 4]

                    # Plotting
                    plt.figure(figsize=(10, 6))

                    plt.plot(timestamp, acc_x, label='AccX')
                    plt.plot(timestamp, acc_y, label='AccY')
                    plt.plot(timestamp, acc_z, label='AccZ')

                    plt.title('Vibration Data')
                    plt.xlabel('milliseconds')
                    plt.ylabel('Acceleration')
                    plt.legend()
                    plt.grid(True)
                    file_path_without_extension, file_extension = os.path.splitext(file_path)
                    
                    plt.savefig(file_path_without_extension + '.png')

                    gui_file_path = file_path_without_extension + '.png'
                    gui_file_dir = file_dir
                    init_form_view = True
                    # gui_step.init_form_view(root,file_path_without_extension + '.png',webcam_in_folder(file_dir))
                    # gui_step.set_step_folder(file_dir)
                except:
                    print(file_path)
    # gui_step.destroy_all_frames()    
    terminate_script('stopped')
    stopped_recording.set()

def record_vibrations(save_dir):
    with serial.Serial(port=port, baudrate=115200) as ser:
        global stopped_recording, stop_recording
        root = tk.Tk()
        gui_step = WeightSelectorApp(root)
        root.update()
        backlog = []
        (save_dir / "no_step").mkdir(parents=True, exist_ok=True)
        (save_dir / "no_step" / "imgs").mkdir(parents=True, exist_ok=True)
        while not stop_recording:
            root.update()
            no_step_path = Path(increment_path((save_dir / "no_step" / "no_step"), exist_ok=False, ending='.txt'))  # increment run
            file = open(no_step_path, "w")
            file.write("timestamp,millis,AccX.AccY,AccZ,GyroX,GyroY,GyroZ\n")
            serial_response=False
            for line in ser:
                root.update()
                line = line.decode('utf-8')
                backlog.append(line)
                if not serial_response and line and line!='':
                    serial_response = True
                if len(backlog) > 300:
                    file.write(backlog[0])
                    backlog.pop(0)
                if last_recording_of_step > 0 or stop_recording:
                    stop_recording = True
                    break
            if not serial_response:
                terminate_script('manual')
            file.close()

            if not stop_recording:
                file_dir = str(save_dir / str(start_step))
                file_path = str(save_dir / str(start_step) / "step.txt")
                file = open(file_path, "w")
                file.write("timestamp,millis,AccX.AccY,AccZ,GyroX,GyroY,GyroZ\n")
                serial_response=False
                i=0
                for line in ser:
                    root.update()
                    line = line.decode('utf-8')
                    backlog.append(line)
                    if not serial_response and line and line!='':
                        serial_response = True
                    if len(backlog) > 300:
                        file.write(backlog[0])
                        backlog.pop(0)
                    if last_recording_of_step <= 0:
                        i=i+1
                    if (last_recording_of_step <= 0 and i>100) or stop_recording:
                        stop_recording = True
                        break
                for element in backlog:
                    file.write(element)
                backlog = []
                file.close()
                if not serial_response:
                    terminate_script('manual')
                else:   
                    print('step!')
                    try:
                        data = np.genfromtxt(file_path, delimiter=',', skip_header=1)  # Skip header

                        # Extract timestamp and acceleration data
                        start_time = data[0, 1]
                        timestamp = [dat-start_time for dat in data[:, 1]]
                        acc_x = data[:, 2]
                        acc_y = data[:, 3]
                        acc_z = data[:, 4]

                        # Plotting
                        plt.figure(figsize=(10, 6))

                        plt.plot(timestamp, acc_x, label='AccX')
                        plt.plot(timestamp, acc_y, label='AccY')
                        plt.plot(timestamp, acc_z, label='AccZ')

                        plt.title('Vibration Data')
                        plt.xlabel('milliseconds')
                        plt.ylabel('Acceleration')
                        plt.legend()
                        plt.grid(True)
                        file_path_without_extension, file_extension = os.path.splitext(file_path)
                        
                        plt.savefig(file_path_without_extension + '.png')

                        gui_step.init_form_view(root,file_path_without_extension + '.png',webcam_in_folder(file_dir))
                        gui_step.set_step_folder(file_dir)
                    except:
                        print(file_path)
    gui_step.destroy_all_frames()    
    terminate_script('stopped')
    stopped_recording.set()

def webcam_in_folder(dir):
    paths = []
    for i,file in enumerate(os.listdir(dir)):
        if file.endswith(".jpg") and not 'step' in file:
            file_path = os.path.join(dir, file)
            paths.append(file_path)
    return paths

def check_if_overstepped_center(xyxy, img):
    center = img.shape[1]/2
    return xyxy[0] < center and xyxy[2] > center

def detect(save_img=False):
    global last_recording_of_step,start_step,save_dir,currently_stepping
    save_img = not nosave and not source.endswith('.txt')  # save inference images
    webcam = source.isnumeric() or source.endswith('.txt') or source.lower().startswith(
        ('rtsp://', 'rtmp://', 'http://', 'https://'))

    # Directories
    save_dir = Path(increment_path(Path(project) / name, exist_ok=exist_ok))  # increment run
    (save_dir / 'labels' if save_txt else save_dir).mkdir(parents=True, exist_ok=True)  # make dir

    # Initialize
    set_logging()
    device = select_device(device_opt)
    half = device.type != 'cpu'  # half precision only supported on CUDA

    # Load model
    model = attempt_load(weights, map_location=device)  # load FP32 model
    stride = int(model.stride.max())  # model stride
    imgsz = check_img_size(img_size, s=stride)  # check img_size

    if trace:
        model = TracedModel(model, device, img_size)

    if half:
        model.half()  # to FP16

    # Second-stage classifier
    classify = False
    if classify:
        modelc = load_classifier(name='resnet101', n=2)  # initialize
        modelc.load_state_dict(torch.load('weights/resnet101.pt', map_location=device)['model']).to(device).eval()

    # Set Dataloader
    vid_path, vid_writer = None, None
    if webcam:
        view_img = check_imshow()
        cudnn.benchmark = True  # set True to speed up constant image size inference
        dataset = LoadStreams(source, img_size=imgsz, stride=stride)
    else:
        dataset = LoadImages(source, img_size=imgsz, stride=stride)

    # Get names and colors
    names = model.module.names if hasattr(model, 'module') else model.names
    colors = [[150,150,255]]

    # Run inference
    if device.type != 'cpu':
        model(torch.zeros(1, 3, imgsz, imgsz).to(device).type_as(next(model.parameters())))  # run once
    old_img_w = old_img_h = imgsz
    old_img_b = 1

    t0 = time.time()

    gui_thread = threading.Thread(target=run_step_gui, args=())
    gui_thread.start()
    record_thread = threading.Thread(target=record_vibrations_txt, args=(save_dir,))
    record_thread.start()
    for path, img, im0s, vid_cap in dataset:
        if stop_recording:
            break
        img = torch.from_numpy(img).to(device)
        img = img.half() if half else img.float()  # uint8 to fp16/32
        img /= 255.0  # 0 - 255 to 0.0 - 1.0
        if img.ndimension() == 3:
            img = img.unsqueeze(0)

        # Warmup
        if device.type != 'cpu' and (old_img_b != img.shape[0] or old_img_h != img.shape[2] or old_img_w != img.shape[3]):
            old_img_b = img.shape[0]
            old_img_h = img.shape[2]
            old_img_w = img.shape[3]
            for i in range(3):
                model(img, augment=augment)[0]

        # Inference
        t1 = time_synchronized()
        with torch.no_grad():   # Calculating gradients would cause a GPU memory leak
            pred = model(img, augment=augment)[0]
        t2 = time_synchronized()

        # Apply NMS
        pred = non_max_suppression(pred, conf_thres, iou_thres, classes=classes, agnostic=agnostic_nms)
        t3 = time_synchronized()

        # Apply Classifier
        if classify:
            pred = apply_classifier(pred, modelc, img, im0s)

        # Process detections
        for i, det in enumerate(pred):  # detections per image
            if webcam:  # batch_size >= 1
                p, s, im0, frame = path[i], '%g: ' % i, im0s[i].copy(), dataset.count
            else:
                p, s, im0, frame = path, '', im0s, getattr(dataset, 'frame', 0)

            p = Path(p)  # to Path
            if len(det):
                has_overstepped_center = False
                # Rescale boxes from img_size to im0 size
                det[:, :4] = scale_coords(img.shape[2:], det[:, :4], im0.shape).round()

                # Print results
                for c in det[:, -1].unique():
                    n = (det[:, -1] == c).sum()  # detections per class
                    s += f"{n} {names[int(c)]}{'s' * (n > 1)}, "  # add to string

                # Write results
                for *xyxy, conf, cls in reversed(det):
                    if save_img or view_img:  # Add bbox to image
                        label = f'{names[int(cls)]} {conf:.2f}'
                        plot_one_box(xyxy, im0, label=label, color=colors[int(cls)], line_thickness=1)
                        if not has_overstepped_center:
                            has_overstepped_center = check_if_overstepped_center(xyxy,im0)

                if has_overstepped_center:
                    cv2.putText(im0, 'Step!', (int(im0.shape[1]/2-10),int(im0.shape[0]-20)), cv2.FONT_HERSHEY_SIMPLEX, 1, colors[0], 2, cv2.LINE_AA) 
                    if start_step <= 0:
                        start_step = t1
                        (save_dir / str(start_step)).mkdir(parents=True, exist_ok=True)
                    last_recording_of_step = t1
                    currently_stepping = True
                    # Print time (inference + NMS)
                    # print(f'{s}Done. ({(1E3 * (t2 - t1)):.1f}ms) Inference, ({(1E3 * (t3 - t2)):.1f}ms) NMS')

                    cv2.imwrite(str(save_dir / str(start_step) / (str(t1)+".jpg")), im0)
                else:
                    if last_recording_of_step>0 and (t1-last_recording_of_step)>0.6:
                        # os.rename(str(save_dir / str(start_step)), str(save_dir / (str(start_step)+"-"+str(last_recording_of_step))))
                        # last_recording_of_step = 0
                        currently_stepping = False
                        start_step = 0
                        cv2.imwrite(str(save_dir / "no_step" / "imgs" / (str(t1)+".jpg")), im0)
                    else:
                        cv2.imwrite(str(save_dir / str(start_step) / (str(t1)+".jpg")), im0)
            else:
                if last_recording_of_step>0 and (t1-last_recording_of_step)>0.6:
                    # os.rename(str(save_dir / str(start_step)), str(save_dir / (str(start_step)+"-"+str(last_recording_of_step))))
                    # last_recording_of_step = 0
                    currently_stepping = False
                    start_step = 0
                    cv2.imwrite(str(save_dir / "no_step" / "imgs" / (str(t1)+".jpg")), im0)
                else:
                    cv2.imwrite(str(save_dir / str(start_step) / (str(t1)+".jpg")), im0)
            cv2.imshow("Webcam Stream", im0)

    if save_txt or save_img:
        s = f"\n{len(list(save_dir.glob('labels/*.txt')))} labels saved to {save_dir / 'labels'}" if save_txt else ''
        #print(f"Results saved to {save_dir}{s}")

    print(f'Done. ({time.time() - t0:.3f}s)')

def terminate_script(key):
    print('terminating...')
    print(key)
    global stop_recording 
    cv2.destroyAllWindows()
    if key == 'manual' or key=='c' or key==keyboard.Key.ctrl or (key.char and key.char=='c'):
        stop_recording = True
        stopped_recording.wait()

        directories = [x[0] for x in os.walk(save_dir)]
        for dir in directories:
            if 'no_step' in dir:
                for i,file in enumerate(os.listdir(dir)):
                    if file.endswith(".txt"):
                        file_path = os.path.join(dir, file)
                        try:
                            data = np.genfromtxt(file_path, delimiter=',', skip_header=1)  # Skip header

                            # Extract timestamp and acceleration data
                            timestamp = data[:, 0]
                            acc_x = data[:, 1]
                            acc_y = data[:, 2]
                            acc_z = data[:, 3]

                            # Plotting
                            plt.figure(figsize=(10, 6))

                            plt.plot(timestamp, acc_x, label='AccX')
                            plt.plot(timestamp, acc_y, label='AccY')
                            plt.plot(timestamp, acc_z, label='AccZ')

                            plt.title('Vibration Data')
                            plt.xlabel('Timestamp')
                            plt.ylabel('Acceleration')
                            plt.legend()
                            plt.grid(True)
                            file_path_without_extension, file_extension = os.path.splitext(file_path)
                            
                            plt.savefig(file_path_without_extension + '.png')
                        except:
                            print(file_path)
            else:
                for i,file in enumerate(os.listdir(dir)):
                    if 'step' in file and file.endswith(".txt"):
                        file_path = os.path.join(dir, file)
                        try:
                            data = np.genfromtxt(file_path, delimiter=',', skip_header=1)  # Skip header

                            # Extract timestamp and acceleration data
                            timestamp = data[:, 0]
                            acc_x = data[:, 1]
                            acc_y = data[:, 2]
                            acc_z = data[:, 3]

                            # Plotting
                            plt.figure(figsize=(10, 6))

                            plt.plot(timestamp, acc_x, label='AccX')
                            plt.plot(timestamp, acc_y, label='AccY')
                            plt.plot(timestamp, acc_z, label='AccZ')

                            plt.title('Vibration Data')
                            plt.xlabel('Timestamp')
                            plt.ylabel('Acceleration')
                            plt.legend()
                            plt.grid(True)
                            file_path_without_extension, file_extension = os.path.splitext(file_path)
                            
                            plt.savefig(file_path_without_extension + '.png')
                        except:
                            print(file_path)
        sys.exit()

# listener = keyboard.Listener(
#     on_press=terminate_script)
# listener.start()

detect()