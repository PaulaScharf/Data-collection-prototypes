import tkinter as tk
from tkinter import Scale
from PIL import Image, ImageTk
import itertools

class WeightSelectorApp:
    def __init__(self, root, step_plot_path='/home/paula/Documents/reedu/TinyAIoT/python/yolov7/runs/timestamps/exp/1701348558.685155/step.png', 
                 step_vid_path=['/home/paula/Documents/reedu/TinyAIoT/python/yolov7/runs/timestamps/exp/1701348547.7402017/1701348547.7402017.jpg',
                                '/home/paula/Documents/reedu/TinyAIoT/python/yolov7/runs/timestamps/exp/1701348547.7402017/1701348548.0977826.jpg',
                                '/home/paula/Documents/reedu/TinyAIoT/python/yolov7/runs/timestamps/exp/1701348547.7402017/1701348548.4456806.jpg']):
        self.root = root
        self.root.title("Step Submitter")

        # Title
        title_label = tk.Label(root, text="This step was last recorded", font=('Helvetica', 16, 'bold'))
        title_label.pack(pady=10)

        mid_frame = tk.Frame(root)
        mid_frame.pack(pady=10)

        self.step_plot = Image.open(step_plot_path)
        self.step_plot = self.step_plot.resize((1000, 600))
        self.step_plot = ImageTk.PhotoImage(self.step_plot)

        self.step_vids = step_vid_path
        self.step_vid_index = itertools.cycle(range(len(self.step_vids)))

        self.step_plot_label = tk.Label(mid_frame, image=self.step_plot)
        self.step_plot_label.pack(side=tk.LEFT, padx=10)

        self.step_vid_label = tk.Label(mid_frame)
        self.step_vid_label.pack(side=tk.RIGHT, padx=10)

        self.rotate_vid_image()  # Initial update

        bottom_frame = tk.Frame(root)
        bottom_frame.pack()

        # Information
        self.info_label = tk.Label(bottom_frame, text="We would like to use this data train a neural network to detect steps through ground vibration."
                                   + " If you wouldnt mind, it would help us out greatly, if you could also provide us your approximate weight and age.", font=("Arial", 16))
        self.info_label.pack()

        # Weight scale
        self.weight_label = tk.Label(bottom_frame, text="Select your weight:", font=("Arial", 16))
        self.weight_label.pack()

        self.weight_scale = Scale(bottom_frame, from_=40, to=150, orient="horizontal", length=300, resolution=5, font=("Arial", 16))
        self.weight_scale.pack(pady=10)

        # Age scale
        self.weight_label = tk.Label(bottom_frame, text="Select your age:", font=("Arial", 16))
        self.weight_label.pack()

        self.weight_scale = Scale(bottom_frame, from_=0, to=123, orient="horizontal", length=300, font=("Arial", 16))
        self.weight_scale.pack(pady=10)

        # Button to submit
        self.submit_button = tk.Button(bottom_frame, text="Submit", command=self.submit_weight, font=("Arial", 25))
        self.submit_button.pack()

    def submit_weight(self):
        selected_weight = self.weight_scale.get()
        print(f"Selected weight: {selected_weight} kg")
    
    def rotate_vid_image(self):
        self.step_vid_path = self.step_vids[next(self.step_vid_index)]
        self.step_vid = Image.open(self.step_vid_path)
        self.step_vid = self.step_vid.resize((600, 450))
        self.step_vid = ImageTk.PhotoImage(self.step_vid)
        self.step_vid_label.config(image=self.step_vid)
        self.step_vid_label.image = self.step_vid  # Keep a reference to avoid garbage collection
        self.root.after(750, self.rotate_vid_image)  # Rotate every 3000 milliseconds (3 seconds)
    
    def update_step_vid(self, step_vid_path):
        self.step_vids = step_vid_path
        self.step_vid_index = itertools.cycle(range(len(self.step_vids)))

    def set_step_plot(self,image_path):
        self.step_plot = Image.open(image_path)
        self.step_plot = self.step_plot.resize((1000, 600))
        self.tk_image = ImageTk.PhotoImage(self.step_plot)
        if hasattr(self, 'step_plot_label'):
            self.step_plot_label.config(image=self.tk_image)
        else:
            self.step_plot_label = tk.Label(self.root, image=self.tk_image)
            self.step_plot_label.pack()