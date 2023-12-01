import matplotlib.pyplot as plt
import numpy as np
import os

# Load data from the TXT file
# file_path = 'yolov7/runs/timestamps/exp8/no_step/no_step.txt'
save_dir = 'yolov7/runs/timestamps/exp'
directories = [x[0] for x in os.walk(save_dir)]
for dir in directories:

    print(dir)
    if 'no_step' in dir:
        for i,file in enumerate(os.listdir(dir)):
            if file.endswith(".txt"):
                file_path = os.path.join(dir, file)
                try:
                    data = np.genfromtxt(file_path, delimiter=',', skip_header=1)  # Skip header

                    # Extract timestamp and acceleration data
                    timestamp = data[:, 1]
                    acc_x = data[:, 2]
                    acc_y = data[:, 3]
                    acc_z = data[:, 4]

                    # Plotting
                    plt.figure(figsize=(10, 6))

                    plt.plot(timestamp, acc_x, label='AccX',linestyle="",marker="o")
                    plt.plot(timestamp, acc_y, label='AccY',linestyle="",marker="o")
                    plt.plot(timestamp, acc_z, label='AccZ',linestyle="",marker="o")

                    plt.title('Vibration Data')
                    plt.xlabel('Timestamp')
                    plt.ylabel('Acceleration')
                    plt.legend()
                    plt.grid(False)
                    file_path_without_extension, file_extension = os.path.splitext(file_path)
                    
                    plt.savefig(file_path_without_extension + '.png')
                except:
                    print("vibrations couldnt be plotted")
    else:
        for i,file in enumerate(os.listdir(dir)):
            if 'step' in file and file.endswith(".txt"):
                file_path = os.path.join(dir, file)
                try:
                    data = np.genfromtxt(file_path, delimiter=',', skip_header=1)  # Skip header

                    # Extract timestamp and acceleration data
                    timestamp = data[:, 1]
                    acc_x = data[:, 2]
                    acc_y = data[:, 3]
                    acc_z = data[:, 4]

                    # Plotting
                    plt.figure(figsize=(10, 6))

                    plt.plot(timestamp, acc_x, label='AccX',linestyle="",marker="o")
                    plt.plot(timestamp, acc_y, label='AccY',linestyle="",marker="o")
                    plt.plot(timestamp, acc_z, label='AccZ',linestyle="",marker="o")

                    plt.title('Vibration Data')
                    plt.xlabel('Timestamp')
                    plt.ylabel('Acceleration')
                    plt.legend()
                    plt.grid(False)
                    file_path_without_extension, file_extension = os.path.splitext(file_path)
                    
                    plt.savefig(file_path_without_extension + '.png')
                except:
                    print(file_path)
                    