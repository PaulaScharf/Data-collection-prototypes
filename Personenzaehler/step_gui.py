import tkinter as tk
from tkinter import Scale
from PIL import Image, ImageTk
import itertools
import textwrap

class WeightSelectorApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Two-View GUI")

        self.wait_frame = tk.Frame(root)
        self.top_frame = tk.Frame(root)
        self.mid_frame = tk.Frame(root)
        self.bottom_frame = tk.Frame(root)
        self.footer_frame = tk.Frame(self.root)

        self.init_wait_view()

    def init_form_view(self, root, step_plot_path='/home/paula/Documents/reedu/TinyAIoT/python/yolov7/runs/timestamps/exp/1701348558.685155/step.png', 
                 step_vid_path=['/home/paula/Documents/reedu/TinyAIoT/python/yolov7/runs/timestamps/exp/1701348547.7402017/1701348547.7402017.jpg',
                                '/home/paula/Documents/reedu/TinyAIoT/python/yolov7/runs/timestamps/exp/1701348547.7402017/1701348548.0977826.jpg',
                                '/home/paula/Documents/reedu/TinyAIoT/python/yolov7/runs/timestamps/exp/1701348547.7402017/1701348548.4456806.jpg']):
        self.wait_view = False

        self.wait_frame.destroy()
        self.footer_frame.destroy()

        self.top_frame.destroy()
        self.mid_frame.destroy()
        self.bottom_frame.destroy()
        self.top_frame = tk.Frame(root)
        self.mid_frame = tk.Frame(root)
        self.bottom_frame = tk.Frame(root)

        self.top_frame = tk.Frame(root)
        self.top_frame.pack(pady=10)
        # Title
        title_label = tk.Label(self.top_frame, text="This step was last recorded", font=('Helvetica', 16, 'bold'))
        title_label.pack(pady=10)

        self.mid_frame = tk.Frame(root)
        self.mid_frame.pack(pady=10)

        self.step_plot = Image.open(step_plot_path)
        self.step_plot = self.step_plot.resize((800, 480))
        self.step_plot = ImageTk.PhotoImage(self.step_plot)

        self.step_vids = step_vid_path
        self.step_vid_index = itertools.cycle(range(len(self.step_vids)))

        self.step_plot_label = tk.Label(self.mid_frame, image=self.step_plot)
        self.step_plot_label.pack(side=tk.LEFT, padx=10)

        self.step_vid_label = tk.Label(self.mid_frame)
        self.step_vid_label.pack(side=tk.RIGHT, padx=10)

        self.rotate_vid_image()  # Initial update

        self.bottom_frame = tk.Frame(root)
        self.bottom_frame.pack(pady=50)

        # Information
        labeltext = textwrap.fill("We would like to use this data train a neural network to detect steps through ground vibration."
                                   + " If you wouldnt mind, it would help us out greatly, if you could also provide us your approximate weight and age.", width=150)
        self.info_label = tk.Label(self.bottom_frame, text=labeltext, font=("Arial", 16))
        self.info_label.pack(pady=10)

        # Weight scale
        self.weight_label = tk.Label(self.bottom_frame, text="Select your weight:", font=("Arial", 16))
        self.weight_label.pack()

        self.weight_scale = Scale(self.bottom_frame, from_=40, to=150, orient="horizontal", length=300, resolution=5, font=("Arial", 16))
        self.weight_scale.pack(pady=10)

        # Age scale
        self.age_label = tk.Label(self.bottom_frame, text="Select your age:", font=("Arial", 16))
        self.age_label.pack()

        self.age_scale = Scale(self.bottom_frame, from_=0, to=123, orient="horizontal", length=300, font=("Arial", 16))
        self.age_scale.pack(pady=10)

        # Button to submit
        self.submit_button = tk.Button(self.bottom_frame, text="Submit", command=self.submit_data, font=("Arial", 25))
        self.submit_button.pack()

        self.step_folder = ''

        self.footer_frame = tk.Frame(self.root)
        self.footer_frame.pack(padx=10, pady=10)
        self.init_footer(self.footer_frame)

    def init_wait_view(self):
        self.wait_view = True
        # Congrats view
        self.top_frame.destroy()
        self.mid_frame.destroy()
        self.bottom_frame.destroy()
        self.footer_frame.destroy()

        self.wait_frame.destroy()
        self.wait_frame = tk.Frame(self.root)
        self.wait_frame.pack(padx=10, pady=10)

        wait_label = tk.Label(self.wait_frame, text="waiting for step.....", width=1620, height=45, font=("Arial", 16))
        wait_label.pack()
        self.init_footer(self.wait_frame)

    def init_footer(self, frame):

        contact_label = tk.Label(frame, text="If you have any questions about this, please contact: p.scharf@reedu.de (R.105)",font=("Arial", 16))
        contact_label.pack(side=tk.LEFT, padx=100)

        self.tinyaiot_logo = Image.open('logos/tinyAIoT_logo.png')
        self.tinyaiot_logo = self.tinyaiot_logo.resize((105, 36))
        self.tinyaiot_logo = ImageTk.PhotoImage(self.tinyaiot_logo)

        self.tinyaiot_logo_label = tk.Label(frame, image=self.tinyaiot_logo)
        self.tinyaiot_logo_label.pack(side=tk.RIGHT, padx=10)

        self.ifgi_logo = Image.open('logos/ifgi_logo.png')
        self.ifgi_logo = self.ifgi_logo.resize((105, 36))
        self.ifgi_logo = ImageTk.PhotoImage(self.ifgi_logo)

        self.ifgi_logo_label = tk.Label(frame, image=self.ifgi_logo)
        self.ifgi_logo_label.pack(side=tk.RIGHT, padx=10)

    def destroy_all_frames(self):
        self.root.destroy()

    def submit_data(self):
        selected_weight = self.weight_scale.get()
        selected_age = self.age_scale.get()
        print(f"Selected weight: {selected_weight} kg \nSelected age: {selected_age} years")
        if self.step_folder != '':
            file = open(self.step_folder + '/person_stats.txt', 'w')
            file.write(f'weight,age\n{selected_weight},{selected_age}')
            file.close()
            self.step_folder=''
            self.init_wait_view()
    
    def rotate_vid_image(self):
        if not self.wait_view:
            self.step_vid_path = self.step_vids[next(self.step_vid_index)]
            self.step_vid = Image.open(self.step_vid_path)
            self.step_vid = self.step_vid.resize((540, 405))
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
    
    def set_step_folder(self, path):
        self.step_folder = path