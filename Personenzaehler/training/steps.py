import pandas
import pathlib
import tensorflow.keras
from tqdm import tqdm
import numpy as np
import math

from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import LSTM, Dense, Reshape, Conv1D, MaxPooling2D, GlobalAveragePooling1D, Flatten, MaxPooling1D, Dropout, Input


from sklearn.model_selection import GridSearchCV


def load_dataset(directory_path):
	# create temporary row of zeros
	column_names = ['time', 'x', 'y', 'z', 'step']
	dataset = pandas.DataFrame(
		data = [len(column_names) * [0.0]],
		columns = column_names,
		dtype = 'Float32'
	)
	all_datasets = []
	# find dataset directory
	directory_path = pathlib.Path(directory_path)
	if not directory_path.is_dir():
		raise RuntimeError(f'the dataset\'s directory does not exist')
	# find dataset index
	index_path = directory_path.joinpath('index.csv')
	if not index_path.is_file():
		raise RuntimeError(f'the dataset\'s index is missing')
	# read dataset index
	data_types = {
		'path': 'string',
		'prefix': 'string',
		'time_index': 'UInt16',
		'time_factor': 'Float32',
		'x_index': 'UInt16',
		'x_factor': 'Float32',
		'y_index': 'UInt16',
		'y_factor': 'Float32',
		'z_index': 'UInt16',
		'z_factor': 'Float32'
	}
	index = pandas.read_csv(
		index_path,
		header = 0,
		names = data_types.keys(),
		dtype = data_types
	)
	# iterate over all formats
	for format_info in index.itertuples(index=False):
		# find format directory
		format_path = directory_path.joinpath(format_info.path)
		if not format_path.is_dir():
			raise RuntimeError(f'the directory "{format_path}" does not exist')
		# iterate over all recordings
		for recording_path in tqdm(format_path.iterdir(), desc=str(format_path)):
			# read recording
			column_indices = {
				'time': format_info.time_index,
				'x': format_info.x_index,
				'y': format_info.y_index,
				'z': format_info.z_index
			}
			recording = pandas.read_csv(
				recording_path,
				header = 0,
				names = column_indices.keys(),
				usecols = column_indices.values(),
				dtype = 'Float32'
			)
			# apply unit conversion factors
			recording['time'] *= format_info.time_factor
			recording['x'] *= format_info.x_factor
			recording['y'] *= format_info.y_factor
			recording['z'] *= format_info.z_factor
			# verify that time is strictly increasing
			time_delta = recording['time'].diff().min()
			if time_delta <= 0.0:
				raise RuntimeException(f'time is not strictly increasing for "{recording_path}"')
			# set recording time to start after dataset time
			# recording['time'] += dataset['time'].iat[-1] + time_delta - recording['time'].iat[0]
			# attach labels based on file name
			recording['step'] = 0.0 if recording_path.stem.startswith(format_info.prefix) else 1.0
			recording['step'] = recording['step'].astype('Float32', copy=False)
			# append recording to dataset
			dataset = pandas.concat([recording], ignore_index=True)
			column_means = dataset.mean()
			chunk_size = 400
			if len(dataset) > chunk_size:
				if dataset['step'][0] == 0.0:
					datasets = []
					num_rows = dataset.shape[0]
					num_chunks = num_rows // chunk_size
					for i in range(num_chunks + 1):
						start_idx = i * chunk_size
						end_idx = start_idx + chunk_size
						
						# If we're at the last chunk and there aren't enough rows left,
						# we'll wrap around to the beginning of the DataFrame
						if end_idx > num_rows:
							remaining_rows = chunk_size - (num_rows - start_idx)
							chunk = pandas.concat([dataset.iloc[start_idx:], dataset.iloc[:remaining_rows]])
						else:
							chunk = dataset.iloc[start_idx:end_idx]
							datasets.append(chunk)
						
						if len(dataset)<400:
							print("len(all_datasets)")
							print(len(all_datasets))
						
						# datasets.append(chunk)
				else:
					# compute the vibrations' amplitude
					amplitude = (dataset - column_means).abs().max(axis=1)
					# find the first index whose amplitude is above one standard deviation
					offset = amplitude.loc[amplitude >= amplitude.std()].index[0]
					# start slightly before that point in time
					offset -= math.ceil(chunk_size / 10)
					# fit the time window inside the recording
					offset = max(0, min(offset, len(dataset) - 400))
					dataset = dataset.shift(-offset).reindex(range(400))

					if len(dataset)<400:
						print("len(all_datasets)")
						print(len(all_datasets))
					datasets = [dataset]
			else:
				dataset = dataset.reindex(range(400))
				if len(dataset)<400:
					print("len(all_datasets)")
					print(len(all_datasets))
				datasets = [dataset]
			for set in datasets:
				# make sure the time window has the correct size
				# set = set.reindex(range(400))
				# perform mean imputation for missing values
				set = set.fillna(column_means)
				# remove initial row of zeros
				set = set.iloc[1:]
				# set start time to zero
				set = set.assign(time=set['time'].iat[0])
				# use time as index
				set = set.assign(time=pandas.to_timedelta(set['time'], 's'))
				set.set_index('time', inplace=True)
				all_datasets.append(set)
	
	return all_datasets

