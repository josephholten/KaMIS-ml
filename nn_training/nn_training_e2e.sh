KAMIS_PATH=$(dirname $0)/..
MODELS=$KAMIS_PATH/models

if (( $# < 1 )); then
    echo "usage: MODEL_NAME [data_directory]"
fi

existing_data=1
for extension in {train,test}.{features,labels}; do 
    if ! find $MODELS/$1.$extension; then 
        existing_data=0
        break
    fi
done

if (( existing_data == 0 )); then
    echo "---------- calculating new feature data -------------"
    cd $2
    $KAMIS_PATH/build/wmis/train_model --only_data --model=$1 
else
    echo "---------- using existing feature data -------------"
fi

source $KAMIS_PATH/nn_training/venv/bin/activate
echo "---------- training neural network -------------"
python3 $KAMIS_PATH/nn_training/nn_training.py $MODELS/$1
echo "---------- converting neural network -------------"
python3 $KAMIS_PATH/wmis/extern/frugally-deep/keras_export/convert_model.py $MODELS/$1.nn_model.h5 $MODELS/$1.nn_model.json
