import argparse
from pathlib import Path
import matplotlib.pyplot as plt
import numpy as np
import os

def write_files(folder_path):
    print("writing label files...")
    with open((folder_path + '/labels.txt')) as labels_file:
        labels_iter = iter(labels_file)
        current_label = next(labels_iter)
        current_label = next(labels_iter)
        current_label_split = current_label.split(',')
        current_label_time = current_label_split[1].split('.')[0]
        current_status = current_label_split[0]
        Path(folder_path + '/labels').mkdir(parents=True, exist_ok=True)
        file = open((folder_path + '/labels/no_step-' + str(current_label_time)+'.txt'), "w")
        file.write("timestamp,millis,AccX.AccY,AccZ,GyroX,GyroY,GyroZ\n")
        with open((folder_path + '/mpu6050.txt')) as sensor_file:
            for index, line in enumerate(sensor_file, 1):
                if(index>1):
                    if not current_label_time in line:
                        file.write(line)    
                    else:
                        file.close()
                        try:
                            current_label = next(labels_iter)
                            current_label_split = current_label.split(',')
                            current_label_time = current_label_split[1].split('.')[0]
                            current_status = current_label_split[0]
                            if current_status == 'start_step':
                                current_label_time = str(int(current_label_time)-1)
                                file = open((folder_path + '/labels/no_step-' + str(current_label_time)+'.txt'), "w")
                            else:
                                file = open((folder_path + '/labels/step-' + str(current_label_time)+'.txt'), "w")
                                current_label_time = str(int(current_label_time)+1)
                            file.write("timestamp,millis,AccX.AccY,AccZ,GyroX,GyroY,GyroZ\n")
                        except:
                            break
                    
def plot_labels(folder_path):
    print('plotting labels...')
    labels_path = folder_path + '/labels'
    for i,file in enumerate(os.listdir(labels_path)):
        if file.endswith(".txt"):
            file_path = os.path.join(labels_path, file)
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

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--folder', type=str, default='/home/paula/Documents/reedu/TinyAIoT/python/yolov7/runs/timestamps/exp92', help='sensor path')
    opt = parser.parse_args()

    write_files(opt.folder)
    plot_labels(opt.folder)
