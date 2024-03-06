import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import classification_report
import numpy as np
from datetime import datetime
from joblib import dump
import time
import pickle
from scipy.stats import pearsonr
from sklearn.preprocessing import StandardScaler, MinMaxScaler
import tensorflow as tf
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import LSTM, Dense
import os
from sklearn.utils import resample
from datetime import datetime
from joblib import load
from sklearn.exceptions import DataConversionWarning
from tqdm import tqdm


# Dieses Skript ist für das Trainieren des Modells
# WICHTIG: Die CSV-Dateinen, die in dieser Studie genutzt wurden, sind für Github zu groß
# Als Platzhalter sind hier zwei Dateien, die Fahrräder und Autos umfassen, diese prepräsentieren 
# aber nur einen kleinen Teil der Gesamtdaten



# Prüfen, ob die Daten schonmal vorverarbeitet wurden
# Falls nein, springt das Skript direkt zum Training
data_path = "./trainingsdaten/fahrrader_and_fahrraeder_reversed/"
train_test_val_path = data_path + "train_data.pkl"
if not os.path.exists(train_test_val_path):
    print("Erzeugen der Datensplits...")


    # Einladen der Daten
    csv = pd.read_csv('./trainingsdaten/org/Fahrraeder.csv')
    csv2 = pd.read_csv('./trainingsdaten/org/Autos.csv')
    data = pd.read_csv('./trainingsdaten/org/Fahrraeder.csv')
    
    data['Timestamp'] = pd.to_datetime(data['Timestamp'], format='%H:%M:%S.%f')
    data['Timestamp'] = data['Timestamp'].dt.time

    # WICHTIG: Hier ist ein Beispiel für das Umkehren von Daten
    # Die Daten dafür sind jedoch für Github zu groß
    data_reverse = pd.read_csv('./trainingsdaten/org/Fahrraeder.csv')
    data_reverse['Timestamp'] = pd.to_datetime(data_reverse['Timestamp'], format='%H:%M:%S.%f')
    data_reverse['Timestamp'] = data_reverse['Timestamp'].dt.time    
    
    data_balancedT = data#pd.concat([data, data_reverse])

    data_Car = pd.read_csv('./trainingsdaten/Autos.csv')
    data_Car['Timestamp'] = pd.to_datetime(data_Car['Timestamp'], format='%H:%M:%S.%f')
    data_Car['Timestamp'] = data_Car['Timestamp'].dt.time
    
    data_balancedT2 = pd.concat([data_balancedT, data_Car])
    
    # Optional: Unter-Sampling der überrepräsentierten Klasse
    class_0 = data_balancedT2[data_balancedT2['Label'] == 0]
    class_1 = data_balancedT2[data_balancedT2['Label'] == 1]

    n_class_1 = len(class_1)
    class_0_sample = class_0.sample(n_class_1)
    data_balanced = pd.concat([class_0_sample, class_1])
    
    

    
    # Daten für Features und Labels aufteilen
    X = data_balanced.iloc[:, :-2].values
    y = data_balanced.iloc[:, -1].values


    # Daten normalisieren
    scaler = StandardScaler()
    X = scaler.fit_transform(X)

    # Daten in das erforderliche Format umwandeln
    X = X.reshape((-1, 20, 64))

    # Daten in Trainings- und Testsets aufteilen
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)
    X_train, X_val, y_train, y_val = train_test_split(X_train, y_train, test_size=0.2, random_state=42)

    
    # speichern der Daten um Zeit zu sparen
    with open(data_path + 'train_data.pkl', 'wb') as file:
        pickle.dump((X_train, y_train), file)

    with open(data_path +'test_data.pkl', 'wb') as file:
        pickle.dump((X_test, y_test), file)

    with open(data_path + 'val_data.pkl', 'wb') as file:
        pickle.dump((X_val, y_val), file)


# Öffnen der geseicherten Trainings-/Test-/Valdaten
with open(data_path + 'train_data.pkl', 'rb') as file:
    X_train, y_train = pickle.load(file)

with open(data_path + 'test_data.pkl', 'rb') as file:
    X_test, y_test = pickle.load(file)

with open(data_path + 'val_data.pkl', 'rb') as file:
    X_val, y_val = pickle.load(file)


print("Daten geladen...")

# Modell aufbauen
model = Sequential()
model.add(LSTM(10, return_sequences=False, input_shape=(20, 64), unroll=False, batch_size=1))  
model.add(Dense(1, activation='sigmoid'))  # Sigmoid-Aktivierung für binäre Klassifikation

# Modell kompilieren
print("Starte Training...")
model.compile(optimizer='adam', loss='binary_crossentropy', metrics=['accuracy', 
        tf.keras.metrics.Precision(name='precision'),
        tf.keras.metrics.Recall(name='recall'),
        tf.keras.metrics.AUC(name='auc')])

# Modell trainieren
model.fit(X_train, y_train, epochs=50, batch_size=64, validation_data=(X_test, y_test))

# Modell evaluieren
metrics = model.evaluate(X_val, y_val, return_dict=True)
print(metrics)

# Model speichern
model.save('model.keras')

def convert_tflite_model(model):
	import tensorflow as tf
	converter = tf.lite.TFLiteConverter.from_keras_model(model)
	tflite_model = converter.convert()
	return tflite_model

def save_tflite_model(tflite_model, save_dir, model_name):
	import os
	if not os.path.exists(save_dir):
		os.makedirs(save_dir)
	save_path = os.path.join(save_dir, model_name)
	with open(save_path, "wb") as f:
		f.write(tflite_model)
	print("Tflite model saved to %s", save_dir)
     
tflite_model = convert_tflite_model(model)

save_tflite_model(tflite_model, 'model', 'overtake_model_adjusted.tflite')
model.save_weights('./model/overtake_model_adjusted')
# after converting to tflite convert it to tflite for micro with the following:
# xxd -i overtake_model_adjusted.tflite > overtake_model_adjusted_data.cc

# Load the TensorFlow Lite model
interpreter = tf.lite.Interpreter(model_path='./model/overtake_model_adjusted.tflite')
interpreter.allocate_tensors()

# Get input and output tensors
input_tensor_index = interpreter.get_input_details()[0]['index']
output_tensor_index = interpreter.get_output_details()[0]['index']

# Run inference on test data
correct_predictions = 0
total_predictions = len(X_val)

for i in range(total_predictions):
    input_data = np.expand_dims(X_val[i], axis=0).astype(np.float32)
    interpreter.set_tensor(input_tensor_index, input_data)
    interpreter.invoke()
    lite_predictions = interpreter.get_tensor(output_tensor_index)[0]
    
    # Compare predicted label with true label
    predicted_label = round(lite_predictions[0],0)
    # print(model.predict(np.array( [X_val[i],])))
    # print(lite_predictions)
    true_label = y_val[i]
    
    if predicted_label == true_label:
        correct_predictions += 1
    else:
         print("lite model predicted: ", lite_predictions[0])
         print("org model predicted:  ", model.predict(np.array( [X_val[i],]),verbose = 0)[0][0])
         

# Compute accuracy
accuracy = correct_predictions / total_predictions
print("TensorFlow Lite model accuracy:", accuracy)