import matplotlib.pyplot as plt
import numpy as np
import os

# Load data from the TXT file
# file_path = 'yolov7/runs/timestamps/exp8/no_step/no_step.txt'
save_dir = 'yolov7/runs/timestamps/exp13'
directories = [x[0] for x in os.walk(save_dir)]
for dir in directories:

    print(dir)
    if 'no_step' in dir:
        for i,file in enumerate(os.listdir(dir)):
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
                print("vibrations couldnt be plotted")
    else:
        for i,file in enumerate(os.listdir(dir)):
            print(file)
            if 'step' in file:
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
                    