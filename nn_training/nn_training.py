import tensorflow as tf
from tensorflow import keras
from keras import layers, backend
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import MinMaxScaler

import sys

# load features and labels
data_path = sys.argv[1]
train_data_path = data_path + ".train"
test_data_path = data_path + ".test"

model_path = data_path + ".nn_model.h5"

print("Reading data")
features = np.loadtxt(train_data_path + ".features")
labels = np.loadtxt(train_data_path + ".labels")


feature_num = np.shape(features)[1]

# split into test/ train randomly
features_train, features_test, labels_train, labels_test = train_test_split(features, labels, test_size=0.1)

# normalize using MinMaxScaler from skikit.learn
print("Normalizing data")
scaler = MinMaxScaler()
scaler.fit(features_train)
features_train = scaler.transform(features_train)
scaler.fit(features_test)
features_test = scaler.transform(features_test)

# Define Sequential model with 3 layers
model = keras.Sequential(
    [
        layers.Input(shape=(feature_num,)),
        layers.Dense(512, activation="relu"),
        layers.Dropout(0.2),
        layers.Dense(512, activation="relu"),
        layers.Dropout(0.2),
        layers.Dense(64, activation="relu"),
        layers.Dense(16, activation="relu"),
        layers.Flatten(),
        layers.Dropout(0.5),
        layers.Dense(1, name="output"),
    ]
)

model.summary()

print("Compiling model")
model.compile(
    optimizer="Adam",
    loss="MeanSquaredError",
)
backend.set_value(model.optimizer.learning_rate, 0.001)

print("Fitting model")
history = model.fit(
    features_train,
    labels_train,
    batch_size=64,
    epochs=10,
    validation_data=(features_test, features_train)
)

model.save(model_path, include_optimizer=False)

# 10 epochs, 128 batch size, 0.001 learning rate