def resample_time(dataset, resolution='5ms'):
	return dataset.resample(resolution).nearest()

def create_1d_model(window_size=399):
	def add_block(model, feature_count, kernel_size, pool_size):
		model.add(tensorflow.keras.layers.Convolution2D(
			filters = feature_count,
			kernel_size = kernel_size,
			padding = 'same',
			activation = 'relu'
		))
		model.add(tensorflow.keras.layers.AveragePooling2D(
			pool_size = pool_size
		))
		model.add(tensorflow.keras.layers.BatchNormalization())

	model = Sequential()
	# model.add(Reshape((int(window_size**0.5), int(window_size**0.5),3), input_shape=(window_size,3)))
	model.add(Input((window_size*3,)))
	model.add(Reshape((window_size,3)))
	model.add(Conv1D(6,(3), activation='relu'))
	# model.add(GlobalAveragePooling1D())
	# model.add(MaxPooling1D(2))
	# model.add(Conv1D(12,(3), activation='relu'))
	# model.add(Flatten())
	# model.add(Dense(16, activation='relu'))
	# model.add(Dropout(0.2))
	model.add(Flatten())
	# model.add(tensorflow.keras.layers.BatchNormalization(
	# 	input_shape = (window_size, 3)
	# ))
	# add_block(model, 4, 7, 1)
	# add_block(model, 2, 5, 1)
	# add_block(model, 1, 3, 1)
	model.add(Dense(
		units = 1,
		activation = 'sigmoid'
	))

	# tensorflow.keras.utils.plot_model(
	# 	model,
	# 	to_file='/home/paula/Documents/reedu/TinyAIoT/step_detection/model.png',
	# 	show_shapes=True,
	# )
	model.compile(
		optimizer = tensorflow.keras.optimizers.Adam(),
		loss = tensorflow.keras.losses.BinaryCrossentropy(),
		metrics=['accuracy', 
			tensorflow.keras.metrics.Precision(name='precision'),
			tensorflow.keras.metrics.Recall(name='recall'),
			tensorflow.keras.metrics.AUC(name='auc')]
	)
	return model

def prepare_data_for_tensorflow(df_array):
    # Prepare features
    X = np.array([df[['x', 'y', 'z']].values.flatten() for df in df_array])
    
    # Prepare targets (assuming 'step' is the same for all rows in a DataFrame)
    y = np.array([df['step'].iloc[0] for df in df_array])
    
    # Normalize features (using a more robust method)
    X = X.astype(np.float32)  # Ensure float type
    # mean = np.mean(X.reshape(-1, X.shape[-1]), axis=0)
    # std = np.std(X.reshape(-1, X.shape[-1]), axis=0)
    # X = (X - mean) / (std + 1e-8)  # Add small epsilon to avoid division by zero
    
    return X, y

def test_tflite(tflite_model, X_test, y_test):
	interpreter = tensorflow.lite.Interpreter(model_content=tflite_model)
	interpreter.allocate_tensors()
	# Get input and output tensors
	input_tensor_index = interpreter.get_input_details()[0]['index']
	output_tensor_index = interpreter.get_output_details()[0]['index']
	# Run inference on test data
	true_pos = 0
	true_neg = 0
	false_pos = 0
	false_neg = 0
	total_predictions = len(X_test)

	for i in range(total_predictions):
		input_data = np.expand_dims(X_test[i], axis=0).astype(np.float32)
		interpreter.set_tensor(input_tensor_index, input_data)
		interpreter.invoke()
		lite_predictions = interpreter.get_tensor(output_tensor_index)[0]
		
		# Compare predicted label with true label
		predicted_label = round(lite_predictions[0],0)
		true_label = y_test[i]
		
		if predicted_label == true_label:
			if predicted_label == 1:
				true_pos += 1
			else:
				true_neg += 1
		else:
			if predicted_label == 1:
				false_pos += 1
			else:
				false_neg += 1
		# else:
		#      print("lite model predicted: ", lite_predictions[0])
		#      print("org model predicted:  ", model.predict(np.array( [X_test[i],]),verbose = 0)[0][0])
	# Compute accuracy
	print("TensorFlow Lite model:")
	accuracy = (true_pos+true_neg) / total_predictions
	if (true_pos + false_pos) > 0:
		precision = (true_pos) / (true_pos + false_pos)
	else:
		precision = 0.0
	if (true_pos + false_neg) > 0:
		recall = (true_pos) / (true_pos + false_neg)
	else:
		recall = 0.0
	return accuracy,precision,recall

# data = load_dataset('data')
# X,y = prepare_data_for_tensorflow(data)
# data = resample_classes_amplitude(data)
# create_1d_model()